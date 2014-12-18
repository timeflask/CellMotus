#include "hrtimer.h"
#include "macro.h"
#include <windows.h>

double sen_timer_now() {
  LARGE_INTEGER liTime, liFreq;
  QueryPerformanceFrequency( &liFreq );
  QueryPerformanceCounter( &liTime );
  return  liTime.QuadPart / (double)liFreq.QuadPart ;
}

