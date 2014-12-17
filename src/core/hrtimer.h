#ifndef __hr_timer_H_
#define __hr_timer_H_
#include "config.h"


typedef struct sen_timer_t {
  float T;  // total time
  float dT; // delta time
  float LT; // last time
} sen_timer_t;


void sen_timer_restart(sen_timer_t *t);
float sen_timer_update(sen_timer_t *t); // ret dT
float sen_timer_now();
#endif
