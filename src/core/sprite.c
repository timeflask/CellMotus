#include "sprite.h"
#include "macro.h"
#include "texture-manager.h"
#include "shader.h"
#include "macro.h"
#include "logger.h"
#include "camera.h"
#include "string.h"
#include "render.h"
#include "view.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Sprite"

/*
void
update_quad(sprite_t* self)
{
  vertex_buffer_clear(self->quad);
  GLushort    indices[6] = {0,1,2, 0,2,3};
  const vec4* coords = &(self->tex->coords);
  const vec4* col = &(((node_t*)self)->color);
  float sw = self->tex->img_width / 2.0f;
  float sh = self->tex->img_height / 2.0f;



  V4F_T2F_C4F vertices[4] = {{ -sw,-sh,0,1, coords->s0,coords->t1, col->r,col->g,col->b,col->a,},
                            {  -sw, sh,0,1, coords->s0,coords->t0, col->r,col->g,col->b,col->a,},
                            {   sw, sh,0,1, coords->s1,coords->t0, col->r,col->g,col->b,col->a,},
                            {   sw,-sh,0,1, coords->s1,coords->t1, col->r,col->g,col->b,col->a,}};

  vec4* bbox = & ( ((node_t*) self)->bbox );
  bbox->x = -sw; bbox->y = -sh;  bbox->z = sw; bbox->w = sh;

  vertex_buffer_push_back( self->quad, vertices, 4, indices, 6 );
  ((node_t*)self)->updated &= ~ (SEN_NODE_UPDATE_BBOX | SEN_NODE_UPDATE_COLOR );
}
*/

static const GLushort  indices[6] = {0,1,2, 0,2,3};
static void
update_buffer(sprite_t* self)
{
  int status  = ((node_t*)self)->updated;

  if (status & SEN_NODE_UPDATE_INVALIDATE_BUFFER) {
    vertex_buffer_invalidate(self->quad);
    //vertex_buffer_delete(self->quad);
    //self->quad = vertex_buffer_new("a_pos:4f,a_tex_coords:2f,a_color:4f");
    status &= ~SEN_NODE_UPDATE_INVALIDATE_BUFFER;
    //status= SEN_NODE_UPDATE_ALL;
  }

  const vec4* col = &(((node_t*)self)->color);
  vec4* bbox = & ( ((node_t*) self)->bbox );
  vec2* size = & ( ((node_t*) self)->size );

  if (status & SEN_NODE_UPDATE_BBOX) {
    vertex_buffer_clear(self->quad);

    const vec4* coords = &(self->tex->coords);

    float sw = self->tex->img_width / 2.0f;
    float sh = self->tex->img_height / 2.0f;



    V4F_T2F_C4F vertices[4] = {{ -sw,-sh,0,1, coords->s0,coords->t1, col->r,col->g,col->b,col->a,},
                              {  -sw, sh,0,1, coords->s0,coords->t0, col->r,col->g,col->b,col->a,},
                              {   sw, sh,0,1, coords->s1,coords->t0, col->r,col->g,col->b,col->a,},
                              {   sw,-sh,0,1, coords->s1,coords->t1, col->r,col->g,col->b,col->a,}};


    bbox->x = -sw; bbox->y = -sh;  bbox->z = sw; bbox->w = sh;
    size->x = self->tex->img_width;
    size->y = self->tex->img_height;

    vertex_buffer_push_back( self->quad, vertices, 4, indices, 6 );
   // vertex_buffer_push_back_vertices( self->quad, vertices, 4);
    memcpy(self->vertices, vertices, sizeof(vertices));
    status &= ~SEN_NODE_UPDATE_BBOX;
    status |= SEN_NODE_UPDATE_MODEL;
  }

  int update_model = status & SEN_NODE_UPDATE_MODEL;

  if ( status & (SEN_NODE_UPDATE_MODEL | SEN_NODE_UPDATE_COLOR)) {
    //V4F_T2F_C4F vertices[4];
//    memcpy(vertices, self->vertices, 4*sizeof(V4F_T2F_C4F));

    const mat4* model = sen_node_model(self);

    int i;
    for (i=0; i<4; ++i) {
      if (update_model) {
//        vec4* v =  (vec4*) ( i*sizeof(V4F_T2F_C4F )+(self->quad->vertices->items));
        //vec4* v =  (vec4*) ( i*sizeof(V4F_T2F_C4F )+(vertices));
        //vec4* v =  (vec4*)  (& vertices[i]);// ( i*sizeof(V4F_T2F_C4F )+(vertices));
        vec4* vp =  (vec4*)  (& (self->vertices[i]) );
        vec4* v =   ( i*sizeof(V4F_T2F_C4F )+(self->quad->vertices->items));
        //v4_transform(model->data, v->data,v->data);
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
        vec4* c =  (vec4*) ( i*sizeof(V4F_T2F_C4F )+sizeof(float)*6+(self->quad->vertices->items));
//        vec4* c =  (vec4*) ( i*sizeof(V4F_T2F_C4F )+sizeof(float)*6+(vertices));
        //vec4* c =  (vec4*) (& (vertices[i].r) );//  i*sizeof(V4F_T2F_C4F )+sizeof(float)*6+(vertices));
//        vec4* c =  (vec4*) (& (self->vertices[i].r) );//  i*sizeof(V4F_T2F_C4F )+sizeof(float)*6+(vertices));
        memcpy(c,col,sizeof(vec4));
      }
    }
    //self->quad->state = 1;
    status &= ~(SEN_NODE_UPDATE_COLOR|SEN_NODE_UPDATE_MODEL|SEN_NODE_UPDATE_PARENT_MODEL);
  }

  ((node_t*)self)->updated = status;
}


static node_interface_t sprite_interface = {
    &sen_sprite_delete, &sen_sprite_render
};

sprite_t*
sen_sprite_new(const char* name,
               const char* texture_name)
{
 // _logfi("+sprite %s, texture=%s", (name ? name : "autoName"), texture_name);
  struct_malloc(sprite_t, self);
  sen_node_init(self, name, 0);
  self->tex = sen_textures_get(texture_name);
  sen_assert(self->tex);
  self->program = sen_shaders_get("sprite");
  sen_assert(self->program);
  self->quad = vertex_buffer_new("a_pos:4f,a_tex_coords:2f,a_color:4f");

  //update_quad(self);
  update_buffer(self);

  ((node_t*)self)->vt = &sprite_interface;

  return self;
}

void
sen_sprite_delete(void *_self)
{
  sen_assert(_self);
  sprite_t *self = (sprite_t *)_self;
 // _logfi("-sprite %s", ((object_t*)self)->name);
  sen_textures_release(self->tex);
  sen_shaders_release(self->program);
  int status  = ((node_t*)self)->updated;
  if (status & SEN_NODE_UPDATE_INVALIDATE_BUFFER) {
    vertex_buffer_invalidate(self->quad);
  }
  vertex_buffer_delete(self->quad);
  sen_node_clean(self);
  free(self);
}

void
sen_sprite_render(void* _self)
{
  sen_assert(_self);
  sprite_t *self = (sprite_t *)_self;

  if ( ((node_t*)self)->color.a < F_EPSILON ) return;
  update_buffer(self);

  vec4* bbox = & ( ((node_t*) self)->bbox );
  const vec4* vp = sen_view_get_viewport();

  //const mat4* mv = sen_node_model(sen_camera());

  node_t* cam_node = (node_t* ) sen_camera();
  const mat4* mv = &(cam_node->model);

  vec4* v = (vec4*)self->quad->vertices->items;
  int use_cam = v->z < 0.9 && v->z > -0.9 ;

  if (use_cam) {
    float sx =  mv->data[12];
    float sy =  mv->data[13];

    float vw = (vp->z - vp->x) / 2.0 ;
    float vh = (vp->w - vp->y) / 2.0 ;
    //if (bbox->x > 250) {
    //_logfi("%.f %.f %.f %.f", vp->x, vp->y, vp->z, vp->w);
    //_logfi("%.f %.f %.f %.f", bbox->x, bbox->y, bbox->z, bbox->w);
    //_logfi("--------------------------------------------");
    //}

    if (bbox->z+ sx< -vw  ||
        bbox->x+ sx>  vw  ||
        bbox->w+ sy< -vh  ||
        bbox->y+ sy>  vh  )
    {
      //_logfi("NO");
      return;
    }
  }

  sen_render_push_buffer(self->quad, self->tex, NULL, self->program, ((node_t*)self)->blend);
  /*
  int status  = ((node_t*)self)->updated;

  if (status & SEN_NODE_UPDATE_INVALIDATE_BUFFER) {
   // _logfi("invalidate %s", ((object_t*)self)->name );
    vertex_buffer_invalidate(self->quad);
    ((node_t*)self)->updated &= ~SEN_NODE_UPDATE_INVALIDATE_BUFFER;
  }

  if ( status & (SEN_NODE_UPDATE_BBOX | SEN_NODE_UPDATE_COLOR )) {
    update_quad(self);
  //  status |= (SEN_NODE_UPDATE_MODEL);
  }
*/
  /*
  update_buffer(self);

  sen_texture_bind(self->tex);
  mat4 mvp;
 // mat4_set_identity(mvp.data);
  camera_t* cam = sen_camera();

  //mat4_multiply2(sen_node_model(self), & cam->view_proj, &mvp);
  //mat4_set_identity(&mvp);
  sen_shader_use(self->program);
  {
    sen_uniform_1iN(self->program, "u_tex0", 0);
    //sen_uniform_m4fN(self->program, "u_mvp",  mvp.data);
    sen_uniform_m4fN(self->program, "u_mvp",  cam->view_proj.data);
    vertex_buffer_render( self->quad, GL_TRIANGLES);
  }
  */
}
