#ifndef __image_H_
#define __image_H_
#include "config.h"
#include "opengl.h"

typedef struct image_t {
  char*  filename;
  size_t width;
  size_t height;
  size_t size;
  void*  raw_data;
  GLenum gl_color_fmt;
  int    channels;
  int    channel_bit_depth;
  int    stride;
  int    alpha;
} image_t;

SEN_DECL image_t* image_new(const char* filename);
SEN_DECL void image_delete(image_t* self);


#endif
