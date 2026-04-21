/*-------------------------------------------------------------------------------------------------------------------------------
Сводный пример работы с библиотеками mtimer, mlogger iniobject, to_words. Смысл в том, чтобы пример был похож на реальную
программу, делающую что-то полезное.
Программа читает конфигурационный файл, находит разбиение на группы чисел (можно менять все параметры) и по каждой группе
ходит единственный таймер, который генерирует случайные числа из диапазона, заданного в каждой группе и переводит их в
пропись. Случайным образом добавляются единицы измерения.
Результат работы выводитс в лог и на экран.
Предусмотрены разные проверки корректности настроечных параметров.
Многократный запуск приводит к ротации лог-файлов.

Автор - Ярослав Медокс
-------------------------------------------------------------------------------------------------------------------------------*/
#include <iostream>
#include <vector>
#include "iniobject.hpp"
#include "mlogger.hpp"
#include "mtimer.hpp"
#include "to_words.hpp"

//Блок макроопределений для упрощения кодирования и улучшения читаемости кода
#define logmessage logger->log
#define logperror(x) logger->log(ERROR, "In %s() line %d. %s: %s", __FUNCTION__,  __LINE__, x, strerror(errno))
#define logcppinfo *logger<<INFO
#define logcppwarn *logger<<WARN
#define logcpperror *logger<<ERROR<<"In "<<__FUNCTION__<<"() line "<<__LINE__<<". "
#define logcppdebug *logger<<DEBUG<<"In "<<__FUNCTION__<<"() line "<<__LINE__<<". "
cLogger *logger;        //Основной логгер программы

//Простая структура, которая хранит параметры групп чисел
struct groupOfNumbers {
  unsigned long _min;
  unsigned long _max;
  unsigned long _count;
  char *_name;
};
//List, который будет содержать эти структуры
std::vector<groupOfNumbers> groups;
//Интервал работы таймера, выводящего сообщения в лог
unsigned long interval;
//Защелка для контроля выхода из программы
bool bCont = true;
//Вспомогательный map, чтобы случайным образом использовать измеряемые величины в выводе
std::map<int, UnitType> types = {{0, UnitType::PERCENT}, {1, UnitType::DEGREE}, {2, UnitType::VOLT}, {3, UnitType::AMPERE}, {4, UnitType::WATT}, {5, UnitType::NONE} };


//Запуск логирования. Параметры не по умолчанию!
void initLogger(cIniObject *ini) {
   if(!logger) {
      cLogParams p;
      char *group  = (char *)"Logger";
      p.max_messages     = ini->getInt(group, (char *)"mmax", 100);
      p.max_size_of_file = ini->getInt(group, (char *)"fsize", 1000000);
      p.path             = ini->getString(group, (char *)"fpath");
      p.num_files        = ini->getInt(group, (char *)"fnum", 3);
      p.sink_cout        = true;
      p.sink_log         = true;
      p.tune_sleep       = true;  //этот логгер основной и имеет право регулировать скорость.
      p.log_level        = logger_getDebugLevelFromChar(ini->getString(group, (char *)"level"));
      if(p.log_level == SPEC) {                                  //вероятно, первый запуск прорграммы и конф-файл еще не создан
        ini->setString(group, (char *)"level", (char *)"DEBUG"); //строки не пишутся из значений по умолчанию, поэтому запишем здесь
        p.log_level      = DEBUG;                                //и установим собственно параметр
      }
      logger = logger_new(&p);
      logger_setSleepTime(ini->getInt(group, (char *)"stime", 10000));  //Выполняется, если нужно, после запуска первого логгера
      if(p.path) free(p.path);
      if(!logger) {
        exit(1);
      }
      logcppinfo<<"ya_hub logger started with level "<<c_level[p.log_level]<<ENDL;
   }
}

//Проверка допустимого значения
unsigned long getValue(uint64_t v, const char* name, const char *key) {
  unsigned long ret = v;
  if(v > 1200000) {
    ret = 1200000;
    logcpperror<<"Too big value set in group: "<<name<<", key: "<<key<<". Setting to "<<ret<<ENDL;
  }
  return ret;
}

//Определение списка групп чисел и заполнение параметров
void initGroups(cIniObject *ini) {
    gsize lines;
    char **keys;
    char *group  = (char *)"Groups";
    keys = ini->getKeys(group, &lines);
    if(lines) {
      for(gsize i=0;i<lines;i++) {
        char *name = ini->getString(group, keys[i], false);
        logcppdebug<<"Found group "<<name<<ENDL;
        groupOfNumbers gn;
        char *key = (char* )"min";
        gn._min   = getValue(ini->getInt(name, key, 0), name, key);
        key = (char* )"max";
        gn._max   = getValue(ini->getInt(name, key, 100), name, key);
        if(gn._min >= gn._max) {
          gn._min = 0;
          logcpperror<<"Min value bigger than max in group "<<name<<". Setting min to "<<gn._min<<ENDL;
        }
        gn._count = ini->getUInt(name, "count", 100);
        gn._name  = name;
        logcppdebug<<"For group "<<name<<" min="<<gn._min<<", max="<<gn._max<<", count="<<gn._count<<ENDL;
        groups.push_back(gn);
      }
    } else {
      logcpperror<<"Couldn't find groups of numbers to work with"<<ENDL;
    }
}

//Главная унифицированная функция для чтения параметров и инициализации всего.
void readValues(char *ini_fname) {
  cIniObject ini(ini_fname);
  initLogger(&ini);
  logcppdebug<<"logger init finished"<<ENDL;
  initGroups(&ini);
  logcppdebug<<"groups init finished"<<ENDL;
  char *group = (char*)"Settings";
  interval = ini.getUInt(group, (char*)"interval", 500);
  logcppdebug<<"settings init finished"<<ENDL;
}

//генератор случайных чисел с ограничением диапазона [_min, _max], включая границы
unsigned long bounded_rand(unsigned long _min, unsigned long _max) {
    unsigned long _ref = _max + 2 -_min;
    for (unsigned long x, r;;)
        if ((x = rand()) && (r = x % _ref))
            return r+_min-1;
}

//Функция единственного таймера. Делает всю работу.
void timerFunction() {
  static unsigned int groups_count = groups.size();
  static unsigned int i = 0, j = 0;
  if(groups_count == 0) {
    logcpperror<<"Groups for work not set in conf file"<<ENDL;
    bCont = false;
    return;
  }
  groupOfNumbers gn = groups[i];
  if(j < gn._count) {
    if(j == 0) logcppinfo<<"Начинаем считать группу "<<gn._name<<ENDL;
    double number = bounded_rand(gn._min, gn._max) + double(bounded_rand(0,10))/10;
    char buf[24];
    sprintf(buf, "%9.1f", number);
    logcppinfo<<"Число: "<<buf<<". Прописью: "<<NumberToWords::convert(number, types[bounded_rand(0,5)])<<ENDL;
    j++;
  } else {  //переходим к следующей группе
    logcppinfo<<"Закончили считать группу "<<gn._name<<ENDL;
    i++;
    j = 0;
  }
  if(i >= groups_count) bCont = false;  //программа отработала все группы чисел, выходим
}

int main(int argc, char *argv[]) {
    char *config_file = nullptr;
    //сначала читаем только путь к конфигурационному файлу, он может быть передан исполняемому файлу как -c path_to_file
    for(int acount=1;acount<argc;acount++) {
       if(strcmp(argv[acount], "-c")==0) {
         if(acount < argc-1) {
           acount++;
           config_file = argv[acount];
           break;
         }
       }
    }
    //теперь можно прочитать конф-файл
    readValues(config_file);
    logcppinfo<<"4in1_ex example started."<<ENDL;
    //Подготовка закончена, начинаем работу
    srand(time(NULL));  //рандомизируем начало
    new cTimer(interval, timerFunction, true);
    //Основной цикл. Закончится, когда все группы будут просчитаны.
    while(bCont) {
      timer_loop(false);
      usleep(timer_getSleepage() * 1000);      //Засыпаем. В это время ни у одного таймера нет плановой сработки
    }
    //Чистим за собой
    timer_killall();
    logcppinfo<<"Normally finished"<<ENDL;
    delete logger;
    return 0;
}
