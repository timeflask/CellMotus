#ifndef __label_H_
#define __label_H_
#include "node.h"
#include "font.h"
#include "vertex-buffer.h"
#include "shader.h"
#include "vector.h"
#include "types.h"

typedef struct label_t {
  node_t            super;
  const font_t*     font;
  vertex_buffer_t*  buff;
  const shader_t*   program;
  wchar_t*          text;
  size_t            text_length;
  vector_t*         verts;
} label_t;

label_t *
sen_label_new(const char* name,
              const char* font_name,
              const char* utf8_text);

label_t *
sen_label_newW(const char* name,
              const char* font_name,
              const wchar_t* text);
void
sen_label_delete(void *self);

void
sen_label_render(void* self);

void
sen_label_set_textW(void* self, const wchar_t* text);
void
sen_label_set_text(void* self, const char* utf8_text);


#endif
