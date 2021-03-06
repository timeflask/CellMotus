#ifndef __quad_H_
#define __quad_H_
#include "node.h"
#include "vertex-buffer.h"
#include "shader.h"
#include "types.h"

typedef struct cquad_t {
  node_t            super;
  vertex_buffer_t*  quad;
  const shader_t*   program;
  V4F_C4F           vertices[4];

} cquad_t ;

SEN_DECL cquad_t  *
sen_quad_new(const char* name);

SEN_DECL void
sen_quad_delete(void *self);

SEN_DECL void
sen_quad_render(void* self);



#endif
