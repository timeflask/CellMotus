#ifndef __render_H_
#define __render_H_
#include "config.h"
#include "node.h"
#include "texture.h"
#include "shader.h"
#include "vertex-buffer.h"
#include "font.h"

void
sen_render_node(node_t* node);

void
sen_render_init();

void
sen_render_destroy();

void
sen_render_clear();

size_t
sen_render_flush(int clear_buff);

void
sen_render_reload();

void
sen_render_push_buffer(vertex_buffer_t* buff,
                       const texture_t* tex,
                       const font_t*    font,
                       const shader_t*  program,
                       blend_func blend);

#endif
