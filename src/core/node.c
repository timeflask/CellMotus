#include "node.h"
#include "macro.h"
#include "logger.h"
#include "render.h"
#include "lmath.h"
#include <math.h>
#include <string.h>


node_t*
sen_node_new()
{
  struct_malloc(node_t, self);
  return self;
}

void
sen_node_destroy(void* self)
{
  //_logfi("-node destroy");
  sen_node_clean((node_t*)self);
  free(self);
}

static node_interface_t node_interface =
{
    &sen_node_destroy, &sen_render_node_children
};

static size_t g_total_nodes = 0;
void
sen_node_init(void*       _self,
              const char* _name,
              void*       _parent)
{
  node_t* self = (node_t*)_self;
  sen_assert(self);
  sen_object_init(self, _name, _parent);
  self->children = 0; //vector_new( sizeof(node_t*) );
  self->posX = 0.0f;
  self->posY = 0.0f;
  self->posZ = 0.0f;
  self->scaleX = 1.0f;
  self->scaleY = 1.0f;
  self->rotZ_X = 0.0f;
  self->rotZ_Y = 0.0f;
  self->anchorX = 0;
  self->anchorY = 0;

  self->color.r = 1;
  self->color.g = 1;
  self->color.b = 1;
  self->color.a = 1;

  self->size.x = 1;
  self->size.y = 1;

  self->bbox.data[0] = -1;
  self->bbox.data[1] = -1;
  self->bbox.data[2] = 1;
  self->bbox.data[3] = 1;

  self->blend = bfStraightAlpha;


  mat4_set_identity(&self->model);

  self->vt = &node_interface;
      //.destroy = &sen_node_destroy;
  //self->vt.render = &sen_render_node_children;//&sen_update_node_children;

  self->updated = SEN_NODE_UPDATE_ALL;
  g_total_nodes++;
}


void
sen_node_clean(void* _self)
{
  node_t* self = (node_t*)_self;
  sen_assert(self);
  //_logfi(" -Node clear [%s]", (char*) self->super.name);
  if (self->children) {
    size_t i;
    for (i = 0; i<self->children->size; ++i) {
      node_t* node = *(node_t**)vector_get(self->children, i);
      if ( ((object_t*)node)->parent == (object_t*) _self && node->vt && node->vt->destroy )
        node->vt->destroy(node);
    }
    vector_delete(self->children);
    self->children = NULL;
  }
  g_total_nodes--;
}

size_t
sen_nodes_total()
{
  return g_total_nodes;
}

void
sen_node_set_anchor(void* _self, float ax, float ay)
{
  node_t* self = (node_t*)_self;
//  self->anchorX = ax / self->size.x;
//  self->anchorY = ay / self->size.y;
  self->anchorX = ax;
  self->anchorY = ay;
  self->updated |= SEN_NODE_UPDATE_MODEL;
}

const mat4*
sen_node_model(void* _self)
{
  vec2 ap;
  node_t* self = (node_t*)_self;
  float* data;
  object_t* parent;

  if (self->updated&SEN_NODE_UPDATE_MODEL) {

  //  _logfi(".%f .%f", self->rotZ_X,self->posY);
/*
    mat4 t;
    mat4_set_translation( &t, self->posX,self->posY, self->posZ);
    mat4 a;
    mat4_set_translation( &a, self->anchorX,self->anchorY, 0);
    mat4 r;
    mat4_set_rotation( &r, self->rotZ_X, 0, 0, -1);
    mat4 s;
    mat4_set_scaling( &s,self->scaleX, self->scaleY, 1  );


    mat4_multiply2( &s, &a, &self->model);
    mat4_multiply( &self->model, &r);
    mat4_multiply( &self->model, &t);
*/


    //float x = self->posX + self->anchorX;
    //float y = self->posY + self->anchorY;
    float x = self->posX ;
    float y = self->posY ;
    float z = self->posZ;
    float cx = 1, sx = 0, cy = 1, sy = 0;
    if (self->rotZ_X || self->rotZ_Y)
    {
        float radiansX = -SEN_DEG_TO_RAD(self->rotZ_X);
        float radiansY = -SEN_DEG_TO_RAD(self->rotZ_Y);
        cx = cosf(radiansX);
        sx = sinf(radiansX);
        cy = cosf(radiansY);
        sy = sinf(radiansY);
    }

    ap.x = self->anchorX * self->scaleX;
    ap.y = self->anchorY * self->scaleY;

    x += cy * -ap.x + -sx * -ap.y;
    y += sy * -ap.x +  cx * -ap.y;


    data = self->model.data;
    data[0]  = cy * self->scaleX;  data[1]  = sy * self->scaleX; data[2]  = 0; data[3]  = 0;
    data[4]  = -sx * self->scaleY; data[5]  = cx * self->scaleY; data[6]  = 0; data[7]  = 0;
    data[8]  = 0;                  data[9]  = 0;                 data[10] = 1; data[11] = 0;
    data[12] = x;                  data[13] = y;                 data[14] = z; data[15] = 1;
    //data[14] = z;

//_logfi("%.f %.f ", ap.x, ap.y);
    //mat4_translate( &(self->model),ap.x,ap.y, 0);
    //mat4_translate( &(self->model),-ap.x,-ap.y, 0);

    parent =  (((object_t*)_self)->parent);
    if (parent) {
      mat4* parent_model = (mat4* )sen_node_model(parent);
      mat4_multiply( &(self->model) , parent_model );
    }
    //node_t* parent = (((object_t*)_self)->parent);

    self->updated &= ~ (SEN_NODE_UPDATE_MODEL | SEN_NODE_UPDATE_PARENT_MODEL);
  }

  if (self->updated&SEN_NODE_UPDATE_PARENT_MODEL ) {
    object_t* parent =  (((object_t*)_self)->parent);
    if (parent) {
      mat4* parent_model = (mat4* )sen_node_model(parent);
      mat4_multiply( &(self->model) , parent_model );
    }
    self->updated &= ~ SEN_NODE_UPDATE_PARENT_MODEL;
  }
  return & (self->model);
}

static void
sen_node_set_update_model(void* _self)
{
  node_t* self = (node_t*)_self;
  size_t i;
  self->updated |= SEN_NODE_UPDATE_MODEL;
  if (!self->children) return ;
  for (i = 0; i<self->children->size; ++i) {
    node_t* node = *(node_t**)vector_get(self->children, i);
    sen_node_set_update_model(node);
  }
}


void
sen_node_set_rotation(void* _self, float rotation)
{
  node_t* self = (node_t*)_self;
  //if (self->rotZ_X == rotation) return;
  self->rotZ_X = rotation;
  self->rotZ_Y = rotation;
  //_logfi(".%f", self->rotZ_X);
  sen_node_set_update_model(_self);
//  self->updated |= SEN_NODE_UPDATE_MODEL;
}


void
sen_node_set_color(void* _self,vec4* color)
{
  node_t* self = (node_t*)_self;
  vec4_set( & (self->color) , color);
  self->updated |= SEN_NODE_UPDATE_COLOR;
}

void
sen_node_set_colorRGBA(void* _self, float r, float g, float b, float a)
{
  node_t* self = (node_t*)_self;
  self->color.r = r;
  self->color.g = g;
  self->color.b = b;
  self->color.a = a;
  self->updated |= SEN_NODE_UPDATE_COLOR;
}



static int
zcmp (const void *pn1, const void *pn2)
{
  float z1 = (*(node_t**)pn1)->posZ;
  float z2 = (*(node_t**)pn2)->posZ;
 // if (z1 != 0.00f || z2!= 0.00f)
 //_logfi("%f %f", z1, z2);
  if (z1<z2) return -1;
  if (z1>z2) return 1;
  return -1;
}

void
sen_node_sort_children_z(void* _self)
{
  node_t* self = (node_t*)_self;
  vector_sort( self->children, &zcmp );
}

void
sen_render_node_children(void* _self)
{
  size_t i;
  node_t* self = (node_t*)_self;
  sen_assert(_self);

  if (!self->children) return;
  //sen_node_sort_children_z(_self);
  for (i = 0; i<self->children->size; ++i)
    sen_render_node( *(node_t**)vector_get(self->children, i) );
}

void
sen_node_add_child(void* _self, void* node, int setParent)
{
  node_t* self = (node_t*)_self;

  sen_assert(_self);
  sen_assert(node);

  if (!self->children)
    self->children = vector_new( sizeof (node_t*) );

  if (setParent)
    ((object_t*)node)->parent = (object_t*)self;

 // _logfi("Node:[%s]=>[%s]", ((object_t*)node)->name, ((object_t*)_self)->name );
  vector_push_back(self->children, &node); sen_node_sort_children_z(_self);
  ((node_t*)node)->updated |= SEN_NODE_UPDATE_MODEL|SEN_NODE_UPDATE_PARENT_MODEL;
}

void
sen_node_remove_child(void* _self, const char* name, void *obj, uint32_t child_id)
{
  node_t* self = (node_t*)_self;
  size_t i;

  sen_assert(_self);

  if (!self->children) return;

  if (name == NULL && obj == NULL && child_id == 0) {
    _logfi("Node clear [%s]", ((object_t*)_self)->name );
    for (i = 0; i<self->children->size; ++i) {
      node_t* child =   *(node_t**)vector_get(self->children, i) ;
      if (((object_t*)child)->parent == (object_t*)self )
          child->vt->destroy(child);
    }
    vector_clear(self->children);
    _logfi("Done clearing [%s]", ((object_t*)_self)->name );
    //self->children = NULL;
  }
  else
    for (i = 0; i<self->children->size; ++i) {
      node_t* child =   *(node_t**)vector_get(self->children, i) ;
      if ( (name && strcmp(((object_t*)child)->name, name) == 0) ||
           (obj && obj == child) ||
           (child_id && ((object_t*)child)->uid == child_id) ) {
        //_logfi("Node DEL:[%s] from [%s]", name, ((object_t*)_self)->name );
        if (((object_t*)child)->parent == (object_t*)self )
          child->vt->destroy(child);
        vector_erase(self->children, i);
        break;
      }
    }
}

int
sen_node_visit(void* _self, void* node_visitor, node_visitor_callback cb)
{
  size_t i;
  node_t* self = (node_t*)_self;
  sen_assert(_self);
  if ( (*cb)(node_visitor,self) ) return 1;
  if (!self->children) return 0;
  for (i = 0; i<self->children->size; ++i) {
    node_t* node = *(node_t**)vector_get(self->children, i);
    if ( sen_node_visit(node, node_visitor, cb) ) return 1;
  }
  return 0;
}

/*
void
sen_update_node_children(void* _self, float dT)
{
  sen_assert(_self);
  node_t* self = (node_t*)_self;
  size_t i;
  for (i = 0; i<self->children->size; ++i) {
    node_t* node = *(node_t**)vector_get(self->children, i);
    if ( node->vt.update ) node->vt.update(node, dT);
  }
}
*/

const vec4*
sen_node_bbox(void* _self)
{
  return & ( ((node_t*)_self)->bbox );
}

void
sen_node_move(void* _self,float dx, float dy)
{
  node_t* self = (node_t*)_self;
  self->posX+=dx; self->posY+=dy;

//  self->updated |= SEN_NODE_UPDATE_MODEL;
  sen_node_set_update_model(_self);

}

void
sen_node_moveTo(void* _self,float x, float y)
{
  node_t* self = (node_t*)_self;
  self->posX=x;
  self->posY=y;
  sen_node_set_update_model(_self);

//  self->updated |= SEN_NODE_UPDATE_MODEL;
}

void
sen_node_Z(void* _self,float z)
{
  object_t* parent;
  node_t* self = (node_t*)_self;
 // if (self->posZ!=z) {
    self->posZ=z;
    parent =  (((object_t*)_self)->parent);
    if (parent) {
      sen_node_sort_children_z(parent);
    }
   // self->updated &= ~ SEN_NODE_UPDATE_PARENT_MODEL;

    sen_node_set_update_model(_self);

//    self->updated |= SEN_NODE_UPDATE_MODEL;
  //}
}


void
sen_node_scale(void* _self,float sx, float sy)
{
  node_t* self = (node_t*)_self;
  self->scaleX=sx; self->scaleY=sy;

  sen_node_set_update_model(_self);
 //self->updated |= SEN_NODE_UPDATE_MODEL;

}

void
sen_node_scaleM(void* _self,float mx, float my)
{
  node_t* self = (node_t*)_self;
  self->scaleX*=mx; self->scaleY*=my;

  sen_node_set_update_model(_self);

//  self->updated |= SEN_NODE_UPDATE_MODEL;
}


