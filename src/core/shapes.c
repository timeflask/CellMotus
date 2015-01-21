#include "shapes.h"
#include "macro.h"
#include "logger.h"
#include "shader.h"
#include "vertex-buffer.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Shapes"


typedef struct {
    float x, y, z, w;
    float r, g, b, a;
} V3F_C4F;

static const shader_t* g_shader = NULL;
static vertex_buffer_t* g_buffer = NULL;

void
sen_shapes_init()
{
  _logfi("init shapes data");
  sen_shaders_load("shape", "vs_pos4f_col4f", "fs_col4f");
  g_shader =  sen_shaders_get("shape");
  sen_assert(g_shader);
  g_buffer = vertex_buffer_new("a_pos:4f,a_color:4f");
}

void
sen_shapes_destroy()
{
  _logfi("deinit shapes data");
  sen_shaders_release(g_shader);
  g_shader = NULL;
}

void
sen_shapes_reload()
{
  _logfi("*** RELOADING SHAPES *** ");
  vertex_buffer_invalidate(g_buffer);
  _logfi("*** DONE *** ");
}

void
sen_shapes_line(const vec2* start, const vec2* end)
{
  V3F_C4F vertices[2] = { { start->x, start->y, 0, 1, 1,1,1,1 },
                          { end->x, end->y, 0,1, 1,1,1,1 },
  };
  mat4 mvp; 

  vertex_buffer_clear(g_buffer);
  vertex_buffer_push_back_vertices(g_buffer, vertices, 2);

  mat4_set_identity(&mvp);
  sen_shader_use(g_shader);
  {
    sen_uniform_m4fN(g_shader, "u_mvp",  mvp.data);
   // sen_uniform_1fN(g_shader, "u_pointSize",  2.0f);
    vertex_buffer_render( g_buffer, GL_LINES);
  }
  gl_check_error();
}
