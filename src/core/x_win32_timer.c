#include "hrtimer.h"
#include "macro.h"
#include <windows.h>

//typedef struct timespec timespec;


double sen_timer_now() {

  LARGE_INTEGER liTime, liFreq;
  QueryPerformanceFrequency( &liFreq );
  QueryPerformanceCounter( &liTime );
  return (double)( liTime.QuadPart / liFreq.QuadPart );
}
/*
void sen_timer_restart(sen_timer_t *t) {
  t->T = 0.0f;
  t->dT = 0.0f;
  t->LT = sen_timer_now();
}

float sen_timer_update(sen_timer_t *t) {
  float nows = sen_timer_now();
  t->dT =   nows - t->LT;
  t->LT = nows;
  t->T += t->dT;
  return t->dT;
}

*/
