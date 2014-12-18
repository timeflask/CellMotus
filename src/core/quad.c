#include "quad.h"
#include "macro.h"
#include "shader.h"
#include "macro.h"
#include "logger.h"
#include "camera.h"
#include "string.h"
#include "render.h"
#include "view.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Quad"

static const GLushort  indices[6] = {0,1,2, 0,2,3};
static void
update_buffer(cquad_t* self)
{
  int status  = ((node_t*)self)->updated;
  const vec4* col = &(((node_t*)self)->color);
  float r = col->r;
  float g = col->g;
  float b = col->b;
  float a = col->a;
  vec4* bbox = & ( ((node_t*) self)->bbox );
  vec2* size = & ( ((node_t*) self)->size );
  float sw = 0.5f; float sh = 0.5f;
  V4F_C4F vertices[4];
  int update_model;

  if (status & SEN_NODE_UPDATE_INVALIDATE_BUFFER) {
    vertex_buffer_invalidate(self->quad);
    status &= ~SEN_NODE_UPDATE_INVALIDATE_BUFFER;
  }


  if (status & SEN_NODE_UPDATE_BBOX) {
    vertex_buffer_clear(self->quad);

    /*
    V4F_C4F vertices[4] = {{ -sw,-sh,0,1,  col->r,col->g,col->b,col->a,},
                           { -sw, sh,0,1, col->r,col->g,col->b,col->a,},
                           {  sw, sh,0,1, col->r,col->g,col->b,col->a,},
                           {  sw,-sh,0,1, col->r,col->g,col->b,col->a,}};
                           */

    vertices[0].x = -sw;
    vertices[0].y = -sh;
    vertices[0].z = 0.0f;
    vertices[0].w = 1.0f;
    vertices[0].r = r;
    vertices[0].g = g;
    vertices[0].b = b;
    vertices[0].a = a;


    vertices[1].x = -sw;
    vertices[1].y = sh;
    vertices[1].z = 0.0f;
    vertices[1].w = 1.0f;
    vertices[1].r = r;
    vertices[1].g = g;
    vertices[1].b = b;
    vertices[1].a = a;

    vertices[2].x = sw;
    vertices[2].y = sh;
    vertices[2].z = 0.0f;
    vertices[2].w = 1.0f;
    vertices[2].r = r;
    vertices[2].g = g;
    vertices[2].b = b;
    vertices[2].a = a;

    vertices[3].x = sw;
    vertices[3].y = -sh;
    vertices[3].z = 0.0f;
    vertices[3].w = 1.0f;
    vertices[3].r = r;
    vertices[3].g = g;
    vertices[3].b = b;
    vertices[3].a = a;

    bbox->x = -sw; bbox->y = -sh;  bbox->z = sw; bbox->w = sh;
    size->x = 1;
    size->y = 1;

    vertex_buffer_push_back( self->quad, vertices, 4, indices, 6 );
    memcpy(self->vertices, vertices, sizeof(vertices));
    status &= ~SEN_NODE_UPDATE_BBOX;
    status |= SEN_NODE_UPDATE_MODEL;
  }

  update_model = status & SEN_NODE_UPDATE_MODEL;

  if ( status & (SEN_NODE_UPDATE_MODEL | SEN_NODE_UPDATE_COLOR)) {

    const mat4* model = sen_node_model(self);

    int i;
    for (i=0; i<4; ++i) {
      if (update_model) {
        vec4* vp =  (vec4*) (& (self->vertices[i]) );
        vec4* v =   (vec4*) ((char*)(self->quad->vertices->items) + i*sizeof(V4F_C4F) );
        v4_transform(model->data, vp->data,v->data);

        if ( i ) {
          if (v->x < bbox->x) bbox->x = v->x; else if (v->x > bbox->z) bbox->z = v->x;
          if (v->y < bbox->y) bbox->y = v->y; else if (v->y > bbox->w) bbox->w = v->y;
        }
        else {
          bbox->x = bbox->z = v->x;
          bbox->y = bbox->w = v->y;
        }
      }

      if (status & SEN_NODE_UPDATE_COLOR) {
        //
        vec4* c =  (vec4*) ( (char*)(self->quad->vertices->items) +
                             i*sizeof(V4F_C4F )+
                             sizeof(float)*4 );
        memcpy(c,col,sizeof(vec4));
      }
    }
    self->quad->state = 1;
    status &= ~(SEN_NODE_UPDATE_COLOR|SEN_NODE_UPDATE_MODEL|SEN_NODE_UPDATE_PARENT_MODEL);
  }

  ((node_t*)self)->updated = status;
}


static node_interface_t quad_interface = {
    &sen_quad_delete, &sen_quad_render
};

cquad_t*
sen_quad_new(const char* name)
{
  struct_malloc(cquad_t, self);
  sen_node_init(self, name, 0);
  self->program = sen_shaders_get("quad");
  sen_assert(self->program);
  self->quad = vertex_buffer_new("a_pos:4f,a_color:4f");

  update_buffer(self);

  ((node_t*)self)->vt = &quad_interface;

  return self;
}

void
sen_quad_delete(void *_self)
{
  cquad_t *self = (cquad_t *)_self;
  int status;
  sen_assert(_self);
  sen_shaders_release(self->program);
  status  = ((node_t*)self)->updated;
  if (status & SEN_NODE_UPDATE_INVALIDATE_BUFFER) {
    vertex_buffer_invalidate(self->quad);
  }
  vertex_buffer_delete(self->quad);
  sen_node_clean(self);
  free(self);
}

void
sen_quad_render(void* _self)
{
  cquad_t *self = (cquad_t *)_self;
  vec4* bbox;
  const vec4* vp;
  vec4* v;
  int use_cam;
  camera_t* cam =  sen_camera();
  node_t* cam_node = (node_t* ) cam;
  const mat4* mv = &(cam_node->model);

  sen_assert(_self);

  //node_t* node = (node_t*) _self;
  if ( ((node_t*)self)->color.a < F_EPSILON ) return;
  update_buffer(self);

  bbox = & ( ((node_t*) self)->bbox );
  vp = sen_view_get_viewport();

  v = (vec4*)self->quad->vertices->items;
  use_cam = v->z < 0.9 && v->z > -0.9 ;

  if (use_cam) {
    float sx =   mv->data[12];
    float sy =  mv->data[13] ;

    float vw = (vp->z - vp->x) / 2.0f ;
    float vh = (vp->w - vp->y) / 2.0f ;

    if (bbox->z+ sx< -vw  ||
        bbox->x+ sx>  vw  ||
        bbox->w+ sy< -vh  ||
        bbox->y+ sy>  vh  )
    {
      return;
    }
  }
  sen_render_push_buffer(self->quad, NULL, NULL, self->program, ((node_t*)self)->blend);
  /*
  sen_shader_use(self->program);
  {
    sen_uniform_m4fN(self->program, "u_mvp",  cam->view_proj.data);
    vertex_buffer_render( self->quad, GL_TRIANGLES);
  }
*/
}
//-------------------------------------------------------------------------------------------------
