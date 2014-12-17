#ifndef __scene_H_
#define __scene_H_
#include "config.h"
#include "node.h"

typedef struct scene_t {
  node_t super;
  const void*  sig_resize;
} scene_t;

scene_t*
sen_scene_new(const char* name);

void
sen_scene_delete(void* self);



void
sen_set_scene(scene_t* scene);

void
sen_set_scene_name(const char* name);

void
update_scene_bbox(object_t* _self, const vec4* vp);

scene_t*
sen_get_scene();

void
sen_scenes_reload();

#endif
