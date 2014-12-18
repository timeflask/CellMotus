#ifndef __render_H_
#define __render_H_
#include "config.h"
#include "node.h"
#include "texture.h"
#include "shader.h"
#include "vertex-buffer.h"
#include "font.h"

SEN_DECL void
sen_render_node(node_t* node);

SEN_DECL void
sen_render_init();

SEN_DECL void
sen_render_destroy();

SEN_DECL void
sen_render_clear();

SEN_DECL size_t
sen_render_flush(int clear_buff);

SEN_DECL void
sen_render_reload();

SEN_DECL void
sen_render_push_buffer(vertex_buffer_t* buff,
                       const texture_t* tex,
                       const font_t*    font,
                       const shader_t*  program,
                       blend_func blend);

#endif
