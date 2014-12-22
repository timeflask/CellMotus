#include "render.h"
#include "macro.h"
#include "logger.h"
#include "lmath.h"
#include "opengl.h"
#include "scene.h"
#include "vector.h"
#include "khash.h"
#include "camera.h"
#include "utils.h"
#include <stdio.h>

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Render"

void
sen_render_node(node_t* node)
{
  if (node && node->vt->render)
    (*(node->vt->render))(node);
}


void
sen_render_clear()
{
  scene_t* scene = sen_get_scene();
  if (scene) {
    vec4* color =&  ((node_t*)scene)->color;
    glClearColor( color->x, color->y, color->z, color->w );
  }
  else
    glClearColor( 1, 0.1f, 0.1f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT );
  //glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  //glEnable( GL_BLEND );
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );


  glDisable( GL_DEPTH_TEST );
  glDepthMask( 0 );

}

struct tex_group_t;
struct blend_group_t;
typedef struct shader_group_t
{
  const shader_t*  program;
  vertex_buffer_t* buff;
  int              num;
  char*            name;
  int              z;
  struct blend_group_t*   bg;
  struct tex_group_t*     tg;
}shader_group_t;

shader_group_t*
shader_group_new(const shader_t* program, const vertex_buffer_t* buff, const char* name, int z,
    struct tex_group_t* tg, struct blend_group_t* bg)
{
  struct_malloc(shader_group_t, self);
  self->program = program;
  self->name = sen_strdup(name);
  self->buff = vertex_buffer_new(vertex_buffer_format(buff));
  self->num = 0;
  self->z = z;
  self->tg = tg;
  self->bg = bg;
  return self;
}

void
shader_group_delete(shader_group_t* self)
{
  sen_assert(self);
  vertex_buffer_delete(self->buff);
  free(self->name);
  free(self);
}

typedef struct tex_group_t
{
  const texture_t* tex;
  const font_t*    font;
  khash_t(hmsp)*   sgs;
  int              num;
}tex_group_t;

tex_group_t*
tex_group_new(const texture_t* tex, const font_t* font)
{
  struct_malloc(tex_group_t, self);
  self->tex = tex;
  self->font = font;
  self->sgs = kh_init(hmsp);
  self->num = 0;
  return self;
}

void
tex_group_delete(tex_group_t* self)
{
  shader_group_t* sg;
  sen_assert(self);
  kh_foreach_value(self->sgs, sg, shader_group_delete(sg));
  kh_destroy(hmsp, self->sgs);
  free(self);
}

typedef struct blend_group_t
{
  khash_t(hmsp)*   tgs;
  int              num;
  unsigned int              key;
}blend_group_t;

blend_group_t*
blend_group_new(unsigned int key)
{
  struct_malloc(blend_group_t, self);
  self->tgs = kh_init(hmsp);
  self->num = 0;
  self->key = key;
  return self;
}

void
blend_group_delete(blend_group_t* self)
{
  tex_group_t* tg;
  sen_assert(self);
  kh_foreach_value(self->tgs, tg, tex_group_delete(tg));
  kh_destroy(hmsp, self->tgs);

  free(self);
}

//static khash_t(hmsp)* g_tgs = NULL;
static khash_t(hmip)* g_bgs = NULL;
static const char* NO_ATLAS = "NO_ATLAS";
vertex_buffer_t*
get_group_buffer(vertex_buffer_t* buff,
                 const texture_t* tex,
                 const font_t*    font,
                 const shader_t*  program,
                 blend_func blend)
{
  tex_group_t* tg;
  blend_group_t* bg;
  unsigned int bkey;
  khiter_t i,j;
  khash_t(hmsp)* tgs;
  const char* atlas_name = NO_ATLAS;
  char buffer [128];
  vec4* v;
  int z;
  shader_group_t* sg;
  khash_t(hmsp)*  sgs;

  if (tex)
    atlas_name = sen_texture_atlas(tex);
  else if (font)
    atlas_name = sen_font_atlas(font);

  bkey =  (unsigned int)blend;

  i = kh_get(hmip, g_bgs, bkey);

  if (i != kh_end(g_bgs))
    bg = kh_val(g_bgs, i);
  else {
    bg =  blend_group_new(bkey);
    kh_insert(hmip, g_bgs, bkey, bg);
  }
  bg->num++;

  tgs = bg->tgs;

  j = kh_get(hmsp, tgs, atlas_name);
  if (j != kh_end(tgs))
    tg = kh_val(tgs, j);
  else {
    tg = tex_group_new(tex,font);
    kh_insert(hmsp, tgs, atlas_name, tg);
  }

  tg->num++;
  v = (vec4*) buff->vertices->items;
  z = (int) (v->z * 10000);

  sprintf (buffer, "%05d%s",z,program->name);
  sgs = tg->sgs;
  i = kh_get(hmsp, sgs, buffer);
  if (i != kh_end(sgs))
    sg = kh_val(sgs, i);
  else {
    sg = shader_group_new(program, buff,buffer,z,tg,bg);
    kh_insert(hmsp, sgs, sg->name, sg);
  }
  if (sg->buff == NULL) {
    sg->buff = vertex_buffer_new(vertex_buffer_format(buff));
  }
  sg->num++;
  return sg->buff;
}

//---------------------------------------------------------------------------------------------
static const vb_index_t  indices[6] = {0,1,2, 0,2,3};
void
sen_render_push_buffer(vertex_buffer_t* buff,
                       const texture_t* tex,
                       const font_t*    font,
                       const shader_t*  program,
                       blend_func blend)
{
  vertex_buffer_t* gb = get_group_buffer(buff, tex, font, program, blend);
  size_t ivsize = buff->vertices->item_size;
  //size_t iisize = buff->indices->item_size;
  size_t i;
  for (i = 0; i < buff->items->size; ++i) {
    ivec4* item = (ivec4*) vector_get(buff->items, i);
/*
    size_t j; size_t index = 0;
    for (j = 0; j < gb->items->size; ++j) {
      ivec4* gb_item = (ivec4*) vector_get(gb->items, j);
      // FIXME
      vec4* vz1 = ( vec4*) (buff->vertices->items + ivsize*item->x);
      vec4* vz2 = ( vec4*) (gb->vertices->items + ivsize*gb_item->x);
      if (vz1->z < vz2->z ) {
        //_logfi("");
        break;
      }
      index++;
    }
*/

//    vertex_buffer_push_back_vertices(gb, buff->vertices->items, buff->vertices->size );
  //  vertex_buffer_push_back_indicies(gb, buff->indicies->items, buff->indicies->size );

    vertex_buffer_push_back( gb,
                             (char*)(buff->vertices->items) + ivsize*item->x, item->y,
                             indices, 6);
    //vertex_buffer_insert( gb, 0,
      //                       buff->vertices->items + ivsize*item->x, item->y,
        //                     indices, 6);

/*
    vertex_buffer_insert( gb, index,
                             buff->vertices->items + ivsize*item->x, item->y,
                             buff->indices->items + iisize*item->z, item->w);
*/
  }
  //vertex_buffer_push_back_vertices(gb, buff->vertices->items, buff->vertices->size );
  //vertex_buffer_push_back_indices(gb, buff->indices->items, buff->indices->size );

}



void set_blending(blend_func f) {
  switch (f) {
    case bfDisable :
    {
      glDisable( GL_BLEND );
      return;
    }
    case bfPremultiAlpha :
    {
      glEnable( GL_BLEND );
      glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
      break;
    }
    case bfStraightAlpha:
    {
      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      break;
    }
    case bfAdditive:
    {
      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE);
      break;
    }
    case bfOneOne:
    {

      glEnable( GL_BLEND );
      glBlendFunc( GL_ONE, GL_ONE);
      break;
    }
    default :
    {
      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
  }
}

static vector_t* zsorter = NULL;
static int
zcmp (const void *pn1, const void *pn2)
{
  int z1 = (*(shader_group_t**)pn1)->z;
  int z2 = (*(shader_group_t**)pn2)->z;

  if (z1<z2) return -1;
  if (z1>z2) return 1;
  return 0;
}

size_t
sen_render_flush(int clear_buff)
{
//  gl_check_error();
  //_logfi("1");
  blend_group_t* bg;
  khint_t i,k,j;
  size_t total = 0;
  khash_t(hmsp)*  tgs;
  camera_t* cam = sen_camera();
  tex_group_t* tg;
  khash_t(hmsp)* sgs;
  shader_group_t* sg;

  vector_clear(zsorter);
  for (k = kh_begin(g_bgs); k != kh_end(g_bgs); ++k)
  {
    if (!kh_exist(g_bgs,k)) continue;
    bg = kh_val(g_bgs, k);

    if (bg->num == 0) {
      kh_del(hmip,g_bgs,k);
      continue;
    }
    tgs = bg->tgs;

   // set_blending( (blend_func) (kh_key(g_bgs, k))  );

    for (i = kh_begin(tgs); i != kh_end(tgs); ++i)
    {
      if (!kh_exist(tgs,i)) continue;
      tg = kh_val(tgs, i);
      if (tg->num == 0) {
        kh_del(hmsp,tgs,i);
        continue;
      }

      /*
      if (tg->tex)
        sen_texture_bind(tg->tex);
      else if (tg->font)
        sen_font_bind(tg->font);
        */

      sgs = tg->sgs;

      for (j = kh_begin(sgs); j != kh_end(sgs); ++j)
      {
        if (!kh_exist(sgs,j)) continue;
        sg = kh_val(sgs, j);
        if (sg->num == 0 || !sg->buff) {
          kh_del(hmsp,sgs,j);
          continue;
        }
        if (sg->buff) {
          /*
          sen_shader_use(sg->program);
          {
            if (tg->tex || tg->font)
              sen_uniform_1iN(sg->program, "u_tex0", 0);
            sen_uniform_m4fN(sg->program, "u_mvp",  cam->view_proj.data);
            vertex_buffer_render( sg->buff, GL_TRIANGLES);
            total+=vertex_buffer_size(sg->buff);
            if (clear_buff)
              vertex_buffer_clear( sg->buff );
            //sen_shader_use(NULL);
          }*/
          vector_push_back( zsorter, &sg );
        }
        sg->num = 0;
      }
      tg->num = 0;
    }
    bg->num = 0;
  }
  if (zsorter->size > 0)
    vector_sort(zsorter, zcmp);

  for (j = 0; j < zsorter->size; j++) {
    shader_group_t* sg = *(shader_group_t**)vector_get(zsorter, j);
   // _logfi("%s %d",sg->name, sg->z);


    set_blending( (blend_func) (sg->bg->key)  );


    if (sg->tg->tex)
      sen_texture_bind(sg->tg->tex);
    else if (sg->tg->font)
      sen_font_bind(sg->tg->font);

    sen_shader_use(sg->program);
    {

      if (sg->tg->tex || sg->tg->font)
        sen_uniform_1iN(sg->program, "u_tex0", 0);
      sen_uniform_m4fN(sg->program, "u_mvp", sg->z > 9500 ? cam->proj.data  : cam->view_proj.data);
      vertex_buffer_render( sg->buff, GL_TRIANGLES);
      total+=vertex_buffer_size(sg->buff);
      if (clear_buff)
        vertex_buffer_clear( sg->buff );

      //sen_shader_use(NULL);
    }
  }

 // _logfi("-------------------------------------------------");
  return total;
}

void
sen_render_reload()
{
  blend_group_t* bg;
  khash_t(hmsp)*  tgs;
  khint_t i,k,j;
  tex_group_t* tg;
  khash_t(hmsp)*  sgs;
  shader_group_t* sg;

  for (k = kh_begin(g_bgs); k != kh_end(g_bgs); ++k)
  {
    if (!kh_exist(g_bgs,k)) continue;
    bg = kh_val(g_bgs, k);
    tgs = bg->tgs;

    for (i = kh_begin(tgs); i != kh_end(tgs); ++i)
    {
      if (!kh_exist(tgs,i)) continue;
      tg = kh_val(tgs, i);
      sgs = tg->sgs;
      for (j = kh_begin(sgs); j != kh_end(sgs); ++j)
      {
        if (!kh_exist(sgs,j)) continue;
        sg = kh_val(sgs, j);

        if (sg->buff) {
          vertex_buffer_delete( sg->buff );
          sg->buff = NULL;
        }
      }
    }
  }
}

void
sen_render_init()
{
//  g_tgs = kh_init(hmsp);

  g_bgs  = kh_init(hmip);
  zsorter = vector_new( sizeof (shader_group_t*) );
}

/*
static void tgs_delete(khash_t(hmsp)* tgs)
{
  tex_group_t* tg;
  kh_foreach_value(tgs, tg, tex_group_delete(tg));
  kh_destroy(hmsp, tgs);
}

*/

void
sen_render_destroy()
{
  //tex_group_t* tg;
  //kh_foreach_value(g_tgs, tg, tex_group_delete(tg));
  //kh_destroy(hmsp, g_tgs);

  blend_group_t* bg;
  kh_foreach_value(g_bgs, bg, blend_group_delete(bg));
  kh_destroy(hmip, g_bgs);
  vector_delete(zsorter);
}
