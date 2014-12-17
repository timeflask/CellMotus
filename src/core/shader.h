#ifndef __SHADER_H__
#define __SHADER_H__

#include "opengl.h"

typedef struct uniform_t {
  char*                  name;
  GLint                  location;
  GLenum                 type;
  GLint                  size;
  const struct shader_t* shader;
} uniform_t ;

typedef struct shader_t {
  const char*  name;
  unsigned int id;
  void*        handle;
} shader_t;

void
sen_shaders_load(const char* name,
                 const char* vs_name,
                 const char* fs_name);

void
sen_shaders_load_files(const char* name,
                       const char* vertex_shader_data,
                       const char* fragment_shader_data);

void
sen_shaders_reload();

void
sen_shaders_collect();

const shader_t*
sen_shaders_get(const char* name);

void
sen_shaders_release(const shader_t* shader);

//-----------------------------------------------------------------
void
sen_shader_use(const shader_t* shader);

const uniform_t*
sen_shader_uniform(const shader_t* shader, const char* uniform_name);
//-----------------------------------------------------------------

void
sen_uniform_1i(const struct uniform_t* uni, GLint i1);
void
sen_uniform_1iN(const shader_t* shader, const char* uniform_name, GLint f1);
void
sen_uniform_1f(const struct uniform_t* uni, GLfloat f1);
void
sen_uniform_1fN(const shader_t* shader, const char* uniform_name, GLfloat f1);
void
sen_uniform_m4f(const struct uniform_t* uni, GLfloat* m4f);
void
sen_uniform_m4fN(const shader_t* shader, const char* uniform_name, GLfloat* m4f);

//-----------------------------------------------------------------
void
sen_shaders_manager_init();
void
sen_shaders_manager_destroy();

//-----------------------------------------------------------------


#endif /* __SHADER_H__ */
