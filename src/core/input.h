#ifndef __input_H_
#define __input_H_
#include "config.h"
#include "lmath.h"

//--------------------------------------------------------------------
typedef struct touch_data_t{
  int    num;
  int*   ids;
  float* xs;
  float* ys;
}touch_data_t;


//--------------------------------------------------------------------
typedef struct touch_t {
  int    id;
  int    has_start;
  vec4   start;
  vec4   point;
  vec4   prev;
}touch_t;

touch_t*
sen_touch_new();

void
sen_touch_to_world(touch_t* t, touch_t* res);


void
sen_touch_delete(touch_t* self);
//--------------------------------------------------------------------
void
sen_input_init();
void
sen_input_destroy();
#endif
