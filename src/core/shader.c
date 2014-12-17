#include <stdlib.h>
#include <string.h>
#include "macro.h"
#include "asset.h"
#include "logger.h"
#include "opengl.h"
#include "shader.h"
#include "utils.h"
#include "khash.h"
#include "lmath.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:ShadersManager"

//-------------------------------------------------------------------------------------
#include "shaders/vs_pos4f_tex2f.h"
#include "shaders/fs_tex2f.h"
#include "shaders/vs_pos4f_tex2f_col4f.h"
#include "shaders/vs_pos4f_col4f.h"
#include "shaders/fs_tex2f_col4f.h"
#include "shaders/fs_text_tex2f_col4f.h"
#include "shaders/fs_col4f.h"

typedef struct preloaded_shader_t {
  const char*   name;
  const GLchar* data;
  const int     size;
} preloaded_shader_t;

static const struct preloaded_shader_t  g_preloaded_shaders[] =
{
    {"vs_pos4f_tex2f",        SEN_vs_pos4f_tex2f,       sizeof(SEN_vs_pos4f_tex2f)},
    {"vs_pos4f_col4f",        SEN_vs_pos4f_col4f,       sizeof(SEN_vs_pos4f_col4f)},
    {"fs_tex2f",              SEN_fs_tex2f,             sizeof(SEN_fs_tex2f)},
    {"vs_pos4f_tex2f_col4f",  SEN_vs_pos4f_tex2f_col4f, sizeof(SEN_vs_pos4f_tex2f_col4f)},
    {"fs_tex2f_col4f",        SEN_fs_tex2f_col4f,       sizeof(SEN_fs_tex2f_col4f)},
    {"fs_text_tex2f_col4f",   SEN_fs_text_tex2f_col4f,  sizeof(SEN_fs_text_tex2f_col4f)},
    {"fs_col4f",              SEN_fs_col4f,             sizeof(SEN_fs_col4f)},
};
static const int g_preloaded_shaders_N = sizeof g_preloaded_shaders / sizeof g_preloaded_shaders[0];
//-------------------------------------------------------------------------------------


typedef struct uniform_datamap_t {
  void* data;
  int   size;
} uniform_datamap_t;

#define DATAMAP_MAX 32
typedef struct shader_data_t {
  char*              name;
  int                refcount;
  char*              vdata;
  char*              fdata;
  GLuint             program_id;
  shader_t           shader;
  khash_t(hmsp)*     uniforms;
  uniform_datamap_t  datamap[DATAMAP_MAX];
} shader_data_t;

static GLuint shader_data_compile( const char* src, const GLenum type );
static void   shader_data_parse_attrs(shader_data_t* self);
static void   shader_data_parse_uniforms(shader_data_t* self);

uniform_t*
uniform_data_new( const shader_t* parent, const char* name, GLint loc, GLenum type, GLint size)
{
  struct_malloc(uniform_t, self);
  self->name = strdup(name); //!!!
  self->location = loc;
  self->size = size;
  self->type = type;
  self->shader = parent;
  return self;
}

static void
uniform_data_delete(uniform_t* self)
{
  free(self->name);
  free(self);
}

static void
shader_data_clear_uniforms(shader_data_t* self)
{
  uniform_t* data;
  kh_foreach_value(self->uniforms, data, uniform_data_delete(data) );
  kh_clear(hmsp, self->uniforms);
  int i;
  for (i = 0; i < DATAMAP_MAX; ++i)
    if (self->datamap[i].data)
      free(self->datamap[i].data);
  memset( self->datamap, 0x00, sizeof(self->datamap));
}

static void
shader_data_reload(shader_data_t* self)
{
  sen_assert(self);

  self->program_id = glCreateProgram( );
  sen_assert(self->program_id != 0);
  GLint link_status;

  GLuint vert_id = shader_data_compile( self->vdata, GL_VERTEX_SHADER);
  GLuint frag_id = shader_data_compile( self->fdata, GL_FRAGMENT_SHADER);

  glAttachShader( self->program_id, vert_id );
  glAttachShader( self->program_id, frag_id );

  glLinkProgram(  self->program_id  );

  glDetachShader( self->program_id, vert_id );
  glDetachShader( self->program_id, frag_id );

  glGetProgramiv( self->program_id, GL_LINK_STATUS, &link_status );
  if (link_status == GL_FALSE)
  {
      GLchar messages[256];
      glGetProgramInfoLog( self->program_id, sizeof(messages), 0, &messages[0] );
      _logfe( "%s\n", messages );
      exit(EXIT_FAILURE);
  }

  glDeleteShader(vert_id);
  glDeleteShader(frag_id);

  shader_data_parse_attrs(self);
  shader_data_parse_uniforms(self);

  self->shader.id = self->program_id;

  gl_check_error();

  _logfi( " program [%s] linked, id=%u", self->name, self->program_id);
}

static int uniform_datamap_update(const uniform_t* uni, const GLvoid* p, int size)
{
  sen_assert(uni);
  if (uni->location < 0 || uni->location >= DATAMAP_MAX) return 0;
  uniform_datamap_t* ud = &(((shader_data_t*) uni->shader->handle)->datamap[uni->location]);
  if ( ud->data == 0 ) {
    ud->data = malloc(size);
    ud->size = size;
    memcpy(ud->data, p, size);
    return 1;
  }

  if (memcmp(ud->data, p, size) == 0)
    return 0;

  memcpy(ud->data, p, size);

  return 1;
}


const uniform_t*
sen_shader_uniform(const shader_t* shader, const char* uniform_name)
{
  sen_assert(shader);
  sen_assert(uniform_name);
  shader_data_t* data = (shader_data_t*)(shader->handle);
  khiter_t k = kh_get(hmsp, data->uniforms, uniform_name);
  if ( k == kh_end(data->uniforms) ) {
    _logfw("uniform [%s] not found in [%s]", uniform_name, shader->name);
    return 0;
  }
  return ((uniform_t*)(kh_val(data->uniforms, k)));
}


shader_data_t*
shader_data_new(const char* name,
                const char* vdata,
                const int   vsize,
                const char* fdata,
                const int   fsize)
{
  sen_assert( vdata && fdata );
  struct_malloc(shader_data_t, self);
  self->vdata = (char*)calloc(sizeof(char), vsize+1);
  memcpy(self->vdata, vdata, vsize);
  self->fdata = (char*)calloc(sizeof(char), fsize+1);
  memcpy(self->fdata, fdata, fsize);
  self->name          = strdup(name);

  self->refcount      = 0;
  self->uniforms = kh_init(hmsp);
  kh_resize(hmsp, self->uniforms, 16);
  memset( self->datamap, 0x00, sizeof(self->datamap));

  shader_data_reload(self);

  self->shader.handle = self;
  self->shader.name   = self->name;


  return self;
}

void
shader_data_delete(shader_data_t* self)
{
  sen_assert(self);
  _logfi(" -[%s] id=%u", self->name, self->program_id);
  shader_data_clear_uniforms(self);
  kh_destroy(hmsp, self->uniforms);
  if (self->program_id)
    glDeleteProgram(self->program_id);

  free(self->vdata);
  free(self->fdata);
  free(self->name);
  free(self);
}


//-------------------------------------------------------------------------------------------
typedef struct shader_manager_t{
  khash_t(hmsp) *shaders;
} shader_manager_t;

static shader_manager_t* g_self = 0;

static inline const preloaded_shader_t*
preloaded_shader_find(const char* name)
{
  int i;
  for (i = 0; i < g_preloaded_shaders_N; ++i )
    if (0==strcmp(name, g_preloaded_shaders[i].name ))
      return  & g_preloaded_shaders[i];
  return 0;
}

shader_manager_t*
shader_manager_new()
{
  struct_malloc(shader_manager_t, self);
  self->shaders = kh_init(hmsp);
  kh_resize(hmsp, self->shaders, 32);
  return self;
}

void
shader_manager_delete(shader_manager_t* self)
{
  sen_assert(self);
  shader_data_t* data;
  kh_foreach_value(self->shaders, data, shader_data_delete(data) );
  kh_destroy(hmsp, self->shaders);

  free(self);
}

void
shader_manager_load_shader( const char * name,
                     const char * vdata,
                     const int    vdata_size,
                     const char * fdata,
                     const int    fdata_size)
{
  sen_assert(name);
  sen_assert(vdata && vdata_size);
  sen_assert(fdata && fdata_size);
  khiter_t k = kh_get(hmsp, g_self->shaders, name);
  if (k != kh_end(g_self->shaders)) {
    _logfi("shader [%s] already in use", name);
    return;
  }

  shader_data_t* new_data = shader_data_new(
      name,
      vdata, vdata_size,
      fdata, fdata_size);

  kh_insert(hmsp, g_self->shaders, new_data->name, new_data);
  _logfi("+shader program [%s], id=%u", new_data->name, new_data->program_id);
}


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void
sen_shaders_load(const char* name,
                 const char* vs_name,
                 const char* fs_name)
{
  const preloaded_shader_t* vsp = preloaded_shader_find(vs_name);
  sen_assert(vsp);
  const preloaded_shader_t* fsp = preloaded_shader_find(fs_name);
  sen_assert(fsp);
  shader_manager_load_shader(name, vsp->data, vsp->size, fsp->data, fsp->size);
}

void
sen_shaders_load_files(const char* name,
                       const char* vertex_shader_filename,
                       const char* fragment_shader_filename)
{
  asset_t* vert_source = asset_new (vertex_shader_filename);
  asset_t* frag_source = asset_new (fragment_shader_filename);

  shader_manager_load_shader(name,
                             vert_source->data,
                             vert_source->size,
                             frag_source->data,
                             frag_source->size);

  asset_delete(vert_source);
  asset_delete(frag_source);
}

const shader_t*
sen_shaders_get(const char* name)
{
  sen_assert(name);
  khiter_t k = kh_get(hmsp, g_self->shaders, name );
  if (k == kh_end(g_self->shaders))
    return 0;
  shader_data_t* data = (shader_data_t*) kh_val( g_self->shaders, k );
  data->refcount++;
  return & (data->shader);
}

void
sen_shaders_release(const shader_t* shader){
  sen_assert(shader);
  khiter_t k = kh_get(hmsp, g_self->shaders, shader->name );
  if (k == kh_end(g_self->shaders))
    return ;
  shader_data_t* data = (shader_data_t*) kh_val( g_self->shaders, k );
  if ( data->refcount ) data->refcount--;
}

static const shader_t* l_shader = 0;

void
sen_shader_use(const shader_t* shader){
  if (shader) {
    if (shader == l_shader) return;

    glUseProgram(shader->id);
    l_shader = shader;

  }
  else {
    glUseProgram(0);
    l_shader = 0;
  }
}

void
sen_shaders_reload() {
  _logfi("*** RELOAD SHADERS ***");
  sen_assert(g_self);
  shader_data_t* data;
  kh_foreach_value(g_self->shaders, data, shader_data_reload(data) );
  sen_shader_use(NULL);
  _logfi("*** END RELOAD SHADERS ***");
}

void
sen_shaders_collect()
{
  _logfi("*** COLLECT SHADERS ***");
  sen_assert(g_self);
  shader_data_t* data; int exit_flag;
  do {
    exit_flag = 0;
    kh_foreach_value(g_self->shaders, data,
        if (data->refcount == 0) {
          shader_data_delete(data);
          data = 0; exit_flag = 1;
          kh_del(hmsp, g_self->shaders, __i);
          break;
        }
    );
  } while (exit_flag);
  _logfi("*** END COLLECT SHADERS ***");

}
// ---------------------------------------------------------

void
sen_shaders_manager_init() {
  if (g_self == 0) {
    _logfi("Init shader programs manager");
    g_self = shader_manager_new();
  }
}

void
sen_shaders_manager_destroy() {
  if (g_self) {
    _logfi("Destroy shader programs manager");
    shader_manager_delete(g_self); g_self = 0;
  }
}

// ---------------------------------------------------------
static GLuint
shader_data_compile( const char* src, const GLenum type )
{

  const GLchar *source[] = {
//#if (SEN_PLATFORM != SEN_PLATFORM_WIN32 && SEN_PLATFORM != SEN_PLATFORM_LINUX && SEN_PLATFORM != SEN_PLATFORM_MAC)
      //  (type == GL_VERTEX_SHADER ? "precision highp float;\n precision highp int;\n" : "precision mediump float;\n precision mediump int;\n"),
//#endif

#ifdef GL_ES_VERSION_2_0
    "#version 100\n"
#else
    "#version 120\n"
#endif
    ,
#ifdef GL_ES_VERSION_2_0
    (type == GL_FRAGMENT_SHADER) ?
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "precision highp float;           \n"
    "#else                            \n"
    "precision mediump float;         \n"
    "#endif                           \n"
    : ""
#else
    "#define lowp   \n"
    "#define mediump\n"
    "#define highp  \n"
#endif

        "uniform mat4 u_mvp;\n",
        "uniform sampler2D u_tex0;\n",
        "//------------------------------------\n",
      (const GLchar*)src,
  };


  GLint compile_status;
  GLuint handle = glCreateShader( type );
  sen_assert(handle != 0);
  //const GLchar * data_ptr = (const GLchar *) src;
  glShaderSource( handle, sizeof(source)/sizeof(*source), source, 0);
  glCompileShader( handle );

  glGetShaderiv( handle, GL_COMPILE_STATUS, &compile_status );
  if( compile_status == GL_FALSE )
  {
      GLchar messages[256];
      glGetShaderInfoLog( handle, sizeof(messages), 0, &messages[0] );
      _logfe( "%s\n", messages );
      exit( EXIT_FAILURE );
  }
  return handle;
}

static void
shader_data_parse_attrs(shader_data_t* self)
{
  GLint attr; GLint len;
  glGetProgramiv(self->program_id, GL_ACTIVE_ATTRIBUTES, &attr);
  if(attr > 0)
  {
    glGetProgramiv(self->program_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &len);
    if(len > 0)
    {
      GLchar* name = (GLchar*) malloc(len + 1);
      int i;
      for(i=0; i < attr; ++i)
      {
          GLint size; GLenum type;
          glGetActiveAttrib(self->program_id, i, len, 0, &size, &type, name);
          name[len] = '\0';
          _logfi(" ATTR[%s], index=%u", name, size, glGetAttribLocation(self->program_id, name));
      }
      free(name);
    }
  }
}

static void   shader_data_parse_uniforms(shader_data_t* self)
{
  GLint unis;
  shader_data_clear_uniforms(self);
  glGetProgramiv(self->program_id, GL_ACTIVE_UNIFORMS, &unis);
  if(unis > 0) {
      GLint len;
      glGetProgramiv(self->program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &len);
      if(len > 0)
      {
          GLchar* name = (GLchar*)malloc(len + 1);
          int i;
          for(i = 0; i < unis; ++i)
          {
            GLint size; GLenum type;
            glGetActiveUniform(self->program_id, i, len, 0, &size, &type, name);
            name[len] = '\0';
            if(size > 1 && len > 3)
            {
              char* c = strrchr(name, '[');
              if(c) *c = '\0';
            }
            GLint loc = glGetUniformLocation(self->program_id, name);
            _logfi(" UNI[%s], loc=%d", name, loc);
            if (loc >= 0) {
              uniform_t* new_uni = uniform_data_new(&(self->shader), name, loc, type, size );
              kh_insert(hmsp, self->uniforms, new_uni->name, new_uni );
            }
          }
          free(name);
      }
  }
}
//--------------------------------------------------------------------------------------------

void
sen_uniform_1i(const struct uniform_t* uni, GLint i1)
{
  if ( uniform_datamap_update(uni, &i1, sizeof(GLint) ) ) {
    glUniform1i( uni->location, i1 );
   // gl_check_error();
  }
  //glUniform1i( uni->location, i1 );
}
void
sen_uniform_1iN(const shader_t* shader, const char* uniform_name, GLint i1)
{
  sen_uniform_1i ( sen_shader_uniform(shader, uniform_name), i1 );
}

void
sen_uniform_1f(const struct uniform_t* uni, GLfloat f1)
{
  if ( uniform_datamap_update(uni, &f1, sizeof(GLfloat) ) ) {
    glUniform1f( uni->location, f1 );
    //gl_check_error();
  }
}

void
sen_uniform_1fN(const shader_t* shader, const char* uniform_name, GLfloat f1)
{
  sen_uniform_1f ( sen_shader_uniform(shader, uniform_name), f1 );
}


void
sen_uniform_m4f(const struct uniform_t* uni, GLfloat* m4f)
{
  if ( uniform_datamap_update(uni, m4f, sizeof(mat4) ) ) {
    glUniformMatrix4fv(   uni->location, 1, 0, m4f );
    //gl_check_error();
  }
}
void
sen_uniform_m4fN(const shader_t* shader, const char* uniform_name, GLfloat* m4f)
{
  sen_uniform_m4f ( sen_shader_uniform(shader, uniform_name), m4f );
}

//--------------------------------------------------------------------------------------------

