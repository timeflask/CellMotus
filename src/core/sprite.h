#ifndef __sprite_H_
#define __sprite_H_
#include "node.h"
#include "texture.h"
#include "vertex-buffer.h"
#include "shader.h"
#include "types.h"

typedef struct sprite_t {
  node_t            super;
  const texture_t*  tex;
  vertex_buffer_t*  quad;
  const shader_t*   program;
  V4F_T2F_C4F       vertices[4];

} sprite_t;

sprite_t *
sen_sprite_new(const char* name,
               const char* texture_name);

void
sen_sprite_delete(void *self);

void
sen_sprite_render(void* self);

#endif
