#ifndef __camera_H_
#define __camera_H_
#include "config.h"
#include "lmath.h"
#include "node.h"

typedef struct camera_t {
  node_t   super;
  //mat4     view;
  mat4     proj;
  mat4     view_proj;
  mat4     ident;
} camera_t;

camera_t*
sen_camera_new(const char* name);

void
sen_camera_destroy(camera_t* self);

void
sen_camera_update_view(object_t* _self, const vec4* r);

//int
//_sen_camera_view_change(object_t* _self, void* data, object_t* sender);

extern camera_t*
sen_camera();
#endif
