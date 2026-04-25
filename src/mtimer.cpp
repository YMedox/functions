/*-------------------------------------------------------------------------------------------------------------------------------
Легкая и простая библиотека работы с таймерами. Не использует никакие современные классы, типа std::vector.
Только базовые С/С++ конструкции.
Таймеры создаются вызовом new cTimer(интервал_в_мс, коллбэк_функция, регулярный, пеараметр) и
удаляется с помощью delete экземпляр_таймера.
Основной цикл программы должен содержать timer_loop(false). Т.е. всё работает в единственном (!) вызывающем потоке.
Если нужно, чтобы сработали принудительно все таймеры, то timer_loop(true). Если один, то экземпляр->check(true).
Интервал до следующей сработки таймера можно менять независимо от его базовой настройки setNextRun(следующая_сработка_через).

Автор - Ярослав Медокс
-------------------------------------------------------------------------------------------------------------------------------*/
#include "mtimer.hpp"
#include <climits>

cTimer *last = NULL;                  //последний таймер в цепочке таймеров
unsigned long current_time, next_run; //текущее время и время ближайшей следующей сработки по всем таймерам
cTimer *timer_currentTimer;           //текущий таймер. Нужно если необходимо получить доступ к адресу таймера при его сработке

//Устанавливает текушее время в глобальную переменную
void get_current_time() {
   struct timespec spec;
   clock_gettime(CLOCK_REALTIME, &spec);
   current_time = spec.tv_sec * 1000L + (spec.tv_nsec / 1000000L); // Convert nanoseconds to milliseconds
}

//Создаем таймер с интервалом interval миллисекунд, он будет вызывать функцию callback_timer_check() и
//если continuous=false, то таймер сработает один раз и удалится
//Кроме этого строится цепь таймеров (можно было бы использовать std::vector вместо этого)
cTimer::cTimer(unsigned long interval, Callback callback_timer_check, bool continuous, void *ctx) {
  prev = last;
  if(prev) prev->next = this;
  last = this;
  next = NULL;
  step = interval;
  callbackFunc = callback_timer_check;
  context = ctx;       //используется для того, чтобы можно было передавать параметр в функцию обработки
  cont = continuous;
  get_current_time();  //иначе неправильно установится время сработки
  setNextRun();
}

//Выравнивает цепь таймеров, исключает выпавшее звено и пересчитавает время ближайшей сработки
cTimer::~cTimer() {
  if(prev) prev->next = next;
  if(next) next->prev = prev;
  else     last = prev;
  next_run = ULONG_MAX;  //просто устанавливаем реальное время сработки. В цикле найдем меньшее из всех
  for(cTimer *timer = last; timer; timer=timer->prev) timer->checkAndSetReadiness();  //при удалении может поменяться время ближайшей сработки какого-либо из таймеров
}

//Возвращает время следующей сработки
unsigned long cTimer::getNextRun() { return readiness; }

//Устанавливает время следующей сработки
void cTimer::setNextRun(unsigned long s) {
  readiness = current_time + (s>0?s:step);
}

//Возвращает интервал таймера
unsigned long cTimer::getStep() { return step; }

void *cTimer::getContext() { return context; }

//Проверяет и устанавливает время ближайшей сработки какого-либо таймера, чтобы можно было определить время для сна
void cTimer::checkAndSetReadiness() {
  if(readiness < next_run) next_run = readiness;
}

//Главная функция. Проверяет не пора ли таймеру сработать и если да, то вызывает его обработчик и устанавливает интервал
void cTimer::check(bool force_read) {
  if(readiness <= current_time || force_read) {
    setNextRun();                         //для одноразового таймера это не нужно, но не обременительно
    timer_currentTimer = this;
    callbackFunc();
    if(!cont) { delete this; return; }    //некрасиво удалять самого себя, зато очень удобно. Нужно вернуться, чтобы next_run правильным был
  }
  checkAndSetReadiness();                 //проверяем общее время ближайшей сработки
}

//Вызывается в бесконечном цикле, в main(), например
//Если force_read=true, то все таймеры сработают и установятся следующие срабатывания на их интервал
void timer_loop(bool force_read) {
   get_current_time();
   next_run = ULONG_MAX;  //просто устанавливаем реальное время сработки. В цикле найдем меньшее из всех
   for(cTimer *timer = last; timer; timer=timer->prev) timer->check(force_read);
}

//Возвращает время до ближайшей сработки любого из таймеров. Вызывается сразу после timer_loop();
//Можно использовать для отправки вызывающего потока в сон.
//Параметр - время простоя по умолчанию, если нет ни одного таймера.
unsigned long timer_getSleepage(unsigned long sleepage) {
   if(!last) return sleepage;
   get_current_time();
   if(next_run > current_time) return next_run - current_time;
   return 0L;
}

//уничтожает все таймеры
void timer_killall() {
   for(cTimer *timer = last; timer; timer=timer->prev) delete timer;
}
