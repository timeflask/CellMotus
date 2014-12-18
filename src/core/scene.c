#include "scene.h"
#include "macro.h"
#include "logger.h"
#include "signals.h"
#include "view.h"
#include "khash.h"
#include "camera.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Scene"


void update_scene_bbox(object_t* _self, const vec4* vp){
  node_t* self = (node_t* )_self;
  scene_t* sself = (scene_t* )_self;
  const vec4* rect = vp ? vp : sen_view_get_viewport();
  camera_t* cam = sen_camera();
  const mat4* mv = sen_node_model(cam);
  float sx =  mv->data[12];
  float sy =  mv->data[13];

  if (!_self) return;

  sen_camera_update_view((object_t*)cam, rect);

  self->bbox.x = -rect->width/2 - sx;
  self->bbox.y = -rect->height/2 - sy;
  self->bbox.z = rect->width/2 - sx;
  self->bbox.w = rect->height/2 - sy;

  sen_signal_emit(sself->sig_resize, & (self->bbox));

}

static int on_view_change(object_t* _self, void* data, object_t* sender, const char* sig)
{
  UNUSED(sender);
  UNUSED(sig);
  sen_assert(data);
  sen_assert(_self);

//  camera_t* cam = sen_camera();
  //_sen_camera_view_change((void*)cam,data, NULL );

  /*
  node_t* self = (node_t* )_self;
  scene_t* sself = (scene_t* )_self;
  vec4* rect = (vec4* )data;

  const mat4* mv = sen_node_model(cam);

  float sx =  mv->data[12];
  float sy =  mv->data[13];

  self->bbox.x = -rect->width/2 - sx;
  self->bbox.y = -rect->height/2 - sy;
  self->bbox.z = rect->width/2 - sx;
  self->bbox.w = rect->height/2 - sy;

  sen_signal_emit(sself->sig_resize, & (self->bbox));
*/
  update_scene_bbox(_self, (const vec4* )data);
  return 0;
}

static khash_t(hmsp)* g_scenes = NULL;

static void scenes_add_global(scene_t* s)
{
  sen_assert(s);
  if (g_scenes == NULL)
    g_scenes = kh_init(hmsp);

  kh_insert(hmsp, g_scenes, obj_name_(s), s);
}
static void scenes_del_global(scene_t* s)
{
  khiter_t pos;
  sen_assert(s);
  if (g_scenes == NULL) return;
  pos = kh_get(hmsp, g_scenes, obj_name_(s));
  if ( pos != kh_end(g_scenes) ) {
    kh_del(hmsp, g_scenes, pos);
    if (kh_size(g_scenes) == 0) {
      kh_destroy(hmsp, g_scenes);
      g_scenes = NULL;
    }
  }
}

scene_t*
sen_scene_new(const char* name)
{
  struct_malloc(scene_t, self);
  _logfi("-scene init");
  sen_node_init((node_t*)self, name, NULL);
  self->sig_resize = sen_signal_get("resize", (object_t*)self);
  on_view_change((object_t*)self, (void*)(sen_view_get_viewport()), NULL, NULL );

  scenes_add_global(self);
  return self;
}

void
sen_scene_delete(void* self)
{
  _logfi("-scene destroy");
  sen_node_clean((node_t*)self);

  sen_signal_release( ((scene_t*)self)->sig_resize);

  scenes_del_global(self);

  free(self);
}



scene_t* g_scene = 0;


void
sen_set_scene_name(const char* name)
{
  khiter_t pos;
  sen_assert(name);
  if (g_scenes == NULL) return;
  pos = kh_get(hmsp, g_scenes, name);
  if ( pos != kh_end(g_scenes) ) {
    sen_set_scene ( kh_val(g_scenes, pos) );
  }
}

static int invalidate_nodes_cb (void* self, node_t* node)
{
  UNUSED(self);
  node->updated |= SEN_NODE_UPDATE_INVALIDATE_BUFFER;
  return 0;
}

void
sen_scenes_reload()
{
  scene_t* scene;
  if (g_scenes == NULL) return;
  kh_foreach_value(g_scenes, scene, sen_node_visit(scene, NULL, &invalidate_nodes_cb) );
}

void
sen_set_scene(scene_t* scene)
{
  if (scene && g_scene != scene) {
    if (g_scene)
      sen_signal_disconnect((object_t*)g_scene, NULL, NULL);
    g_scene = scene;

    update_scene_bbox((object_t*)g_scene , NULL);
    sen_signal_connect("view", "view_change", &on_view_change, (object_t*)g_scene);
    _logfi("CURRENT SCENE => %s", ((object_t*)g_scene)->name);
  }
}

scene_t*
sen_get_scene()
{
  return g_scene;
}
