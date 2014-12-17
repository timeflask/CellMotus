#ifndef __texture_H__
#define __texture_H__
#include "config.h"
#include "lmath.h"

typedef struct texture_t {
  const char*         name;
  unsigned int        id;
  vec4                coords;

  int                 img_alpha;
  int                 img_width;
  int                 img_height;

  const void*         _node;
} texture_t;

void
sen_texture_bind(const texture_t* tex);

const char*
sen_texture_atlas(const texture_t* tex);

#endif
