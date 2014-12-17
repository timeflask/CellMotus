#ifndef __font_H__
#define __font_H__

#include "config.h"
#include "vector.h"
#include "texture-atlas.h"

typedef struct
{
  wchar_t charcode;
  float   kerning;
} kerning_t;

typedef struct
{
    wchar_t charcode;

    size_t width;
    size_t height;

    int offset_x;
    int offset_y;

    float advance_x;
    float advance_y;

    float s0;
    float t0;
    float s1;
    float t1;

    vector_t * kerning;
} glyph_t;

typedef struct font_t {
  const char*         name;
  unsigned int        id;

  wchar_t*            alphabet;

  float       size;
  int         hinting;
  int         filtering;
  int         kerning;
  float       height;
  float       linegap;
  float       ascender;
  float       descender;

  const void*  _node;
  void*        glyphs;
} font_t;


void
sen_font_bind(const font_t* tex);

const glyph_t *
sen_font_get_glyph(const font_t * font, wchar_t charcode );

float
sen_glyph_kerning(const glyph_t * glyph, wchar_t charcode );
//--------------------------------------------------------------------------

font_t*
sen_font_new( const char* filename,
                  float size,
                  const wchar_t* alphabet,
                  texture_atlas_t* atlas);
void
sen_font_delete(font_t* self);

void
sen_font_init(font_t* self,
                  const char* filename,
                  float size,
                  const wchar_t* alphabet,
                  texture_atlas_t* atlas);
void
sen_font_clean(font_t* self);

const char*
sen_font_atlas(const font_t* font);

#endif
