#ifndef MLOGGER_HPP_INCLUDED
#define MLOGGER_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>
#include <mutex>
#include <pthread.h>
#include <sys/time.h>
#include <stdarg.h>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


#define L_OFF 0
#define ERROR 1
#define WARN  2
#define INFO  3
#define DEBUG 4
#define SPEC  5


struct cLogParams {
  char     	*path;               //Путь к файлу. Если NULL - для первого логгера определится автоматически
  uint8_t  	num_files;           //Количество файлов в ротации.
  long          max_size_of_file;    //Ориентир для максимального размера файла.
  int      	max_messages;        //Ориентир для максимального количества записей лога в памяти, ожидающих сброса в файл.
  bool     	sink_cout;           //true = Выводить на экран сообщения. Можно менять по ходу программы.
  bool     	sink_log;            //true = записывать в файл. файл открывается всегда. Можно менять по ходу программы.
  bool     	tune_sleep;          //true = разрешить регулирование времени простоя потока. Надо устанавливать только для одного самого нагруженного логгера. Это поведение библиотеки по умолчанию
  uint8_t  log_level;
};

std::string getTimePrefix();
extern const char *c_level[];
extern const char *ENDL;



class cLogger {
  private:
     std::ofstream log_file;
     cLogParams params;
     void prepareName(char *name, uint8_t i);
     bool rotateFiles();
     void finalizePath(char *buf);
     void setWorkingPath();
     std::vector<std::string> v_messages;
     std::mutex message_mutex;
     size_t half_of_queue, doubled_queue;
     std::stringstream os;
     std::map<pthread_t, std::string> msgs;
     std::map<pthread_t, bool> lvls;
  public:
     cLogger(cLogParams *p);
     ~cLogger();
     template <typename T> cLogger &operator <<(const T &x) {
        pthread_t thread = pthread_self();
        message_mutex.lock();
        auto it = msgs.find(thread);
        auto lv = lvls.find(thread);
        os.str("");
        os << x;
        if(it == msgs.end()) {
           uint8_t level = atoi(os.str().c_str());
           if(params.log_level >= level) {
             msgs.insert(std::pair<pthread_t, std::string>(thread, getTimePrefix()+c_level[level]));
             lvls.insert(std::pair<pthread_t, bool>(thread, true));
           } else {
             msgs.insert(std::pair<pthread_t, std::string>(thread, ""));
             lvls.insert(std::pair<pthread_t, bool>(thread, false));
           }
           it = msgs.find(thread);
           lv = lvls.find(thread);
        } else {
          if(lv->second) {
            msgs[thread] += os.str();
            if(msgs[thread].back() == '\n') {
              if(params.sink_cout) { std::cout<<it->second; }
              if(params.sink_log)  { std::string s = it->second; s.pop_back(); v_messages.push_back(s); }
              msgs.erase(it);
              lvls.erase(lv);
            }
          } else {
            if(os.str().back() == '\n') {
              msgs.erase(it);
              lvls.erase(lv);
            }
          }
        }
        message_mutex.unlock();
        return *this;
     }
     bool openLogFile();
     void log(uint8_t level, const char *msg, ...);
     void flush();
     void flushOne();
     void getParams(cLogParams *p);
     void setParams(cLogParams *p);
     size_t getQueueSize();
};


void logger_setSleepTime(unsigned long sleep);
void logger_getDefaults(cLogParams *p);
uint8_t logger_getDebugLevelFromChar(const char *level);


cLogger *logger_new(cLogParams *params);


#endif // MLOGGER_HPP_INCLUDED
