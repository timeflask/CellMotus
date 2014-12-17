#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_LCD_FILTER_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <wchar.h>
#include <string.h>
#include <stdio.h>

#include "font.h"
#include "texture-manager.h"
#include "opengl.h"
#include "macro.h"
#include "logger.h"
#include "asset.h"
#include "utils.h"
#include "khash.h"
#include "edtaa3func.h"
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Font"
extern unsigned int g_current_tex;

void
sen_font_bind(const font_t* font)
{
  if (font) {
    if (!font->id) {
      _logfi("Need to upload font [%s]", font->name);
      sen_texture_manager_upload((void*)font->_node);
    }
    if (g_current_tex != font->id) {
      glBindTexture( GL_TEXTURE_2D, font->id );
      g_current_tex = font->id;
    }
  }
  else {
    glBindTexture( GL_TEXTURE_2D, 0 );
    g_current_tex = 0;
  }

}


const glyph_t *
sen_font_get_glyph(const font_t * font,
                   wchar_t        charcode )
{
  sen_assert( font );
  khash_t(hmip) *glyphs = (khash_t(hmip)*) font->glyphs;
  khiter_t k = kh_get(hmip, glyphs, (int)charcode );
  return k == kh_end(glyphs) ? NULL : (const glyph_t *) kh_val( glyphs, k );;
}

float
sen_glyph_kerning(const glyph_t * glyph,
                  wchar_t         charcode)
{
  size_t i;
  sen_assert( glyph );
  for( i=0; i<vector_size(glyph->kerning); ++i )
  {
    kerning_t * kerning = (kerning_t *) vector_get( glyph->kerning, i );
    if( kerning->charcode == charcode )
    {
        return kerning->kerning;
    }
  }
  return 0;
}


static glyph_t*
glyph_new()
{
  struct_malloc(glyph_t, self);
//  self->id        = 0;
  self->width     = 0;
  self->height    = 0;
  //self->outline_type = 0;
  //self->outline_thickness = 0.0;
  self->offset_x  = 0;
  self->offset_y  = 0;
  self->advance_x = 0.0;
  self->advance_y = 0.0;
  self->s0        = 0.0;
  self->t0        = 0.0;
  self->s1        = 0.0;
  self->t1        = 0.0;
  self->kerning   = vector_new( sizeof(kerning_t) );
  return self;
}

static void
glyph_delete(glyph_t* self)
{
  sen_assert( self );
  vector_delete( self->kerning );
  free( self );
}

void
font_face_close( FT_Library  *library,
                 FT_Face     *face )
{
  _logfi("-close font face");
  FT_Done_Face( *face );
  FT_Done_FreeType( *library );
}

static const int HIRESFACTOR = 1;

void
font_face_load( FT_Library  *library,
                asset_t     *font_file,
                const float  size,
                FT_Face     *face )
{
  sen_assert(font_file);
  _logfi("... loading font face [%s], size=%.1f", font_file->path, size);
  size_t hres = 64;
  FT_Error error;
  FT_Matrix matrix = { (int)((1.0/hres) * 0x10000L),
                       (int)((0.0)      * 0x10000L),
                       (int)((0.0)      * 0x10000L),
                       (int)((1.0)      * 0x10000L) };

  sen_assert( library );
  sen_assert( font_file );

  error = FT_Init_FreeType( library );
  if( error )
  {
      _logfe("FT_Error (0x%02x) : %s\n",
              FT_Errors[error].code, FT_Errors[error].message);
      exit(EXIT_FAILURE);
  }

  error =  FT_New_Memory_Face (*library, font_file->data, font_file->size, 0, face);
  if( error )
  {
      _logfe( "FT_Error (line %d, code 0x%02x) : %s\n",
               __LINE__, FT_Errors[error].code, FT_Errors[error].message);
      FT_Done_FreeType( *library );
      exit(EXIT_FAILURE);
  }

  error = FT_Select_Charmap( *face, FT_ENCODING_UNICODE );
  if( error )
  {
      _logfe( "FT_Error (line %d, code 0x%02x) : %s\n",
               __LINE__, FT_Errors[error].code, FT_Errors[error].message );
      FT_Done_Face( *face );
      FT_Done_FreeType( *library );
      exit(EXIT_FAILURE);
  }

  error = FT_Set_Char_Size( *face, (int)(size*64*HIRESFACTOR), 0, 72*hres, 72 );
  if( error )
  {
      _logfe("FT_Error (line %d, code 0x%02x) : %s\n",
               __LINE__, FT_Errors[error].code, FT_Errors[error].message );
      FT_Done_Face( *face );
      FT_Done_FreeType( *library );
      exit(EXIT_FAILURE);
  }
  FT_Set_Transform( *face, &matrix, NULL );
  _logfi("... loading done");
}


static const wchar_t * FONT_CACHE =
    L" !\"#$%&'()*+,-./0123456789:;<=>?"
    L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    L"`abcdefghijklmnopqrstuvwxyz{|}~";

void
sen_font_init(font_t* self,
               const char* filename,
               float size,
               const wchar_t* alphabet,
               texture_atlas_t* atlas)
{
  sen_assert( size > 4 );
  sen_assert( atlas );
  asset_t* font_file = asset_new( filename );

  if (alphabet)
    self->alphabet = sen_strdupW(alphabet);
  else
    self->alphabet = sen_strdupW(FONT_CACHE);

  self->height = 0;
  self->ascender = 0;
  self->descender = 0;
  self->size = size;
  self->hinting = 1;
  self->kerning = 1;
  self->filtering = 1;

  FT_Library      library;
  FT_Face         face;
  FT_Size_Metrics metrics;
  font_face_load(&library, font_file, self->size, &face);


  metrics = face->size->metrics;
  self->ascender = (metrics.ascender >> 6) / 100.0;
  self->descender = (metrics.descender >> 6) / 100.0;
  self->height = (metrics.height >> 6) / 100.0;
  self->linegap = self->height - self->ascender + self->descender;


  self->glyphs = kh_init(hmip);

  size_t width  = atlas->width;
  size_t height = atlas->height;

  ivec4 region = texture_atlas_get_region( atlas, 5, 5 );
  if ( region.x < 0 )
  {
      _logfe("Cannot load glyph [%d] from [%s], atlas is full", -1, filename);
      goto end;
  }

  glyph_t * glyph = glyph_new( );
  static unsigned char data[4*4*4] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  texture_atlas_set_region( atlas, region.x, region.y, 4, 4, data, 0 );
  glyph->charcode = (wchar_t)(-1);
  glyph->s0 = (region.x+2)/(float)width;
  glyph->t0 = (region.y+2)/(float)height;
  glyph->s1 = (region.x+3)/(float)width;
  glyph->t1 = (region.y+3)/(float)height;
  kh_insert(hmip, ((khash_t(hmip)*)self->glyphs), (int)glyph->charcode , glyph);

  FT_UInt       glyph_index;
  FT_Error      error;
  FT_GlyphSlot  slot;

  FT_Bitmap     ft_bitmap;
  size_t        i, x, y, w, h;
  for( i=0; i<wcslen(self->alphabet); ++i )
  {

    if (kh_get (hmip, ((khash_t(hmip)*)self->glyphs), (int)(self->alphabet[i])  )
        != kh_end(((khash_t(hmip)*)self->glyphs))) continue;



      FT_Int32 flags = 0;
      int ft_bitmap_width = 0;
      int ft_bitmap_rows = 0;
      int ft_glyph_top = 0;
      int ft_glyph_left = 0;
      glyph_index = FT_Get_Char_Index( face, self->alphabet[i] );

      flags |= FT_LOAD_RENDER;

      if( !self->hinting )
          flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
      else
          flags |= FT_LOAD_FORCE_AUTOHINT;

      error = FT_Load_Glyph( face, glyph_index, flags );
      if( error )
      {
          _logfe( "FT_Error (line %d, code 0x%02x) : %s\n",
                   __LINE__, FT_Errors[error].code, FT_Errors[error].message );
          goto end;
      }

      slot            = face->glyph;
      ft_bitmap       = slot->bitmap;
      ft_bitmap_width = slot->bitmap.width;
      ft_bitmap_rows  = slot->bitmap.rows;
   //   ft_bitmap_pitch = slot->bitmap.pitch;
      ft_glyph_top    = slot->bitmap_top;
      ft_glyph_left   = slot->bitmap_left;

    // Allocate high resolution buffer
/*
      double padding = 0.1;
      double highres_size = self->size*HIRESFACTOR;
      double lowres_size = self->size;

      size_t highres_width  = ft_bitmap.width + 2*padding*highres_size;
      size_t highres_height = ft_bitmap.rows + 2*padding*highres_size;
      double * highres_data = (double *) malloc( highres_width*highres_height*sizeof(double) );
      memset( highres_data, 0, highres_width*highres_height*sizeof(double) );

      // Copy high resolution bitmap with padding and normalize values
      int m,n;
      for( m=0; m < ft_bitmap.rows; ++m )
      {
          for( n=0; n < ft_bitmap.width; ++n )
          {
              int x = n + padding;
              int y = m + padding;
              highres_data[y*highres_width+x] = ft_bitmap.buffer[m*ft_bitmap.width+n]/255.0;
          }
      }

      distance_map2( highres_data, highres_width, highres_height );
      // Allocate low resolution buffer
      size_t lowres_width  = round(highres_width * lowres_size/highres_size);
      size_t lowres_height = round(highres_height * lowres_width/(float) highres_width);
      double * lowres_data = (double *) malloc( lowres_width*lowres_height*sizeof(double) );
      memset( lowres_data, 0, lowres_width*lowres_height*sizeof(double) );

      // Scale down highres buffer into lowres buffer
      resize( highres_data, highres_width, highres_height,
              lowres_data,  lowres_width,  lowres_height );

      unsigned char * data =
          (unsigned char *) malloc( lowres_width*lowres_height*sizeof(unsigned char) );
      for( m=0; m < lowres_height; ++m )
      {
          for( n=0; n < lowres_width; ++n )
          {
              double v = lowres_data[m*lowres_width+n];
              data[m*lowres_width+n] = (int) (255*(1-v));
          }
      }

      float ratio = lowres_size / highres_size;
      size_t pitch  = lowres_width * sizeof( unsigned char );
*/



      w = ft_bitmap_width + 1;
      h = ft_bitmap_rows + 1;
      region = texture_atlas_get_region( atlas, w, h );
      if ( region.x < 0 )
      {
          _logfe( "Texture atlas is full");
          continue;
      }
      w = w - 1;
      h = h - 1;
      x = region.x;
      y = region.y;

      size_t sz = ft_bitmap_width*ft_bitmap_rows*4; size_t j;
      unsigned char* buff = calloc(sz,1);
      //unsigned char* map = make_distance_map(ft_bitmap.buffer, ft_bitmap_width, ft_bitmap_rows);
      for (j=0; j<(sz/4); ++j)
        buff[j*4+3] = ft_bitmap.buffer[j];
/*
      texture_atlas_set_region( atlas, x, y, w, h,
                                ft_bitmap.buffer, ft_bitmap.pitch );
*/

      texture_atlas_set_region( atlas, x, y, w, h,
                                buff, ft_bitmap.pitch *4 );

      free(buff);
      //free(map);

      glyph = glyph_new( );
      glyph->charcode = self->alphabet[i];
  //    glyph->width    = lowres_width;
//      glyph->height   = lowres_height;
     // _logfe("%d %d", glyph->width, glyph->height);
      glyph->width    = w;
      glyph->height   = h;
      //glyph->outline_type = self->outline_type;
      //glyph->outline_thickness = self->outline_thickness;
  //    glyph->offset_x = (slot->bitmap_left + padding*highres_width) * ratio;
    //  glyph->offset_y = (slot->bitmap_top + padding*highres_height) * ratio;
      glyph->offset_x = ft_glyph_left;
      glyph->offset_y = ft_glyph_top;

      glyph->s0       = x/(float)width;
      glyph->t0       = y/(float)height;
      glyph->s1       = (x + glyph->width)/(float)width;
      glyph->t1       = (y + glyph->height)/(float)height;


/*
      region = texture_atlas_get_region( atlas, glyph->width, glyph->height );
      size_t sz = glyph->width*glyph->height*4;
      size_t j;
      unsigned char* buff = calloc(sz,1);
      for (j=0; j<(sz/4); ++j)
        buff[j*4+3] = data[j];
      texture_atlas_set_region( atlas, region.x, region.y, glyph->width, glyph->height, buff, pitch*4 );
*/

/*
      free( buff);
      free( highres_data );
      free( lowres_data );
      free( data );
*/

/*
      glyph->s0       = (region.x)/(float)atlas->width;
      glyph->t0       = (region.y)/(float)atlas->height;
      glyph->s1       = (region.x + glyph->width)/(float)atlas->width;
      glyph->t1       = (region.y + glyph->height)/(float)atlas->height;
*/

      // Discard hinting to get advance
      FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
      slot = face->glyph;
      glyph->advance_x = slot->advance.x/64.0;
      glyph->advance_y = slot->advance.y/64.0;

      kh_insert(hmip, ((khash_t(hmip)*)self->glyphs), (int)glyph->charcode , glyph);

  }

  FT_UInt prev_index;
  glyph_t *prev_glyph;
  FT_Vector kerning;
  khash_t(hmip) *glyphs = (khash_t(hmip)*) self->glyphs;

  khint_t __i, __j;
  for (__i = kh_begin(glyphs); __i != kh_end(glyphs); ++__i)
  {
    if (!kh_exist(glyphs,__i)) continue;
    glyph = kh_val(glyphs,__i);
    if (glyph->charcode == (wchar_t)(-1) ) continue;
    glyph_index = FT_Get_Char_Index( face, glyph->charcode );
    vector_clear( glyph->kerning );
    for (__j = kh_begin(glyphs); __j != kh_end(glyphs); ++__j)
    {
      if (!kh_exist(glyphs,__j)) continue;
      prev_glyph = kh_val(glyphs,__j);
      if (prev_glyph->charcode == (wchar_t)(-1) ) continue;
      prev_index = FT_Get_Char_Index( face, prev_glyph->charcode );
      FT_Get_Kerning( face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning );
      if( kerning.x )
      {
        kerning_t k = {prev_glyph->charcode, kerning.x / (float)(64.0f*64.0f)};
        vector_push_back( glyph->kerning, &k );
      }
    }
  }

end:
  font_face_close(&library, &face);
  asset_delete(font_file);
}

void
sen_font_clean(font_t* self)
{
  sen_assert(self);
  glyph_t* glyph;
  kh_foreach_value( ((khash_t(hmip)*)self->glyphs), glyph, glyph_delete(glyph) );
  kh_destroy(hmip, self->glyphs);
  free(self->alphabet);
}

font_t*
sen_font_new( const char* filename,
               float size,
               const wchar_t* alphabet,
               texture_atlas_t* atlas)
{
  _logfi("+font object [%s]", filename);
  struct_malloc(font_t, self);
  sen_font_init(self, filename, size, alphabet, atlas);
  return self;
}

void
sen_font_delete(font_t* self)
{
  _logfi("-font object");
  sen_font_clean(self);
  free(self);
}



//------------------------------------------------------------------------------------------
unsigned char *
make_distance_map( unsigned char *img,
                   unsigned int width, unsigned int height )
{
    short * xdist = (short *)  malloc( width * height * sizeof(short) );
    short * ydist = (short *)  malloc( width * height * sizeof(short) );
    double * gx   = (double *) calloc( width * height, sizeof(double) );
    double * gy      = (double *) calloc( width * height, sizeof(double) );
    double * data    = (double *) calloc( width * height, sizeof(double) );
    double * outside = (double *) calloc( width * height, sizeof(double) );
    double * inside  = (double *) calloc( width * height, sizeof(double) );
    unsigned int i;

    // Convert img into double (data)
    double img_min = 255, img_max = -255;
    for( i=0; i<width*height; ++i)
    {
        double v = img[i];
        data[i] = v;
        if (v > img_max) img_max = v;
        if (v < img_min) img_min = v;
    }
    // Rescale image levels between 0 and 1
    for( i=0; i<width*height; ++i)
    {
        data[i] = (img[i]-img_min)/img_max;
    }

    // Compute outside = edtaa3(bitmap); % Transform background (0's)
    computegradient( data, width, height, gx, gy);
    edtaa3(data, gx, gy, height, width, xdist, ydist, outside);
    for( i=0; i<width*height; ++i)
        if( outside[i] < 0 )
            outside[i] = 0.0;

    // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
    memset(gx, 0, sizeof(double)*width*height );
    memset(gy, 0, sizeof(double)*width*height );
    for( i=0; i<width*height; ++i)
        data[i] = 1 - data[i];
    computegradient( data, width, height, gx, gy);
    edtaa3(data, gx, gy, height, width, xdist, ydist, inside);
    for( i=0; i<width*height; ++i)
        if( inside[i] < 0 )
            inside[i] = 0.0;

    // distmap = outside - inside; % Bipolar distance field
    unsigned char *out = (unsigned char *) malloc( width * height * sizeof(unsigned char) );
    for( i=0; i<width*height; ++i)
    {
        outside[i] -= inside[i];
        outside[i] = 128+outside[i]*16;
        if( outside[i] < 0 ) outside[i] = 0;
        if( outside[i] > 255 ) outside[i] = 255;
        out[i] = 255 - (unsigned char) outside[i];
        //out[i] = (unsigned char) outside[i];
    }

    free( xdist );
    free( ydist );
    free( gx );
    free( gy );
    free( data );
    free( outside );
    free( inside );
    return out;
}

void
distance_map2( double *data, unsigned int width, unsigned int height )
{
    short * xdist = (short *)  malloc( width * height * sizeof(short) );
    short * ydist = (short *)  malloc( width * height * sizeof(short) );
    double * gx   = (double *) calloc( width * height, sizeof(double) );
    double * gy      = (double *) calloc( width * height, sizeof(double) );
    double * outside = (double *) calloc( width * height, sizeof(double) );
    double * inside  = (double *) calloc( width * height, sizeof(double) );
    unsigned int i;

    // Compute outside = edtaa3(bitmap); % Transform background (0's)
    computegradient( data, height, width, gx, gy);
    edtaa3(data, gx, gy, width, height, xdist, ydist, outside);
    for( i=0; i<width*height; ++i)
    {
        if( outside[i] < 0.0 )
        {
            outside[i] = 0.0;
        }
    }

    // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
    memset( gx, 0, sizeof(double)*width*height );
    memset( gy, 0, sizeof(double)*width*height );
    for( i=0; i<width*height; ++i)
        data[i] = 1 - data[i];
    computegradient( data, height, width, gx, gy );
    edtaa3( data, gx, gy, width, height, xdist, ydist, inside );
    for( i=0; i<width*height; ++i )
    {
        if( inside[i] < 0 )
        {
            inside[i] = 0.0;
        }
    }

    // distmap = outside - inside; % Bipolar distance field
    float vmin = +INFINITY;
    for( i=0; i<width*height; ++i)
    {
        outside[i] -= inside[i];
        if( outside[i] < vmin )
        {
            vmin = outside[i];
        }
    }
    vmin = abs(vmin);
    for( i=0; i<width*height; ++i)
    {
        float v = outside[i];
        if     ( v < -vmin) outside[i] = -vmin;
        else if( v > +vmin) outside[i] = +vmin;
        data[i] = (outside[i]+vmin)/(2*vmin);
    }

    free( xdist );
    free( ydist );
    free( gx );
    free( gy );
    free( outside );
    free( inside );
}

float
MitchellNetravali( float x )
{
    const float B = 1/3.0, C = 1/3.0; // Recommended
    // const float B =   1.0, C =   0.0; // Cubic B-spline (smoother results)
    // const float B =   0.0, C = 1/2.0; // Catmull-Rom spline (sharper results)
    x = fabs(x);
    if( x < 1 )
         return ( ( 12 -  9 * B - 6 * C) * x * x * x
                + (-18 + 12 * B + 6 * C) * x * x
                + (  6 -  2 * B) ) / 6;
    else if( x < 2 )
        return ( (     -B -  6 * C) * x * x * x
               + (  6 * B + 30 * C) * x * x
               + (-12 * B - 48 * C) * x
               + (  8 * B + 24 * C) ) / 6;
    else
        return 0;
}


// ------------------------------------------------------------ interpolate ---
float
interpolate( float x, float y0, float y1, float y2, float y3 )
{
    float c0 = MitchellNetravali(x-1);
    float c1 = MitchellNetravali(x  );
    float c2 = MitchellNetravali(x+1);
    float c3 = MitchellNetravali(x+2);
    float r =  c0*y0 + c1*y1 + c2*y2 + c3*y3;
    return min( max( r, 0.0 ), 1.0 );
}

#undef min
#undef max
#define max(a,b) ((int)(a) > (int)(b) ? (int)(a) : (int)(b))
#define min(a,b) ((int)(a) < (int)(b) ? (int)(a) : (int)(b))
// ------------------------------------------------------------------ scale ---
int
resize( double *src_data, size_t src_width, size_t src_height,
        double *dst_data, size_t dst_width, size_t dst_height )
{
    if( (src_width == dst_width) && (src_height == dst_height) )
    {
        memcpy( dst_data, src_data, src_width*src_height*sizeof(double));
        return 0;
    }
    size_t i,j;
    float xscale = src_width / (float) dst_width;
    float yscale = src_height / (float) dst_height;
    for( j=0; j < dst_height; ++j )
    {
        for( i=0; i < dst_width; ++i )
        {
            int src_i = (int) floor( i * xscale );
            int src_j = (int) floor( j * yscale );
            int i0 = min( max( 0, src_i-1 ), src_width-1 );
            int i1 = min( max( 0, src_i   ), src_width-1 );
            int i2 = min( max( 0, src_i+1 ), src_width-1 );
            int i3 = min( max( 0, src_i+2 ), src_width-1 );
            int j0 = min( max( 0, src_j-1 ), src_height-1 );
            int j1 = min( max( 0, src_j   ), src_height-1 );
            int j2 = min( max( 0, src_j+1 ), src_height-1 );
            int j3 = min( max( 0, src_j+2 ), src_height-1 );
            float t0 = interpolate( i / (float) dst_width,
                                    src_data[j0*src_width+i0],
                                    src_data[j0*src_width+i1],
                                    src_data[j0*src_width+i2],
                                    src_data[j0*src_width+i3] );
            float t1 = interpolate( i / (float) dst_width,
                                    src_data[j1*src_width+i0],
                                    src_data[j1*src_width+i1],
                                    src_data[j1*src_width+i2],
                                    src_data[j1*src_width+i3] );
            float t2 = interpolate( i / (float) dst_width,
                                    src_data[j2*src_width+i0],
                                    src_data[j2*src_width+i1],
                                    src_data[j2*src_width+i2],
                                    src_data[j2*src_width+i3] );
            float t3 = interpolate( i / (float) dst_width,
                                    src_data[j3*src_width+i0],
                                    src_data[j3*src_width+i1],
                                    src_data[j3*src_width+i2],
                                    src_data[j3*src_width+i3] );
            float y =  interpolate( j / (float) dst_height, t0, t1, t2, t3 );
            dst_data[j*dst_width+i] = y;
        }
    }
    return 0;
}
