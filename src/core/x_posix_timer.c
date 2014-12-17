#include "hrtimer.h"
#include "macro.h"
#include <sys/time.h>


//typedef struct timespec timespec;

double sen_timer_now() {
  struct timeval now;
  if (gettimeofday(&now, NULL) != 0) return 0;

//  clock_gettime(CLOCK_REALTIME, &now);
  return (double)now.tv_sec +  (now.tv_usec/ 1000000.0f);
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
