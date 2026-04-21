/*------------------------------------------------------------------------------------------------------------------------------
Простая асинхронная потокобезопасная реализация логгера. Разные потоки могут писать в один лог.
Создание логгера - logger_new(). Создавать можно много. Но, начиная со второго, явно указывать путь к файлу лога.
Кроме этого, начиная со второго устанавливается запрет на настройку времени простоя потока.

Поток создается автоматически при создании первого логгера. Тогда же устанавливаются параметры по умолчанию для логеров.
Использование: метод logId->log(INFO, "format %d %s", ...) логгера *logId возвращенного функцией logger_new()
Или вот так *logId<<INFO<<"Something happened"<<ENDL; ОБЯЗАТЕЛЬНО ИСПОЛЬЗОВАНИЕ ENDL В КОНЦЕ ВВОДА!
По окончании работы программы - удалите логгер. При этом из памяти все будет сброшено в файл (зашито в деструкторе).
При переполнении буфера памяти, логгер сбрасывает все в файл принудительно. При этом новые сообщения не добавляются, вызывающий
поток будет ждать выгрузки в файл. Это имеет значение только на предельных скоростях.

По окончании программы логгер обязательно удалить!

Параметры логгера вплоть до имени файла можно менять на ходу (но не все поменяется сразу же).

Автор - Ярослав Медокс
-------------------------------------------------------------------------------------------------------------------------------*/
#include "mlogger.hpp"


//время простоя потока, шаг регулировки простоя, начальное значение простоя для возврата к нему, микросекунд
unsigned long SLEEP_TIME, sleep_step, sleep_set;
//уровни логирования
const char *c_level[] = {"  OFF: ", "ERROR: ", " WARN: ", " INFO: ", "DEBUG: ", " SPEC: "};
//обязательное окончание строки при исподьзовании оператора <<
const char *ENDL = "\n";
//максимальная длина пути при определении положения лог-файла
static const int max_path = 512;
//коллекция всех логгеров. На все логгеры - один поток
std::vector<cLogger *> Loggers;
//настройки новых логгеров по умолчанию
cLogParams defaults;

//Установка референсного (по умолчанию) значения простоя потока записи в файл, микросекукды.
//По мере записи оно регулируется автоматически
void logger_setSleepTime(unsigned long sleep) {
   SLEEP_TIME = sleep;
   sleep_step = sleep / 10;
   sleep_set  = sleep;
}

//Возвращает копию defaults.
void logger_getDefaults(cLogParams *p) {
  if(p) { memcpy(p, &defaults, sizeof(cLogParams)); }
}


//Установить/получить параметры.
void cLogger::setParams(cLogParams *p) {
  if(p) {
     if(p->path != params.path) {
       if(params.path) free(params.path);   //это может быть тот же самый указатель, что и p->path
       char *buf = static_cast<char *>(malloc(strlen(p->path)+1));
       if(buf) strcpy(buf, p->path);
       memcpy(&params, p,         sizeof(cLogParams));
       if(buf) params.path = buf;
     } else {
       memcpy(&params, p,         sizeof(cLogParams));
     }
  } else {
     if(params.path) { free(params.path); params.path = 0; }
     memcpy(&params, &defaults, sizeof(cLogParams));
  }
  half_of_queue = params.max_messages >> 1;
  doubled_queue = params.max_messages << 1;
}
void cLogger::getParams(cLogParams *p) {
  if(p) { memcpy(p, &params, sizeof(cLogParams)); }
}

//Служебная функция. Финализирует путь к первому файлу лога.
void cLogger::finalizePath(char *buf) {
        params.path =static_cast<char *>(malloc(strlen(buf)+5));
        strcpy(params.path, buf);
        strcpy(&params.path[strlen(buf)], ".log");
}

//Служебная функция. Если не получилось с определением пути к логу, пробуем взять рабочий каталог
void cLogger::setWorkingPath() {
    char buf[max_path];
    if (getcwd(buf, sizeof(buf)) != NULL) {
       finalizePath(buf);
    } else {
       finalizePath((char*)"/log");
    }
}

//Конструктор. Передаются параметры логгера. ДЛя первого логгера можно ничего не передавать (NULL). Имя файла определится автоматически и будет лежать рядом с использняемым файлом
//При повторных вызовах обязательно указывать аболютный путь к файлу лога.
cLogger::cLogger(cLogParams *p) {
  memset(&params, 0, sizeof(cLogParams)); //добавлено 03.02.2019. Иначе в setParams лишний вызов free()
  params.log_level = INFO;  //по умоланию уровень логирования - INFO
  setParams(p);
  if(params.path == NULL) {
      char buf[max_path];
      ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf));
      if(len <= (ssize_t)sizeof(buf) - 5) {
        buf[len]   = 0;
        finalizePath(buf);
      } else {
         std::cout<<"Error creating cLogger 1";
         setWorkingPath();
      }
  } else {  //указываем абсоютный путь
    char *path = static_cast<char *>(malloc(strlen(p->path)+1));
    if(path) {
      strcpy(path, p->path);
      params.path = path;
    } else {
       std::cout<<"Error allocating memory for log path";
       setWorkingPath();
    }
  }
  openLogFile();
}

//Возвращает количество сообщений в очереди на запись.
size_t cLogger::getQueueSize() {
  return v_messages.size();
}

//Сбрасываем все из памяти в файл и закрываем его.  Обязательно удалить себя из вектора Loggers. Это позволяет безопасно удалять логгеры и снова создавать
cLogger::~cLogger() {
  flush();
  if(log_file.is_open()) log_file.close(); //close(log_file);
  if(params.path) free(params.path);
  std::vector<cLogger *>::iterator it=std::find(Loggers.begin(), Loggers.end(), this);
  if(it != Loggers.end()) {
     Loggers.erase(it);      //надо проверить, что не возникает segmentation fault
  }
  //std::cout<<Loggers.size()<<std::endl;
}

//службеная функция
void cLogger::prepareName(char *name, uint8_t i) {
       strcpy(name, params.path);
       if(i) sprintf(&name[strlen(params.path)-3], "%03d", i);
}

//Функция ротации файлов. проверяется размерр текущего файла и количество разрешенных файлов.
//Важно, что размер файла не ограничен, если количество файлов 1.
//Файлы переименовываются по цепочке. Не стоит много файлов указывать.
bool cLogger::rotateFiles() {
  long pos = log_file.tellp(); //нельзя делать unsigned long
  if(pos < 0) pos = 0;
  if(pos >= params.max_size_of_file && params.num_files > 1) {
    if(log_file.is_open()) log_file.close();
    uint8_t i;
    int len = strlen(params.path) + 1;
    char *name_from = static_cast<char *>(malloc(len));
    char *name_to   = static_cast<char *>(malloc(len));
    if(name_from && name_to) {
      for(i=params.num_files-1; i>0; i--) {
        prepareName(name_to, i);
        if(i == params.num_files-1) {
          if(remove(name_to) !=0) {
            std::cout<<"Error removing log file "<<name_to<<std::endl;
           // return false;
          }
        }
        prepareName(name_from, i-1);
        if(rename(name_from, name_to) !=0 ) {
          std::cout<<"Error renaming log file from "<<name_from<<" to "<<name_to<<std::endl;
          // return false;
        }
      }
    } else {
      std::cout<<"Error allocating memory in rotateFiles"<<std::endl;
      return false;
    }
    if(name_from) free(name_from);
    if(name_to)   free(name_to);
  }
  return true;
}

//Открывает файл и проверяет ротацию
bool cLogger::openLogFile() {
  if(!rotateFiles()) return false;
  if(!log_file.is_open()) {
    log_file.open(params.path, std::ios_base::app);
    if(!log_file.is_open()) {
      std::cout<<"Error opening file "<<params.path<<std::endl;
      //SLEEP_TIME = 1e8;   //замедляем поток максимально
      params.sink_log = false; //ПРОСТО ОТКЛЮЧАЕМ ЛОГРОВАНИЕ В ФАЙЛ
      return false;
    }
  }
  return true;
}

//Получаем отметку времени
std::string getTimePrefix() {
  timeval t_v;
  time_t seconds;
  struct tm *t_m;
  gettimeofday(&t_v, NULL);
  seconds = t_v.tv_sec;
  t_m = localtime(&seconds);
  char prefix[86];  //если здесь поставить реальный размер (22), то программа завершается с кодом 134. Компилятор предупреждает о записи между 23 и 85
  sprintf(prefix, "%02d.%02d.%02d %02d:%02d:%02d.%03d ", t_m->tm_mday, t_m->tm_mon+1, t_m->tm_year-100, t_m->tm_hour, t_m->tm_min, t_m->tm_sec, int((t_v.tv_usec) / 1000));
  return (std::string)prefix;
}

//возвращает уровень логирования по имени из c_level без пробелов и двоеточия
uint8_t logger_getDebugLevelFromChar(const char *level) {
  if(level == NULL) return SPEC;
  std::string slevel = level;
  for(uint8_t i=0;i<=SPEC;i++) {   //!!!! SPEC должен быть последним!
    std::string tmp = c_level[i];
    //while(tmp.substr(0, 1) == " ") { tmp = tmp.substr(1); }
    tmp.erase(0, tmp.find_first_not_of(" "));
    //while(tmp.substr(tmp.length()-1, 1) == " ") { tmp = tmp.substr(0, tmp.length()-1);
    tmp.erase(tmp.find_last_not_of(" ") + 1);
    //tmp = tmp.substr(0, tmp.length()-1);
    tmp.resize(tmp.length()-1);
    if(tmp.compare(slevel) == 0) return i;
  }
  return SPEC;
}


//Главный вызов логгера.сделана в стиле printf. Понимает только %d и %s (и %%) без спецификаций размера
//Потокобезопасная
void cLogger::log(uint8_t level, const char *msg, ...) {
  if(params.log_level < level) return;  //если не попадаем в уровень логирования просто не пишем ничего
  std::string _msg = getTimePrefix() + c_level[level];
  va_list argptr;
  va_start(argptr, msg);
  char *p, *prev;
  prev = (char *)(msg);
  p = strstr(prev, "%");
  if(p) {
   //memset(buf, 0, sizeof(buf));
   do {
     char buf[256];
     strncpy(buf, prev, p-prev); buf[p-prev] = 0;
     _msg += (std::string)buf;
     prev = p + 1;
     switch(*prev) {
         case 'd':
            sprintf(buf, "%d", va_arg(argptr, int));
            break;
         case 's':
            strcpy(buf, va_arg(argptr, char *));
            break;
         case '%':
            strcpy(buf, "%");
            break;
         default:
            break;
     }
     prev += 1;
     _msg += (std::string)buf;
     p = strstr(prev, "%");
   } while(p);
   _msg += (std::string)prev;
  } else {
    _msg += (std::string)msg;
  }
  if(params.sink_log) {
    message_mutex.lock();
    v_messages.push_back(_msg);
    message_mutex.unlock();
  }
  va_end(argptr);
  if(params.sink_cout) std::cout<<_msg<<std::endl;
}

//Сохранить все. Вызывается из деструктора. ПОэтому логгер лучше удалять рпи завершении программы.
void cLogger::flush() {
  //std::vector<std::string>::iterator it;
    if(params.sink_log) {
       //for(it=v_messages.begin();it<v_messages.end();it++) {
       for(const auto& it:v_messages) {
         log_file<<it<<std::endl;
       }
    }
    v_messages.clear();
}

//Потокобезопасная функция построчной записи в лог файл. Здесь же проверяется ротация файла и настраивается время простоя потока, чтобы меньше тормозить другие потки.
//Пишет построчно. В будущем можно повысить производительность, если писать сразу по несколько строк или же большими блоками памяти.
//Важно, чтобы время простоя регулировал только один логгер. По усолчанию - это первый логгер.
void cLogger::flushOne() {
  message_mutex.lock();
  size_t sz = v_messages.size();
  if(sz < doubled_queue) {
       std::vector<std::string>::iterator it=v_messages.begin();
       if(it != v_messages.end()) {
         log_file<<*it<<std::endl;
         v_messages.erase(it);
       }
       if(params.tune_sleep) {
         if(sz > half_of_queue) {          //регулируем время простоя потока записи в файл. Если записи начинают накапливаться - уменьшаем время простоя, уходят - увеличиваем.
           if(SLEEP_TIME > sleep_step) { SLEEP_TIME -= sleep_step; }  //Диапазон от sleep_step до sleep_set
           else                        { SLEEP_TIME = SLEEP_TIME >> 1; };  //делим пополам до нуля. Если попали сюда - значит неправильно подобраны параметры логгера
         } else {
           if(SLEEP_TIME < sleep_set ) SLEEP_TIME += sleep_step;  //увеличиваем постепенно время сна
         }
       }
  } else {
     //Log(WARN, "Log messages queue is %d. Flushing log. Try to tune logger parameters", sz);
     char s[16]; sprintf(s, "%i", int(sz));
     std::string msg = getTimePrefix() + c_level[WARN] + "Log message queue is " + s + ". Flushing to file.";
     if(params.sink_cout) std::cout<<msg<<std::endl;
     v_messages.push_back(msg);
     flush();
  }
  message_mutex.unlock();
}

//Функция потока логгера. Записывает из памяти сообщения в файл. Один поток и одна функция на все логгеры!
void *logger_thread_func(void *ptr) {
  //std::vector<cLogger *>::iterator it_logger;
  while(1) {
    //for(it_logger=Loggers.begin();it_logger<Loggers.end();it_logger++) {
    for(const auto& c:Loggers) {
      //cLogger *c = *it_logger;
      if(c->openLogFile()) {  //здесь ротация зашита
        c->flushOne();
      }
    }
    usleep(SLEEP_TIME);    //эта величина регулируется в FlushOne
  }
};

//Вызов инициализации логгера. При первом вызове запускает поток для записи в файл. При последующих только добавляет логгеры. Поток остается один.
//Важно при последующих вызовах указывать абсолютный путь к лог файлу.
cLogger *logger_new(cLogParams *params) {
   static pthread_t log_thread = 0;
   if(Loggers.size()) { //не первый логгер
     if(params == NULL) {
        std::cout<<"Not allowed to pass null for the next call of logger_new!"<<std::endl;
        return NULL;
     } else {
       if(params->path == NULL) {
          std::cout<<"Not specified path to log file in the next call of logger_new!"<<std::endl;
          return NULL;
       }
       if(params->tune_sleep) {
          std::cout<<"Setting tune_sleep=false in the next call of logger_new!"<<std::endl;
          params->tune_sleep = false;
       }
     }
   }
   if(defaults.num_files <= 0) {  //при первом запуске устанавливаем значения по умолчанию. Далее они сохраняются
      defaults.num_files = 3;
      defaults.max_size_of_file = 1024L * 1024L;
      defaults.max_messages = 100;
      defaults.path = NULL;
      defaults.sink_cout  = true;
      defaults.sink_log   = true;
      defaults.tune_sleep = true;
      defaults.log_level  = INFO;
      defaults.path = NULL;
      SLEEP_TIME = sleep_set = 1000L;
      sleep_step = sleep_set / 10;
   }
   cLogger *logger = new cLogger(params);
   if(logger) Loggers.push_back(logger);
   if(Loggers.size() == 1) {        //первый логгер
      defaults.tune_sleep = false;  //после первого логгера по дефолту - запрещается регулировать время простоя потока.
      if(log_thread == 0) {         //Поток создается один и только один раз!
        if (pthread_create(&log_thread, NULL, logger_thread_func, NULL) != 0) {
          logger->log(ERROR, (char*)("Creating the loop thread"));
          delete logger;            //Будет сделана запись в файл при удалении логгера.
          Loggers.erase(Loggers.begin());
          log_thread = 0;
          return NULL;
        }
      }
   }
   return logger;
}
