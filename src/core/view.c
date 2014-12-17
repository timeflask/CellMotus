#include "view.h"
#include "object.h"
#include "macro.h"
#include "logger.h"
#include "opengl.h"
#include "signals.h"

/*
#ifdef ANDROID
#include <EGL/egl.h>
PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT = 0;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT = 0;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT = 0;

void initExtensions()
{
     glGenVertexArraysOESEXT = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
     sen_assert(glGenVertexArraysOESEXT);
     glBindVertexArrayOESEXT = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
     sen_assert(glBindVertexArrayOESEXT);
     glDeleteVertexArraysOESEXT = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
     sen_assert(glDeleteVertexArraysOESEXT);
}
#endif
*/

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:View"

static const struct gl_context_attr default_attr = {8,8,8,8,24,8};
static struct gl_context_attr g_attrs = {8,8,8,8,24,8};

typedef struct view_t {
  object_t super;
  vec2   s_size;
  vec2   r_size;
  vec2   origin;
  float  scaleX;
  float  scaleY;
  vec4   viewport;
  const void* sig_view_change;
} view_t;

static view_t* g_self = 0;

static int on_resize2(object_t* _null,
                     void* data,
                     object_t* _null2,
                     const char* sig)
{
  UNUSED(_null); UNUSED(_null2);UNUSED(sig);
  sen_assert(data);
  vec2 *size = (vec2 *)data;
  sen_view_set_screen(0,0,size->w, size->h);
  //sen_view_set_screen(0,0,1024, 768);
  return 0;
}


view_t*
view_new ()
{
  struct_malloc(view_t, self);
  sen_object_init(self, "view", 0);
  self->r_size.x = 0;
  self->r_size.y = 0;
  self->s_size.x = 0;
  self->s_size.y = 0;
  self->scaleX = 1;
  self->scaleY = 1;

  self->origin.x = 0;
  self->origin.y = 0;

  self->viewport.x = 0;
  self->viewport.y = 0;
  self->viewport.z = 1;
  self->viewport.w = 1;

  self->sig_view_change = sen_signal_get("view_change", (object_t*)self);

  sen_signal_connect("platform", "resize", &on_resize2, (struct object_t*)self);

  return self;
}


void
view_destroy(view_t* self)
{
  sen_signal_release_emitter((object_t*)self);
  sen_signal_disconnect((object_t*)self, NULL, NULL);
  free(self);
}
//-----------------------------------------------------------------
void
sen_view_set_resolution(float w, float h)
{
  UNUSED(w);UNUSED(h);
}

void
sen_view_set_screen(float x, float y, float w, float h)
{
  if (!g_self) return;

  g_self->r_size.x = w;
  g_self->r_size.y = h;
  g_self->s_size.x = w;
  g_self->s_size.y = h;

  glViewport((GLint)(x),
             (GLint)(y),
             (GLsizei)(w),
             (GLsizei)(h));

  g_self->viewport.x = x;
  g_self->viewport.y = y;
  g_self->viewport.width = w;
  g_self->viewport.height = h;


  sen_signal_emit ( g_self->sig_view_change, &(g_self->viewport));

}

void
sen_view_set_screenv4(const vec4* vp)
{
  sen_view_set_screen(vp->x, vp->y, vp->z, vp->w);
}

void
sen_view_reset()
{
  sen_view_set_screenv4(sen_view_get_viewport());
}
//-----------------------------------------------------------------
const vec4*
sen_view_get_viewport()
{
  sen_assert(g_self);
  return & ( g_self->viewport );
}

const struct gl_context_attr*
sen_view_get_attributes()
{
  return & (g_attrs);
}

void
sen_view_set_attributes(const struct gl_context_attr* attrs)
{
  g_attrs = attrs ? *attrs : default_attr;
}
//-----------------------------------------------------------------

void
sen_views_init()
{
  if (!g_self) {
    _logfi("Create view");
//#ifdef ANDROID
  //  initExtensions();
//#endif
    g_self = view_new();
  }
}

void
sen_views_destroy()
{
  if (g_self) {
    _logfi("Destroy view");
    view_destroy(g_self);
  }

}
