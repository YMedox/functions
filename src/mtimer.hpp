#ifndef MTIMER_HPP_INCLUDED
#define MTIMER_HPP_INCLUDED
#include <ctime>

typedef void (*Callback)(void);
void timer_loop(bool force_read);
unsigned long timer_getSleepage();
void timer_killall();

class cTimer {
protected:
  unsigned long readiness, step;
  bool cont;
  Callback callbackFunc;
  void* context;
public:
  cTimer *prev, *next;
  ~cTimer();
  cTimer(unsigned long interval, Callback callback_timer_check, bool continuos, void *ctx = NULL);
  void check(bool force_read);
  void checkAndSetReadiness();
  void setNextRun(unsigned long s = 0);
  unsigned long getNextRun();
  unsigned long getStep();
  void *getContext();
};

extern cTimer *timer_currentTimer;

#endif // MTIMER_HPP_INCLUDED
