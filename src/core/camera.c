#include "view.h"
#include "camera.h"
#include "macro.h"
#include "logger.h"
#include "opengl.h"
#include "signals.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN::Camera"
/*
static void matrix_log(const mat4* mat) {
  const float* m = mat->data;
  _logfi("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
    m[0],m[1],m[2],m[3],m[4],m[5],m[6],m[7],m[8],m[9],m[10],m[11],m[12],m[13],m[14],m[15]);
  
}
*/
void
sen_camera_update_view(object_t* _self, const vec4* r)
{
  const vec4* rect = r ? r : sen_view_get_viewport();
  camera_t* self = (camera_t* )_self;

  sen_assert(_self);
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
  struct_malloc(camera_t, self);
  _logfi("create camera %s", name ? name : "<none>");
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

