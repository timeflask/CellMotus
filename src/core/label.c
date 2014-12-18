#include <wchar.h>
#include <string.h>
#include "label.h"
#include "macro.h"
#include "texture-manager.h"
#include "shader.h"
#include "macro.h"
#include "logger.h"
#include "camera.h"
#include "utils.h"
#include "font.h"
#include "view.h"
#include "render.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Label"

static const GLushort indices[6] = {0,1,2, 0,2,3};

void update_text( label_t* self )
{
    size_t i;

    node_t* node = (node_t*)self;

    vec4 * color = & (node->color);
    float r = color->r,
          g = color->g,
          b = color->b,
          a = color->a;

    vec4* bbox = & ( node->bbox );

    vertex_buffer_t* buffer  = self->buff;
    const font_t *font       = self->font;
    const wchar_t *text      = self->text;
    size_t len = wcslen(text);
    vector_t* verts = self->verts;
    vec2 pen = {{0,0}};

    vertex_buffer_clear(buffer);
    vector_clear(verts);

    vector_reserve( verts, len*4 );
    for( i=0; i<len; ++i )
    {
        const glyph_t *glyph = sen_font_get_glyph( font, text[i] );
        if( glyph != NULL )
        {
            int kerning = 0;
            int x0,y0,x1,y1;
            float s0,t0,s1,t1;
            V4F_T2F_C4F vertices[4];

            if( i > 0)
            {
                kerning = (int)sen_glyph_kerning( glyph, text[i-1] );
            }
            pen.x += kerning;

            x0  = (int)( pen.x + glyph->offset_x );
            y0  = (int)( pen.y + glyph->offset_y );
            x1  = (int)( x0 + glyph->width );
            y1  = (int)( y0 - glyph->height );

            s0 = glyph->s0;
            t0 = glyph->t0;
            s1 = glyph->s1;
            t1 = glyph->t1;

            /*
            V4F_T2F_C4F vertices[4] = { { x0,y0,0,1,  s0,t0,  r,g,b,a },
                                        { x0,y1,0,1,  s0,t1,  r,g,b,a },
                                        { x1,y1,0,1,  s1,t1,  r,g,b,a },
                                        { x1,y0,0,1,  s1,t0,  r,g,b,a } };
                                        */
            vertices[0].x = (float)x0;
            vertices[0].y = (float)y0;
            vertices[0].z = 0.0f;
            vertices[0].w = 1.0f;
            vertices[0].s = s0;
            vertices[0].t = t0;
            vertices[0].r = r;
            vertices[0].g = g;
            vertices[0].b = b;
            vertices[0].a = a;


            vertices[1].x = (float)x0;
            vertices[1].y = (float)y1;
            vertices[1].z = 0.0f;
            vertices[1].w = 1.0f;
            vertices[1].s = s0;
            vertices[1].t = t1;
            vertices[1].r = r;
            vertices[1].g = g;
            vertices[1].b = b;
            vertices[1].a = a;

            vertices[2].x = (float)x1;
            vertices[2].y = (float)y1;
            vertices[2].z = 0.0f;
            vertices[2].w = 1.0f;
            vertices[2].s = s1;
            vertices[2].t = t1;
            vertices[2].r = r;
            vertices[2].g = g;
            vertices[2].b = b;
            vertices[2].a = a;

            vertices[3].x = (float)x1;
            vertices[3].y = (float)y0;
            vertices[3].z = 0.0f;
            vertices[3].w = 1.0f;
            vertices[3].s = s1;
            vertices[3].t = t0;
            vertices[3].r = r;
            vertices[3].g = g;
            vertices[3].b = b;
            vertices[3].a = a;

            vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );
            vector_push_back_data(verts, vertices, 4);
            pen.x += glyph->advance_x;

            if (i==0) {
              bbox->x = (float) x0;
              bbox->y = (float) y1;
              bbox->width  = x1-bbox->x;
              bbox->height = y0-bbox->y;
            }
            else {
              if  (x0 < bbox->x)                 bbox->x = (float) x0;
              if  (y1 < bbox->y)                 bbox->y = (float) y1;
              if ((x1 - bbox->x) > bbox->width)  bbox->width  = x1-bbox->x;
              if ((y0 - bbox->y) > bbox->height) bbox->height = y0-bbox->y;
            }
        }
    }
    ((node_t*)self)->updated &= ~ (SEN_NODE_UPDATE_BBOX | SEN_NODE_UPDATE_COLOR );
    ((node_t*)self)->updated |= SEN_NODE_UPDATE_MODEL ;
}

static void
update_buffer(label_t* self)
{
  int status  = ((node_t*)self)->updated;
  const vec4* col = &(((node_t*)self)->color);
  vec4* bbox = & ( ((node_t*) self)->bbox );
  int update_model;

  if (status & SEN_NODE_UPDATE_INVALIDATE_BUFFER) {
    vertex_buffer_invalidate(self->buff);
    status &= ~SEN_NODE_UPDATE_INVALIDATE_BUFFER;
  }

  //vec2* size = & ( ((node_t*) self)->size );

  if (status & SEN_NODE_UPDATE_BBOX) {
    update_text(self);
    status |= SEN_NODE_UPDATE_MODEL;
  }

  update_model = status & SEN_NODE_UPDATE_MODEL;

  if ( status & (SEN_NODE_UPDATE_MODEL | SEN_NODE_UPDATE_COLOR)) {

    const mat4* model = sen_node_model(self);

    size_t i, sz = self->verts->size;
    for (i=0; i<sz; ++i) {
      if (update_model) {
        vec4* vp = (vec4*) vector_get( self->verts, i );
        vec4* v =  (vec4*)( (char*)(self->buff->vertices->items) + i*sizeof(V4F_T2F_C4F));

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
        vec4* c =(vec4*)( (char*)(self->buff->vertices->items) +
                          i*sizeof(V4F_T2F_C4F ) +
                          sizeof(float)*6);

        memcpy(c,col,sizeof(vec4));
      }
    }
    status &= ~(SEN_NODE_UPDATE_COLOR|SEN_NODE_UPDATE_MODEL|SEN_NODE_UPDATE_PARENT_MODEL);
  }

  ((node_t*)self)->updated = status;
}


static node_interface_t label_interface = {
    &sen_label_delete, &sen_label_render
};

label_t*
sen_label_newW(const char* name,
              const char* font_name,
              const wchar_t* text)
{
  //_logfi("+label %s, font=%s", name, font_name);
  struct_malloc(label_t, self);
  sen_node_init(self, name, 0);
  self->font = sen_textures_get_font(font_name);
  sen_assert(self->font);
  self->program = sen_shaders_get("label");
  sen_assert(self->program);
  self->buff = vertex_buffer_new("a_pos:4f,a_tex_coords:2f,a_color:4f");

  self->verts = vector_new ( sizeof (V4F_T2F_C4F) );

  self->text        = NULL;
  self->text_length = 0;

  sen_label_set_textW(self, text);
  update_buffer(self);


  ((node_t*)self)->vt = &label_interface;
  return self;
}

label_t*
sen_label_new(const char* name,
              const char* font_name,
              const char* text)
{
  wchar_t* wt = sen_strdupToW(text);
  label_t* self = sen_label_newW(name, font_name, wt);
  free(wt);
  return self;
}

void
sen_label_delete(void *_self)
{
  label_t *self = (label_t *)_self;
  int status;

  sen_assert(_self);
 // _logfi("-label %s", ((object_t*)self)->name);
  free(self->text);
  sen_textures_release_font(self->font);
  sen_shaders_release(self->program);
  status = ((node_t*)self)->updated;
  if (status & SEN_NODE_UPDATE_INVALIDATE_BUFFER) {
    vertex_buffer_invalidate(self->buff);
  }
  vertex_buffer_delete(self->buff);
  vector_delete(self->verts);
  sen_node_clean(self);
  free(self);
}

void
sen_label_render(void* _self)
{
  label_t *self = (label_t *)_self;
  node_t* node = (node_t*) _self;
  vec4* bbox;
  const vec4* vp;
  node_t* cam_node = (node_t* ) sen_camera();
  const mat4* mv = &(cam_node->model);
  vec4* v;
  int use_cam;

  sen_assert(_self);

  if ( node->color.a < F_EPSILON || self->buff->vertices->size == 0 ) return;
  update_buffer(self);


  bbox = & ( ((node_t*) self)->bbox );
  vp = sen_view_get_viewport();

  v = (vec4*) (self->buff->vertices->items);
  use_cam = v->z < 0.9 && v->z > -0.9 ;
  if (use_cam) {
    float sx =   mv->data[12] ;
    float sy =   mv->data[13] ;

    float vw = (vp->z - vp->x) / 2.0f ;
    float vh = (vp->w - vp->y) / 2.0f ;

    if (bbox->z+ sx< -vw  ||
        bbox->x+ sx>  vw  ||
        bbox->w+ sy< -vh  ||
        bbox->y+ sy>  vh  )
    {
     // _logfi("NO");
      return;
    }
  }


/*
  int status = ((node_t*)self)->updated;

  if (status & SEN_NODE_UPDATE_INVALIDATE_BUFFER) {
    vertex_buffer_invalidate(self->buff);
    ((node_t*)self)->updated &= ~SEN_NODE_UPDATE_INVALIDATE_BUFFER;

  }

  if (status & (SEN_NODE_UPDATE_BBOX | SEN_NODE_UPDATE_COLOR) ) {
    update_text(self);
  }


  vec4* bbox = & ( ((node_t*) self)->bbox );
  const vec4* vp = sen_view_get_viewport();

  node_t* cam_node = (node_t* ) sen_camera();
  const mat4* mv = &(cam_node->model);

  float sx =  mv->data[12];
  float sy =  mv->data[13];

  float vw = (vp->z - vp->x) / 2.0 ;
  float vh = (vp->w - vp->y) / 2.0 ;

  if (bbox->z+ sx< -vw  ||
      bbox->x+ sx>  vw  ||
      bbox->w+ sy< -vh  ||
      bbox->y+ sy>  vh  )
  {
    //_logfi("NO");
    return;
  }
*/

  sen_render_push_buffer(self->buff, NULL,self->font, self->program, node->blend);

  //sen_font_bind(self->font);

  /*
  mat4 mvp;
  camera_t* cam = sen_camera();

  //if ( ((node_t*)self)->posZ < 0.9 )
//    mat4_multiply2(sen_node_model(self), & cam->view_proj, &mvp);
  //else
    //mat4_multiply2(sen_node_model(self), & cam->proj, &mvp);

  sen_shader_use(self->program);
  {
    sen_uniform_1iN(self->program, "u_tex0", 0);
    sen_uniform_m4fN(self->program, "u_mvp",  & cam->view_proj);
    vertex_buffer_render( self->buff, GL_TRIANGLES);
  }
  gl_check_error();
  */
}

void
sen_label_set_textW(void* _self, const wchar_t* text)
{
  label_t *self = (label_t *)_self;
  size_t len;

  sen_assert(_self);
  sen_assert(text);

  if (self->text && wcscmp(self->text, text)==0) return;


  len = wcslen(text);
  if (self->text) {
    if (len <= self->text_length)
      memcpy(self->text, text, sizeof(wchar_t)*(len+1) );
    else {
      free(self->text);
      self->text = sen_strdupW(text);
      self->text_length = len;
    }
  }
  else {
    self->text = sen_strdupW(text);
    self->text_length = len;
  }

  ((node_t*)self)->updated |= SEN_NODE_UPDATE_BBOX;
  update_text(_self);
}

void
sen_label_set_text(void* _self, const char* text)
{
  wchar_t* wt = sen_strdupToW(text);
  sen_label_set_textW(_self, wt);
  free(wt);
}
