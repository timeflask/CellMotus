#include "view.h"
#include "camera.h"
#include "macro.h"
#include "logger.h"
#include "opengl.h"
#include "signals.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN::Camera"


void
sen_camera_update_view(object_t* _self, const vec4* r)
{
  const vec4* rect = r ? r : sen_view_get_viewport();

  sen_assert(_self);
  camera_t* self = (camera_t* )_self;
  mat4_set_orthographic( &self->proj,
                        -rect->width/2, rect->width/2,
                        -rect->height/2, rect->height/2,
                        -1, 1);


  mat4_set_identity(&self->view_proj);


  mat4_multiply(&self->view_proj, (mat4*)sen_node_model(self));
  mat4_multiply(&self->view_proj, &self->proj);

}
/*
static int
sen_camera_view_change(object_t* _self, void* data, object_t* sender)
{
  UNUSED(sender);
  sen_assert(data);
  sen_assert(_self);
  sen_camera_update_view(_self, (vec4* )data);

  return 0;
}
*/
camera_t*
sen_camera_new(const char* name)
{
  _logfi("create camera %s", name ? name : "<none>");
  struct_malloc(camera_t, self);
  sen_node_init( self, name, NULL);
  mat4_set_identity( & (self->proj) );
  mat4_set_identity( & (self->view_proj) );

  sen_camera_update_view((object_t*) self, NULL);
  return self;
}

void
sen_camera_destroy(camera_t* self)
{
  sen_node_clean(self);
  free(self); self = NULL;
}

