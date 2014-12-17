#include <time.h>
#include "hrtimer.h"
#include "macro.h"


typedef struct timespec timespec;

float sen_timer_now() {
  timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (float)(now.tv_sec*1000000000ull + now.tv_nsec)  * 0.000000001f;
}

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

