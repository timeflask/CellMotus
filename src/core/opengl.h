#ifndef __gl_H_
#define __gl_H_
#include "config.h"


#if SEN_PLATFORM == SEN_PLATFORM_MAC

  #import <OpenGL/gl.h>
  #import <OpenGL/glu.h>
  #import <OpenGL/glext.h>

  #define glDeleteVertexArrays            glDeleteVertexArraysAPPLE
  #define glGenVertexArrays               glGenVertexArraysAPPLE
  #define glBindVertexArray               glBindVertexArrayAPPLE
  #define glClearDepthf                   glClearDepth
  #define glDepthRangef                   glDepthRange
  #define glReleaseShaderCompiler(xxx)

#elif SEN_PLATFORM == SEN_PLATFORM_WIN32
  #define GLEW_STATIC
  #include "glew.h"
#elif SEN_PLATFORM == SEN_PLATFORM_LINUX
  #define GLEW_STATIC
  #include "glew.h"
//  #include <GL/glew.h>
#elif SEN_PLATFORM == SEN_PLATFORM_ANDROID

  #define GL_DEPTH24_STENCIL8	 GL_DEPTH24_STENCIL8_OES
  #define GL_WRITE_ONLY	GL_WRITE_ONLY_OES

  #include <GLES2/gl2platform.h>
  #ifndef GL_GLEXT_PROTOTYPES
    #define GL_GLEXT_PROTOTYPES 1
  #endif

  #include <GLES2/gl2.h>
  #include <GLES2/gl2ext.h>
  #include <EGL/egl.h>

  typedef char GLchar;
  #ifndef GL_BGRA
    #define GL_BGRA  0x80E1
  #endif

#elif SEN_PLATFORM == SEN_PLATFORM_IOS

  #define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
  #define GL_WRITE_ONLY	GL_WRITE_ONLY_OES

  #include <OpenGLES/ES2/gl.h>
  #include <OpenGLES/ES2/glext.h>

#endif 


#endif
