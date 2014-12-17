#ifndef __node_H_
#define __node_H_
#include "config.h"
#include "object.h"
#include "vector.h"
#include "lmath.h"


typedef struct node_interface_t {
  void (*destroy)(void*);
  void (*render)(void*);
} node_interface_t;

#define SEN_NODE_UPDATE_NONE               0
#define SEN_NODE_UPDATE_MODEL             (1<<0)
#define SEN_NODE_UPDATE_COLOR             (1<<1)
#define SEN_NODE_UPDATE_BBOX              (1<<2)
#define SEN_NODE_UPDATE_INVALIDATE_BUFFER (1<<3)
#define SEN_NODE_UPDATE_PARENT_MODEL      (1<<4)

#define SEN_NODE_UPDATE_ALL (SEN_NODE_UPDATE_MODEL|SEN_NODE_UPDATE_COLOR|SEN_NODE_UPDATE_BBOX|SEN_NODE_UPDATE_PARENT_MODEL)


typedef enum blend_func {
  bfDisable,
  bfPremultiAlpha,
  bfStraightAlpha,
  bfAdditive,
  bfOneOne,
} blend_func ;

typedef struct node_t {
  object_t          super;
  vector_t*         children;

  float             posX;
  float             posY;
  float             posZ;

  float             scaleX;
  float             scaleY;

  float             rotZ_X;
  float             rotZ_Y;

  float             anchorX;
  float             anchorY;

  int               updated;

  mat4              model;
  vec4              color;

  vec4              bbox;
  vec2              size;

  blend_func        blend;


  node_interface_t* vt;
} node_t;

node_t*
sen_node_new();

void
sen_node_destroy(void* self);


void
sen_node_init(void*       _self,
              const char* _name,
              void*       _parent);

void
sen_node_clean(void* _self);

const mat4*
sen_node_model(void* _self);

//---------------------------------------------------------------------------------
void
sen_node_move(void* _self,float dx, float dy);
void
sen_node_Z(void* _self,float z);
void
sen_node_moveTo(void* _self,float x, float y);
void
sen_node_scale(void* _self,float sx, float sy);

void
sen_node_scaleM(void* _self,float mx, float my);

void
sen_node_set_anchor(void* _self, float ax, float ay);

void
sen_node_set_rotation(void* _self, float rotation);

void
sen_node_set_colorRGBA(void* _self, float r, float g, float b, float a);

void
sen_node_set_color(void* _self, vec4* color);

const vec4*
sen_node_bbox(void* _self);

//---------------------------------------------------------------------------------

void
sen_node_add_child(void* self, void* node, int setParent);

void
sen_node_remove_child(void* self, const char* name, void *child, uint32_t child_id);

//---------------------------------------------------------------------------------
typedef int (*node_visitor_callback)(void*, node_t*);

int
sen_node_visit(void* self, void* node_visitor, node_visitor_callback cb);

void
sen_node_sort_children_z(void* self);
//---------------------------------------------------------------------------------

//void
//sen_update_node_children(void* self, float dT);
void
sen_render_node_children(void* self);

size_t
sen_nodes_total();

#endif
