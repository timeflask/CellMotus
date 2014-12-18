#include "image.h"
#include "asset.h"
#include "logger.h"
#include "macro.h"
#include "asset.h"
#include "utils.h"
#include <png.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  png_byte* data;
  png_size_t size;
} DataHandle;

typedef struct {
  DataHandle data;
  png_size_t offset;
} ReadDataHandle;

typedef struct {
  png_uint_32 width;
  png_uint_32 height;
  int color_type;
  int bit_depth;
  int channels;
  int alpha;
} PngInfo;

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:PNGImage"

static void read_png_data_callback(
  png_structp png_ptr, png_byte* png_data, png_size_t read_length);
static PngInfo read_and_update_info(const png_structp png_ptr, const png_infop info_ptr);
static DataHandle read_entire_png_image(
  const png_structp png_ptr, const png_infop info_ptr, const png_uint_32 height);
static GLenum get_gl_color_format(const int png_color_format);
typedef struct {
  const int width;
  const int height;
  const int size;
  const GLenum gl_color_format;
  const void* data;
} RawImageData;


void image_load(image_t* self) {

  asset_t* source = asset_new(self->filename);
  const void* png_data = source->data;
  png_size_t png_data_size = source->size;
  png_structp png_ptr; png_infop info_ptr;
  PngInfo png_info;
  DataHandle raw_image;

  ReadDataHandle png_data_handle;
  png_data_handle.data.data = (png_byte*)png_data;
  png_data_handle.data.size = png_data_size;
  png_data_handle.offset = 0;


  sen_assert(png_data != NULL && png_data_size > 8);
  _logfi("%d bytes loaded from  %s", png_data_size,  self->filename);

  sen_assert(png_check_sig((png_const_bytep)png_data, 8));

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  sen_assert(png_ptr != NULL);
  info_ptr = png_create_info_struct(png_ptr);
  sen_assert(info_ptr != NULL);


  png_set_read_fn(png_ptr, &png_data_handle, read_png_data_callback);
  if (setjmp(png_jmpbuf(png_ptr))) {
    _logfe("Error reading PNG file %s", self->filename);
    asset_delete(source);
    exit(1);
  //return;
  }

  png_info = read_and_update_info(png_ptr, info_ptr);
  raw_image = read_entire_png_image(png_ptr, info_ptr, png_info.height);

  png_read_end(png_ptr, info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  self->width = png_info.width;
  self->height = png_info.height;
  self->size =  raw_image.size;
  self->gl_color_fmt = get_gl_color_format(png_info.color_type);
  self->raw_data = (void*)raw_image.data;
  self->channels = png_info.channels;
  self->channel_bit_depth = png_info.bit_depth;
  self->stride =  self->width * self->channel_bit_depth * self->channels / 8;
  _logfi( "Image: %s  w,h = %d, %d  channels=%d  ncbp=%d  depth=%d",
      self->filename,
      self->width, self->height,
      self->channels,
      self->channel_bit_depth,
      self->channels*self->channel_bit_depth/8);
  asset_delete(source);
}

image_t* image_new(const char* filename) {
  struct_malloc(image_t, self);
  sen_assert(filename);
  _logfi("New image \"%s\"", filename);
  self->filename = sen_strdup( filename );
  self->raw_data = 0;
  image_load(self);
  return self;
}

void image_delete(image_t* self) {
  if (self->raw_data) {
    _logfi("Free \"%s\" image data...", self->filename);
    free(self->raw_data);
  }
  free(self->filename);
  free(self);
}


static void read_png_data_callback(png_structp png_ptr, png_byte* raw_data, png_size_t read_length) {
  ReadDataHandle* handle = (ReadDataHandle*)png_get_io_ptr(png_ptr);
  const png_byte* png_src = handle->data.data + handle->offset;

  memcpy(raw_data, png_src, read_length);
  handle->offset += read_length;
}

static PngInfo read_and_update_info(const png_structp png_ptr, const png_infop info_ptr) {
  png_uint_32 width, height;
  int bit_depth, color_type, alpha=1;
  PngInfo ret;

  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);


  // Convert transparency to full alpha
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    _logfi(" libpng: +Convert transparency to full alpha");
    png_set_tRNS_to_alpha(png_ptr);
  }


  // Convert grayscale, if needed.
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
    _logfi(" libpng: +Convert grayscale");
    png_set_expand_gray_1_2_4_to_8(png_ptr);
  }

  // Convert paletted images, if needed.
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    _logfi(" libpng: +Convert palette");
    png_set_palette_to_rgb(png_ptr);

  }

  if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
      _logfi(" libpng: +Convert gray to rgb ");
       png_set_gray_to_rgb(png_ptr);
       if (color_type == PNG_COLOR_TYPE_GRAY) {
         _logfi(" libpng: +add 0xFF alpha channel ");
         png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
         alpha = 0;
       }
  } else
  {
  // Add alpha channel, if there is none (rationale: GL_RGBA is faster than GL_RGB on many GPUs)
    if (color_type == PNG_COLOR_TYPE_PALETTE || color_type == PNG_COLOR_TYPE_RGB) {
      _logfi(" libpng: +add 0xFF alpha channel ");
       png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
       alpha = 0;
    }
  }

  // Ensure 8-bit packing
  if (bit_depth < 8)
     png_set_packing(png_ptr);
  //else if (bit_depth == 16)
    //png_set_scale_16(png_ptr);

  png_read_update_info(png_ptr, info_ptr);

  // Read the new color type after updates have been made.
  ret.alpha = alpha;
  ret.bit_depth = bit_depth;
  ret.channels = png_get_channels(png_ptr, info_ptr);
  ret.color_type = png_get_color_type(png_ptr, info_ptr);
  ret.height = height;
  ret.width = width;
  return ret;
}

static DataHandle read_entire_png_image(const png_structp png_ptr, const png_infop info_ptr, const png_uint_32 height) {
  const png_size_t row_size = png_get_rowbytes(png_ptr, info_ptr);
  const png_size_t data_length = row_size * height;
  png_byte* raw_image;
  png_byte** row_ptrs = malloc(sizeof(png_byte*) * height); //[height];
  png_uint_32 i;
  DataHandle ret;

  sen_assert(row_size > 0);

  raw_image = (png_byte*)malloc(data_length);
  sen_assert(raw_image != NULL);


  for (i = 0; i < height; i++) {
    row_ptrs[i] = raw_image + i * row_size;
  }

  png_read_image(png_ptr, &row_ptrs[0]);
  free(row_ptrs);

  ret.data = raw_image;
  ret.size = data_length;
  return ret;
}

static GLenum get_gl_color_format(const int png_color_format) {
/*
  sen_assert(png_color_format == PNG_COLOR_TYPE_GRAY
      || png_color_format == PNG_COLOR_TYPE_RGB_ALPHA
      || png_color_format == PNG_COLOR_TYPE_GRAY_ALPHA);
*/
  sen_assert(png_color_format == PNG_COLOR_TYPE_RGB_ALPHA);

  switch (png_color_format) {
    case PNG_COLOR_TYPE_GRAY:
      return GL_LUMINANCE;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      return GL_RGBA;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      return GL_LUMINANCE_ALPHA;
  }

  return 0;
}
