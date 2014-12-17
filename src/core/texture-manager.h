#ifndef __TEXTURE_manager_H__
#define __TEXTURE_manager_H__
#include "config.h"
#include "texture-atlas.h"
#include "texture.h"
#include "font.h"

void
sen_textures_load(const char* filename,
                  const char* name,
                  const char* atlas_name,
                  size_t      atlas_size);

void
sen_textures_load_fontW(const char*      filename,
                       const float      font_size,
                       const wchar_t*   alphabet,
                       const char*      name,
                       const char*      atlas_name,
                       size_t           atlas_size);
void
sen_textures_load_font(const char*      filename,
                       const float      font_size,
                       const char*      utf8_alphabet,
                       const char*      name,
                       const char*      atlas_name,
                       size_t           atlas_size);

const texture_t*
sen_textures_get(const char* name);

void
sen_textures_release(const texture_t* tex);

const font_t*
sen_textures_get_font(const char* name);

void
sen_textures_release_font(const font_t* font);


void
sen_textures_reload();

void
sen_textures_collect(const char* atlas_name);

//------------------------------------------------------------------------------
void
sen_texture_manager_upload(void* _node);
void
sen_texture_manager_init(size_t default_atlas_size);
void
sen_texture_manager_destroy();
const texture_atlas_t*
sen_texture_manager_current_atlas();

#endif
