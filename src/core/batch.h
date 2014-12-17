#ifndef __batch_H_
#define __batch_H_
#include "config.h"
#include "node.h"
#include "texture.h"
#include "shader.h"
#include "vertex-buffer.h"
#include "font.h"

typedef struct batch_t
{
  const texture_t  *tex;
  const font_t     *font;
  vertex_buffer_t   buff;
  const shader_t   *program;
  blend_func        blend;
  const mat4*       mvp;
  float             zmin;
  float             zmax;
} batch_t;

batch_t*
sen_batch_new(const texture_t  *_tex,
              const font_t     *_font,
              const shader_t   *_program,
              blend_func        _blend,
              const mat4*       _mvp,
              float             zOrder);

void
sen_batch_destroy(batch_t* self);

#endif
