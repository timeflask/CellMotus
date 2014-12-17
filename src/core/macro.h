#ifndef __macro_H_
#define __macro_H_
#include "config.h"
#include <assert.h>
#ifdef SEN_DEBUG
  #define sen_assert(condition) \
  if (!(condition)) {_logfe("SEN:Assertion!\nCondition: %s\n[%d] %s\n", #condition, __LINE__, __FILE__); assert(0); }  else (void) 0;
#else
  #define sen_assert(condition)
#endif

#ifdef SEN_DEBUG
  #define sen_assert_msg(condition, msg) \
  if (!(condition)) {_logfe("SEN:Assertion \nMSG:%s\nCondition: %s\n[%d] %s\n",msg, #condition, __LINE__, __FILE__); assert(0); }  else (void) 0;
#else
  #define sen_assert_msg(condition, msg)
#endif

#ifndef UNUSED
  #define UNUSED(x) (void)(x)
#endif

#define struct_malloc(t,o)                                                            \
  t* o = (t*)malloc(sizeof(t));                                                       \
  do { if( !o )                                                                       \
  {                                                                                   \
    _logfe("Sys:malloc", "line %d: No more memory for allocating data\n", __LINE__ ); \
    exit( EXIT_FAILURE );                                                             \
  } } while (0)                                                                       \


#define struct_mallocG(t,global)                                                      \
  global= (t*)malloc(sizeof(t));                                                      \
  do { if( !global )                                                                  \
  {                                                                                   \
    _logfe("Sys:malloc", "line %d: No more memory for allocating data\n", __LINE__ ); \
    exit( EXIT_FAILURE );                                                             \
  } } while (0)                                                                       \


#ifdef SEN_LOGGER_ENABLE
  #define _logfv(FMT,...) do { sen_logv(SEN_LOG_TAG, FMT, ## __VA_ARGS__); } while (0)
  #define _logfd(FMT,...) do { sen_logd(SEN_LOG_TAG, FMT, ## __VA_ARGS__); } while (0)
  #define _logfi(FMT,...) do { sen_logi(SEN_LOG_TAG, FMT, ## __VA_ARGS__); } while (0)
  #define _logfw(FMT,...) do { sen_logw(SEN_LOG_TAG, FMT, ## __VA_ARGS__); } while (0)
  #define _logfe(FMT,...) do { sen_loge(SEN_LOG_TAG, FMT, ## __VA_ARGS__); } while (0)
#else
  #define _logfv(FMT,...)
  #define _logfd(FMT,...)
  #define _logfi(FMT,...)
  #define _logfw(FMT,...)
  #define _logfe(FMT,...)
#endif


#ifndef F_EPSILON
#define F_EPSILON     1.192092896e-07F
#endif


#ifndef SEN_DEBUG
#define gl_check_error()
#else
#define gl_check_error() \
    do { \
        GLenum __error = glGetError(); \
        if(__error != GL_NO_ERROR) { \
            _logfe("OpenGL error 0x%04X, [%d]%s\n%s\n", __error, __LINE__, __FILE__, __FUNCTION__); \
            switch(__error) { \
                    case GL_INVALID_OPERATION: _logfe("-- hint: GL_INVALID_OPERATION");break; \
                    case GL_INVALID_ENUM:_logfe("-- hint: INVALID_ENUM");           break; \
                    case GL_INVALID_VALUE:_logfe("-- hint: INVALID_VALUE");          break; \
                    case GL_OUT_OF_MEMORY: _logfe("-- hint: OUT_OF_MEMORY");               break; \
                    case GL_INVALID_FRAMEBUFFER_OPERATION:  _logfe("-- hint: INVALID_FRAMEBUFFER_OPERATION");   break; \
            } assert(0);\
        } else break;\
    } while (1)
#endif

#define BUFFER_OFFSET(i) ((void*)(i))

#define TO_STRING(x) #x


#define SEN_DEG_TO_RAD(A) ((A) * 0.01745329252f)

#endif
