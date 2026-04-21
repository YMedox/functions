/*-------------------------------------------------------------------------------------------------------------------------------
Пример работы с библиотекой mlogger.
Функция main подробно прокомментирована.
В примере создание основного логгера, цикл для искусственного переполнения буфера сообщений в памяти и принудительного сброса
в файл. Если сброса в файл не было, значит дисковая подсистема  очень быстрая.
Создание второго логгера. Изменение параметров второго логгера на ходу.

Автор - Ярослав Медокс
-------------------------------------------------------------------------------------------------------------------------------*/
#include <iostream>
#include "mlogger.hpp"

//Блок макроопределений для упрощения кодирования и улучшения читаемости кода
#define logmessage logger->log
#define logperror(x) logger->log(ERROR, "In %s() line %d. %s: %s", __FUNCTION__,  __LINE__, x, strerror(errno))
#define logcppinfo *logger<<INFO
#define logcppwarn *logger<<WARN
#define logcpperror *logger<<ERROR<<"In "<<__FUNCTION__<<"() line "<<__LINE__<<". "
#define logcppdebug *logger<<DEBUG<<"In "<<__FUNCTION__<<"() line "<<__LINE__<<". "
cLogger *logger;        //Основной логгер программы

using namespace std;

int main()
{
    cLogParams p;
/*    p.log_level        = INFO;        //Задаем уровень логирования. Всего 6 уровней, см hpp.
    p.max_messages     = 50;            //Ориентир максимального количества сообщений в памяти до принудительного сброса в файл
    p.max_size_of_file = 500000L;       //Ориентир максимального размера файла в байтах
    p.num_files        = 4;             //Количество файлов в ротации
    p.sink_cout        = true;          //Выводить на экран
    p.sink_log         = true;          //Выводить в файл
    p.tune_sleep       = true;          //Разрешить регулировку скорости потока логгера (вернее времени сна)
    p.path             = nullptr;*/     //Если указать NULL, то создастся файл в папке программы, с таким же именем и расширением log
    logger = logger_new(NULL);          //Создаем основной логгер. Альтернативно заданию всех параметров, можно передать NULL и все значения будут по умолчанию
                                        //Все значения параметров логгера можно поменять на ходу через getParams и setParams.
    logger_setSleepTime(1000);          //Вызывается после запуска 1го логгера! Установка времени простоя потока логгера в микросекундах. Можно не вызывать, значение будет установлено по умолчанию в библиотеке
    if(!logger) {
      cout<<"Couldn't create logger"<<endl;
      return -1;
    }
    logger->log(INFO, "Logger sample started!");                        //Пример классической записи
    for(int i=0;i<509;i++) {                                            //В цикле быстро забивается очередь сообщений на сохранение и вызывается принудительный сброс в файл
      logcppinfo<<"Logging "<<i<<" message"<<ENDL;                      //Пример записи в стиле потока. Нужно завершать символом "\n"
      if(i == 100) logcpperror<<"Example of error message "<<ENDL;      //Пример записи в стиле потока
    }
    usleep(1000);                                                       //Даем сбросить все в файл.
    logcppwarn<<"Have a look at log file somewhere in between. Possibly there were flushings of overloaded queu."<<ENDL;

    logger->getParams(&p);              //Для второго логгера возьмем основные параметры первого и поменяем то, что обязательно надо менять
    p.path       = (char*)"auxlog.log"; //Первый параметр - путь с именем файла, он же не может писать в тот же лог-файл, что и основной логгер
    p.tune_sleep = false;               //Все последующие после первого логгера не могут регулировать время сна, чтобы не повредить основному логгеру.
    cLogger *auxlog = logger_new(&p);   //Создаем второй логгер с передачей параметров.
    if(auxlog) {
      auxlog->log(INFO, "Creating additional log file with name %s", p.path);  //Эта запись будет выведена на экран, туда же, куда и основной логгер выводит
      p.sink_cout  = false;                                                    //Выключаем вывод на экран для второго логгера
      auxlog->setParams(&p);
      *auxlog<<WARN<<"Change p.sink_cout to see messages in terminal"<<ENDL;  //Эта запись будет только в файле, на экране не будет
      delete auxlog;
    }
    string msg = "Check " + string(p.path) + " for additional content";
    logmessage(INFO, msg.c_str());                                             //Еще одно макро для упрощения записи
    delete logger;
    return 0;
}
