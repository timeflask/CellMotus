#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include "opengl.h"
#include "texture-atlas.h"
#include "macro.h"
#include "logger.h"
#include "texture-manager.h"
#include "vector.h"
#include "utils.h"
#include "image.h"
#include "khash.h"
#include "asset.h"
#include "font.h"
#include "vector.h"



#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:TexManager"

//------------------------------------------------------- atlas node
typedef struct atlas_node_t {
  char*                     name;
  texture_atlas_t*          atlas;
  int                       state;
  int                       refcount;
  size_t                    size;
  unsigned int              id;
} atlas_node_t;

typedef struct texture_node_t {
  int            order;
  texture_t*     tex;
  font_t*        font;
  char*          name;
  char*          filename;
  atlas_node_t*  ref;
} texture_node_t;

int texture_node_cmp(const void * pt1, const void *pt2)
{
  texture_node_t* t1 = *(texture_node_t**)pt1;
  texture_node_t* t2 = *(texture_node_t**)pt2;
  if (t1->order < t2->order)
    return -1;
  if (t1->order > t2->order)
    return 1;
  return 0;
}

static atlas_node_t*
atlas_node_new (const char* name, size_t atlas_size)
{
  static uint32_t uniqCounter = 0;
  char buffer[32];
  struct_malloc(atlas_node_t,self);

  if (name)
    self->name = sen_strdup(name);
  else {
    sprintf(buffer, "auto_%zd_atlas_%u",atlas_size, ++uniqCounter);
    self->name = sen_strdup(buffer);
  }
  self->atlas = 0;
  self->size = atlas_size;
  self->refcount = 0;
  self->id = 0;

  _logfi(" + texture atlas node \"%s\" [%dx%dx4]", self->name, self->size, self->size);
  return self;
}

static void
atlas_node_delete(atlas_node_t* self)
{
  sen_assert(self);
  _logfi(" - clean texture atlas node \"%s\"", self->name);
  if( self->id )
  {
    _logfi(" - clean GPU texture atlas node \"%s\", id=%d", self->name, self->id);
    glDeleteTextures( 1, &self->id );
  }
  free(self->name);
  if (self->atlas)
    texture_atlas_delete(self->atlas);
  free(self);
}
//------------------------------------------------------- tex node

static texture_node_t*
texture_node_new(const char* filename,  const char* name,  atlas_node_t* ref)
{
  struct_malloc(texture_node_t,self);
  sen_assert(ref);
  self->name = sen_strdup(name);
  self->filename = sen_strdup(filename);
  self->ref = ref;
  self->order = 0;

  self->tex = NULL;
  self->font = NULL;
  _logfi(" + texture node \"%s\" => %s", self->name, ref->name);
  return self;
}

static void
texture_node_delete(texture_node_t* self)
{
  _logfi(" - clean texture node \"%s\" ", self->name);
  if (self->tex)
    free(self->tex);
  if (self->font)
    sen_font_delete(self->font);
  free(self->name);
  free(self->filename);
  free(self);
}
//------------------------------------------------------- Manager

typedef struct texture_manager_t {
  int                    default_size;
  atlas_node_t*          current;
  khash_t(hmsp)          *atlas_map;
  khash_t(hmsp)          *tex_map;
} texture_manager_t;

static texture_manager_t* g_self = 0;

//------------------------------------------------------- Manager new
static texture_manager_t*
texture_manager_new(size_t default_atlas_size) {
  struct_malloc(texture_manager_t, self);
  _logfi("Create new texture manager");

  self->default_size = default_atlas_size;
  self->current = 0;

  self->atlas_map = kh_init(hmsp);
  kh_resize_hmsp(self->atlas_map, 32);

  self->tex_map = kh_init(hmsp);
  kh_resize_hmsp(self->tex_map, 64);

  return self;
}

static void clear_nodes(texture_manager_t* self)
{
  texture_node_t* tex;
  atlas_node_t* atl;
  sen_assert(self);

  kh_foreach_value(self->tex_map, tex, texture_node_delete(tex) );
  kh_clear(hmsp, self->tex_map);

  kh_foreach_value(self->atlas_map, atl, atlas_node_delete( atl) );
  kh_clear(hmsp, self->atlas_map);
}

static void
texture_manager_delete(texture_manager_t* self) {
  _logfi("Delete texture manager");
  sen_assert(self);

  clear_nodes(self);

  kh_destroy(hmsp, self->tex_map);
  kh_destroy(hmsp, self->atlas_map);

  free(self);
}

void
sen_texture_manager_init(size_t default_atlas_size)
{
  if (g_self == 0) {
    _logfi("Init textures manager");
    g_self = texture_manager_new(default_atlas_size);
  }
}

void
sen_texture_manager_destroy()
{
  if (g_self) {
    _logfi("destroy textures manager");
    texture_manager_delete(g_self);
  }
}


//------------------------------------------------------- good atlas size
static int
is_good_atlas_size (size_t x)
{

  return (  (
   x == 64 || x == 128 || x == 256 || x == 512 || x == 1024 ||
   x == 2048 || x == 4096 || x == 8192 || x == 16384 ) );
}

//------------------------------------------------------- Manager find atlas node
static atlas_node_t*
atlas_find(const char* atlas_name, khiter_t * pos)
{
  *pos = kh_get(hmsp, g_self->atlas_map, atlas_name);
  return *pos != kh_end(g_self->atlas_map) ? kh_val(g_self->atlas_map, *pos) : 0;
}

static texture_node_t*
tex_find(const char* name, khiter_t * pos)
{
  *pos = kh_get(hmsp, g_self->tex_map, name);
  return *pos != kh_end(g_self->tex_map) ? kh_val(g_self->tex_map, *pos) : 0;
}

//------------------------------------------------------- atlas reload (if needed)
static void
atlas_reload( atlas_node_t* a_node )
{
  vector_t *textures;
  texture_node_t* t_node;
  size_t i;
  size_t width;
  size_t height;
  image_t* img;
  ivec4 reg;
  float sz;
  wchar_t* tmp;

  sen_assert(g_self);

  if (a_node->atlas)
    return;


  _logfi(" !!! Atlas to RAM - texture atlas \"%s\"", a_node->name);
  a_node->atlas = texture_atlas_new(a_node->size, a_node->size, 4);


  textures = vector_new( sizeof (texture_node_t*) );
  kh_foreach_value(g_self->tex_map, t_node,
      if ( t_node->ref == a_node ) {
        vector_push_back(textures, &t_node);
      }
  );
  if (textures->size > 1)
    vector_sort(textures, &texture_node_cmp);
  for (i=0;i<textures->size;++i) {
    t_node = *(texture_node_t**)vector_get(textures, i);
    if (t_node->tex != NULL) {
      img = image_new(t_node->filename);
      reg = texture_atlas_get_region(a_node->atlas, img->width + 1, img->height + 1);
      sen_assert( reg.x > 0 );
      texture_atlas_set_region(a_node->atlas, reg.x, reg.y, img->width, img->height, img->raw_data, img->stride);
      width  = a_node->atlas->width;
      height = a_node->atlas->height;
      t_node->tex->coords.x = reg.x/(float)width;
      t_node->tex->coords.y = reg.y/(float)height;
      t_node->tex->coords.z = (reg.x + img->width)/(float)width;
      t_node->tex->coords.w = (reg.y + img->height)/(float)height;
      image_delete(img);
    }
    else if ( t_node->font != NULL )
    {
      //font_t* font = _sen_font_new(t_node->filename, t_node->font->size, t_node->font->alphabet, a_node->atlas);
      sz = (t_node->font->size);
      tmp = sen_strdupW(t_node->font->alphabet);
      sen_font_clean(t_node->font);
      sen_font_init(t_node->font, t_node->filename, sz, tmp, a_node->atlas);
      free(tmp);
      t_node->font->name = t_node->name;
      t_node->font->id = 0;
      t_node->font->_node = t_node->ref;
      //_sen_font_delete(t_node->font);
      //t_node->font = font;
    }
  }


  vector_delete(textures);

}
//------------------------------------------------------- Manager LOAD
static void
sen_atlas_select(const char* atlas_name,
                 size_t      atlas_size)
{
  texture_manager_t* self = (texture_manager_t* )g_self;
  khiter_t i;
  atlas_node_t* found;

  sen_assert(g_self);

  if (atlas_name) {
    if (!self->current || strcmp(atlas_name, self->current->name) ) {
      found = atlas_find(atlas_name, &i);
      if (found) {
       _logfi (" current texture atlas => %s", found->name);
        self->current = found;
      }
      else {
        _logfi (" texture atlas %s not found, creating new...", atlas_name);
        if (atlas_size == 0)
          atlas_size = self->default_size;
        sen_assert( is_good_atlas_size(atlas_size) );
        if (is_good_atlas_size(atlas_size)) {
          atlas_node_t* new_node = atlas_node_new(atlas_name, atlas_size);
          kh_insert(hmsp, self->atlas_map, new_node->name, new_node);
          _logfi (" current texture atlas => %s", atlas_name);
          self->current = new_node;
        }
      }
    }
  }
  else {
    if (self->current)
      _logfi (" -using default atlas ...");
    else {
      _logfi (" -create default atlas ");
      if (atlas_size == 0)
        atlas_size = self->default_size;
      sen_assert( is_good_atlas_size(atlas_size) );
      if (is_good_atlas_size(atlas_size)) {
        atlas_node_t* new_node = atlas_node_new(0, atlas_size);
        kh_insert(hmsp, self->atlas_map, new_node->name, new_node);
        _logfi (" current texture atlas => %s", new_node->name);
        self->current = new_node;
      }
    }
  }
}

void
sen_textures_load(const char* filename,
                const char* name,
                const char* atlas_name,
                size_t atlas_size)
{
  texture_manager_t* self = (texture_manager_t* )g_self;
  const char* _name;
  khiter_t i;
  texture_node_t* found;
  image_t* img;
  ivec4 reg;
  atlas_node_t* new_node;
  texture_node_t* new_tex;
  size_t width, height;

  sen_assert(g_self);

  sen_atlas_select(atlas_name, atlas_size);

  if (filename == NULL)
    return;

  _name = name?name:filename;
  found = tex_find(_name, &i);

  if (found) {
   _logfi ("texture name %s already loaded to %s, refcount=%d", found->name, found->ref->name, found->ref->refcount);
   return;
  }

  img = image_new(filename);
  atlas_reload( self->current );
  reg = texture_atlas_get_region(self->current->atlas, img->width+1, img->height+1);
  //
  if (reg.x < 0 || reg.y < 0) {
    _logfw (" current atlas %s is full, creating new ...", self->current->name);
    if (atlas_size == 0)
      atlas_size = self->current->atlas->width;
    new_node = atlas_node_new(atlas_name, max(atlas_size, max(img->width, img->height)) );
    kh_insert(hmsp, self->atlas_map, new_node->name, new_node);
    _logfi (" current texture atlas => %s", new_node->name);
    self->current = new_node;
    atlas_reload( self->current );
    reg = texture_atlas_get_region(self->current->atlas, img->width+1, img->height+1);
  }


  texture_atlas_set_region(self->current->atlas, reg.x, reg.y, img->width, img->height, img->raw_data, img->stride);
  new_tex = texture_node_new(filename,_name, self->current);
  width  = self->current->atlas->width;
  height = self->current->atlas->height;
  if (new_tex->tex == NULL) {
    new_tex->tex = (texture_t*)malloc(sizeof(texture_t));
    new_tex->tex->name = new_tex->name;
    new_tex->tex->id = 0;
    new_tex->tex->_node = new_tex->ref;
  }
  new_tex->tex->coords.x = reg.x/(float)width;
  new_tex->tex->coords.y = reg.y/(float)height;
  new_tex->tex->coords.z = (reg.x + img->width)/(float)width;
  new_tex->tex->coords.w = (reg.y + img->height)/(float)height;

  new_tex->tex->img_alpha = img->alpha;
  new_tex->tex->img_width = img->width;
  new_tex->tex->img_height = img->height;

  new_tex->order=kh_size(self->tex_map);

  kh_insert(hmsp, self->tex_map, new_tex->name, new_tex);
  image_delete(img);
}


void
sen_textures_load_fontW(const char*      filename,
                       const float      font_size,
                       const wchar_t*   alphabet,
                       const char*      name,
                       const char*      atlas_name,
                       size_t           atlas_size)
{
  texture_manager_t* self = (texture_manager_t* )g_self;
  const char* _name;
  khiter_t i;
  texture_node_t* found;
  font_t* font;
  texture_node_t* new_tex;

  sen_assert(g_self);

  sen_atlas_select(atlas_name, atlas_size);

  if (filename == NULL)
    return;

  _name = name?name:filename;
  found = tex_find(_name, &i);
  if (found) {
   _logfi ("texture name %s already loaded to %s, refcount=%d", found->name, found->ref->name, found->ref->refcount);
   return;
  }

  atlas_reload( self->current );

  font = sen_font_new(filename, font_size, alphabet, self->current->atlas);

  new_tex = texture_node_new(filename,_name, self->current);
  font->name = new_tex->name;
  font->id = 0;
  font->_node = new_tex->ref;
  new_tex->font = font;
  new_tex->order=kh_size(self->tex_map);
  kh_insert(hmsp, self->tex_map, new_tex->name, new_tex);

}

void
sen_textures_load_font(const char*      filename,
                       const float      font_size,
                       const char*      utf8_alphabet,
                       const char*      name,
                       const char*      atlas_name,
                       size_t           atlas_size)
{
  wchar_t* ab =  utf8_alphabet ?  sen_strdupToW(utf8_alphabet) : NULL;
  sen_textures_load_fontW(filename, font_size, ab, name, atlas_name, atlas_size);
  if(ab)
    free(ab);
}

static void
atlas_unload( atlas_node_t* found, khiter_t pos)
{
  texture_node_t* node;  int flag;
  atlas_node_t* anode;
  do {
    flag = 0;
    kh_foreach_value(g_self->tex_map, node,
        if ( node->ref == found ) {
          texture_node_delete(node);
          kh_del(hmsp, g_self->tex_map, __i);
          flag = 1;
          break;
        }
    );
  } while(flag);

  if (found == g_self->current) {
     g_self->current = 0;
     kh_foreach_value(g_self->atlas_map, anode,
       if (strcmp(found->name, anode->name)) {
         g_self->current = anode;
       }
     );
     _logfi (" current texture atlas => %s", g_self->current->name);
  }

  atlas_node_delete(found);
  kh_del(hmsp, g_self->atlas_map, pos);
}

void
sen_textures_collect(const char* atlas_name)
{
  atlas_node_t* found;
  khiter_t i;
  atlas_node_t* node;

  sen_assert(g_self);
  _logfi("*** COLLECT TEXTURES ***");

  if (atlas_name) {
    do {
      _logfi (" -unloading %s ", atlas_name);
      found = atlas_find(atlas_name, &i);
      if (found && found->refcount == 0)
        atlas_unload(found, i);
      else {
        if (found && found->refcount != 0)
         _logfi ("[%s] name refcount=%d, reject garbage collect request", found->name, found->refcount);
        else {
          _logfi ("  *[%s] name purged", atlas_name);
        }
        break;
      }
    } while (found);
  }
  else {
    _logfi ("Textures clearing...");
    do {
      found = 0; i=0;
      kh_foreach_value(g_self->atlas_map, node,
        if (node->refcount == 0) {
          found = node;
          i = __i;
          break;
        }
      );

      if (found)
        atlas_unload(found, i);
      else
        break;
    } while (1);
    _logfi ("Textures clearing done!");
#ifdef SEN_DEBUG
    kh_foreach_value(g_self->atlas_map, node,
        _logfi ("  [%s] refs:%d", node->name, node->refcount);
    );
#endif
  }
  _logfi("*** END COLLECT TEXTURES ***");
}

const texture_t*
sen_textures_get(const char* name)
{
  khiter_t i;
  texture_node_t* found;

  sen_assert(g_self);
  sen_assert(name);

  found = tex_find(name, &i);
  if (found && found->tex!=NULL) {
    found->ref->refcount++;
    return found->tex;
  }
  return 0;
}

void
sen_textures_release(const texture_t* tex)
{
  atlas_node_t* node = (atlas_node_t*)(tex->_node);
  sen_assert(tex);
  if (node->refcount)
    node->refcount--;
}

const font_t*
sen_textures_get_font(const char* name)
{
  khiter_t i;
  texture_node_t* found;

  sen_assert(g_self);
  sen_assert(name);

  found = tex_find(name, &i);
  if (found && found->font!=NULL) {
    found->ref->refcount++;
    return found->font;
  }
  return 0;

}

void
sen_textures_release_font(const font_t* font)
{
  atlas_node_t* node;
  sen_assert(font);
  node = (atlas_node_t*)(font->_node);
  if (node->refcount)
    node->refcount--;
}

void
sen_texture_manager_upload_atlas(atlas_node_t* node)
{
  texture_node_t* t;

  sen_assert(g_self);
  sen_assert(node);
  sen_assert(node->atlas);


  node->id = texture_atlas_upload(node->atlas);
  _logfi("Uploading/Binding texture atlas \"%s\", id=%d", node->name, node->id);

  kh_foreach_value(g_self->tex_map, t,
      if ( t->ref == node ) {
        if (t->tex != NULL)
          t->tex->id = node->id;
        else if (t->font != NULL)
          t->font->id = node->id;
      }
    );

  _logfi(" !!! RAM Clearing - texture atlas \"%s\", id=%d", node->name, node->id);
  texture_atlas_delete(node->atlas); node->atlas = 0;

}

void
sen_texture_manager_upload(void* _node)
{
  sen_assert(_node);
  sen_texture_manager_upload_atlas((atlas_node_t*)(_node));
}

const char*
sen_texture_atlas(const texture_t* tex)
{
  sen_assert(tex);
  return ((atlas_node_t*)(tex->_node))->name;
}

const char*
sen_font_atlas(const font_t* font)
{
  sen_assert(font);
  return ((atlas_node_t*)(font->_node))->name;
}


void
sen_textures_reload()
{
  texture_node_t* t;
  atlas_node_t* a;

  sen_assert(g_self);
  _logfi("*** RELOAD TEXTURES ***");

  kh_foreach_value(g_self->tex_map, t,
      if (t->tex != NULL)
        t->tex->id = 0;
      else if (t->font != NULL)
        t->font->id = 0;
  );
  kh_foreach_value(g_self->atlas_map, a, atlas_reload( a ) );

  sen_font_bind(NULL);
  sen_texture_bind(NULL);

  _logfi("*** END RELOAD TEXTURES ***");

}

const texture_atlas_t*
sen_texture_manager_current_atlas()
{
  sen_assert(g_self);
  return g_self->current ? g_self->current->atlas : 0;
}
