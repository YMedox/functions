/*-------------------------------------------------------------------------------------------------------------------------------
Пример работы с библиотекой mtimer.
Функция main подробно прокомментирована.
В примере создание регулярных и одноразового таймеров. Создание таймера, которому можно передать параметр, а также
изменение интервала сработки работающего регулярного таймера.

Время работы программы определяется параметрами при создании cTimerWithParam. В исходном варианте - чуть больше 20 секунд.

Автор - Ярослав Медокс
-------------------------------------------------------------------------------------------------------------------------------*/

#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <functional>
#include <map>
#include "mtimer.hpp"

using namespace std;
static bool bCont = true;   //защелка основного цикла программы
cTimer *crazyTimer;         //указатель на "сумасшедший" таймер, чтобы можно было управлять его параметрами

//Получаем отметку времени
string getTimePrefix() {
  timeval t_v;
  time_t seconds;
  struct tm *t_m;
  gettimeofday(&t_v, NULL);
  seconds = t_v.tv_sec;
  t_m = localtime(&seconds);
  char prefix[86];
  sprintf(prefix, "%02d:%02d:%02d.%03d ", t_m->tm_hour, t_m->tm_min, t_m->tm_sec, int((t_v.tv_usec) / 1000));
  return string(prefix);
}

//генератор случайных чисел с ограничением диапазона
unsigned bounded_rand(unsigned range) {
    for (unsigned x, r;;)
        if (x = rand(), r = x % range, x - r <= -range)
            return r;
}

//Выводит строку лога программы
void printString(string msg) {
  cout<<getTimePrefix()<<msg<<endl;
}

//Функция первого таймера
void firstTimerFunc() {
  printString("This is simple repeating timer");
}

//Функция второго таймера
void secondTimerFunc() {
  printString("This is one time timer. Bye, we will never meet again.");
}

//Функция "сумасшедшего" таймера, сработки которого определяются в другом таймере случайным образом.
//Иллюстрация возможности установки интервала, не равного интервалу при создании таймера
void crazyTimerFunc() {
  printString("This is crazy timer. My interval is randomly set by cTimerWithParam");
}

//Функция для таймера с переданными параметрами. 2 параметра зашито в обертке для таймера.
void forTWithParam() {
    int *steps = (int*)(timer_currentTimer->getContext());  //получаем сохраненный в таймере параметр
    unsigned long step = timer_currentTimer->getStep();     //шаг таймера
    static int toStop = *steps - 1;                         //здесь начинаем отчет назад
    unsigned long nextInterval = bounded_rand(3000);        //генерим следующий интервал для crazyTimer
    printString("This is timer with parameter. I will push crazyTimer to shot next time after "+to_string(nextInterval)+" ms.");
    if(crazyTimer) crazyTimer->setNextRun(nextInterval);    //Один таймер устанавливает интервал другому таймеру.
    if(toStop) {
      printString("Program will stop after "+to_string(toStop*step)+" ms");
      toStop--;
    } else {
      printString("Program will stop now");
      bCont = false;
    }
}

int main() {

    srand(time(NULL));                                          //рандомизируем начало

    printString("Start");                                       //Засекаем время
    new cTimer(1000, firstTimerFunc, true);                     //Создаем простой периодический таймер на 1с. Даже если он не создастся, он не сломает программу
    new cTimer(5000, secondTimerFunc, false);                   //Создаем простой одноразовый таймер. Он самоудалится после отработки. Ничего чистить не надо.
    crazyTimer = new cTimer(1000000L, crazyTimerFunc, true);    //Создаем таймер с заведомо нереальным интервалом. Он будет установливаться следующим таймером
    int steps = 10;
    new cTimer(2000, forTWithParam, true, &steps);              //А вот если нужно в таймер при создании передать параметр, то это можно сделать с помощью передачи контекста
                                                                //Параметры: 2000 - это собственный интервал в ms, 10 - параметр, определяющий число циклов работы до остановки программы.
    while(bCont) {                                              //Незамысловатый основной цикл программы. Остановлен будет таймером с параметром
      timer_loop(false);                                        //Проверяем все таймеры. Если передать true, то срабатают все не дожидаяся своего времени;
      unsigned long sleepage = timer_getSleepage();             //После проверки и "перезарядки" всех таймеров программа может поспать, чтобы не грузить процессор
      cout<<endl;
      printString("Main thread goes to sleep for "+to_string(sleepage)+" ms\n");
      usleep(sleepage * 1000);                                  //Засыпаем. В это время ни у одного таймера нет плановой сработки
    }
    printString("Stop");                                        //финальная засечка времени
    timer_killall();                                            //удаляем все таймеры
    return 0;
}
