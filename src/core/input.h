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
typedef struct input_point {

  double x;
  double y;
  double z;
  double w;

} input_point;

typedef struct touch_t {
  int       id;
  int       has_start;
  input_point start;
  input_point point;
  input_point prev;
}touch_t;

typedef struct scroll_t 
{
  double x; 
  double y;
} scroll_t;

SEN_DECL touch_t*
sen_touch_new();

SEN_DECL void
sen_touch_to_world(touch_t* t, touch_t* res);


SEN_DECL void
sen_touch_delete(touch_t* self);
//--------------------------------------------------------------------
SEN_DECL void
sen_input_init();
SEN_DECL void
sen_input_destroy();
#endif
