#include "sen.h"
#include "x_glfw_desktop_app.h"
#include "opengl.h"
#include "glfw3.h"


#if (SEN_PLATFORM == SEN_PLATFORM_WINDOWS)
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#ifndef GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WGL
#endif
#include "glfw3native.h"
#endif

#if (SEN_PLATFORM == SEN_PLATFORM_MACOS)
#ifndef GLFW_EXPOSE_NATIVE_NSGL
#define GLFW_EXPOSE_NATIVE_NSGL
#endif
#ifndef GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include "glfw3native.h"
#endif 

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:GLFW"


static GLFWwindow* mainWindow = NULL;
static GLFWmonitor* monitor = NULL;

static int init_glew();
static void init_signals();
static void destroy_signals();
static void error_callback(int id, const char* err);
static void 
iconify_callback(GLFWwindow* window, int iconified);
static void key_callback(GLFWwindow* window, 
                         int key, 
                         int scancode, 
                         int action, 
                         int mods);

static void 
mouse_callback(GLFWwindow* window, int button, int action, int modify);
static void 
mouseMove_callback(GLFWwindow* window, double x, double y);
static void 
mouseScroll_callback(GLFWwindow* window, double x, double y);

static void 
init()
{

  int frameBufferW, frameBufferH;
  const struct gl_context_attr* attrs =
  sen_view_get_attributes();

  if (mainWindow) return;

  _logfi("Desktop app initialization");

  glfwSetErrorCallback( error_callback );
  if (! glfwInit() )
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
  glfwWindowHint(GLFW_RED_BITS,attrs->r);
  glfwWindowHint(GLFW_GREEN_BITS,attrs->g);
  glfwWindowHint(GLFW_BLUE_BITS,attrs->b);
  glfwWindowHint(GLFW_ALPHA_BITS,attrs->a);
  glfwWindowHint(GLFW_DEPTH_BITS,attrs->d);
  glfwWindowHint(GLFW_STENCIL_BITS,attrs->s);

  mainWindow = glfwCreateWindow(960, 640, "hello", 0, 0);

  glfwSetWindowIconifyCallback(mainWindow, iconify_callback);
  glfwSetKeyCallback(mainWindow, key_callback);
  glfwSetMouseButtonCallback(mainWindow, mouse_callback);
  glfwSetCursorPosCallback(mainWindow, mouseMove_callback);
  glfwSetScrollCallback(mainWindow, mouseScroll_callback);


  if (!mainWindow)
  {
      glfwTerminate();
      exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(mainWindow);

  init_glew();
  
  glfwGetFramebufferSize(mainWindow, &frameBufferW, &frameBufferH);
  _logfi("Window Frame buffer size %dx%d", frameBufferW, frameBufferH);
  sen_init(frameBufferW, frameBufferH);
  init_signals();
}

static void 
loop()
{
  while (!glfwWindowShouldClose(mainWindow))
  {
    sen_process();
    glfwSwapBuffers(mainWindow);
    glfwPollEvents();
  }
}

static void 
destroy()
{
  _logfi("Desktop app terminating");
  destroy_signals();
  sen_destroy();
  if (mainWindow) {
    _logfi("Window destroy");
    glfwDestroyWindow(mainWindow);
  }
  glfwTerminate();
  mainWindow = NULL;
}

int sen_desktop_app_run()
{
  init();
  loop();
  destroy();
  return EXIT_SUCCESS;
}

void sen_desktop_app_exit()
{
  if (mainWindow) {
    glfwSetWindowShouldClose(mainWindow,1);
  }
}

#if (SEN_PLATFORM == SEN_PLATFORM_WINDOWS)
static int glew_bind()
{
    const char *gl_extensions = (const char*)glGetString(GL_EXTENSIONS);

    if (!glGenFramebuffers)
    {
        _logfe("OpenGL: glGenFramebuffers is NULL");
        if (strstr(gl_extensions, "ARB_framebuffer_object"))
        {
            _logfi("OpenGL: ARB_framebuffer_object is supported");

            glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) wglGetProcAddress("glIsRenderbuffer");
            glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbuffer");
            glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) wglGetProcAddress("glDeleteRenderbuffers");
            glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffers");
            glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorage");
            glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) wglGetProcAddress("glGetRenderbufferParameteriv");
            glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) wglGetProcAddress("glIsFramebuffer");
            glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
            glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffers");
            glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
            glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
            glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC) wglGetProcAddress("glFramebufferTexture1D");
            glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");
            glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC) wglGetProcAddress("glFramebufferTexture3D");
            glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbuffer");
            glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
            glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
        }
        else
        if (strstr(gl_extensions, "EXT_framebuffer_object"))
        {
            _logfi("OpenGL: EXT_framebuffer_object is supported");
            glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) wglGetProcAddress("glIsRenderbufferEXT");
            glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbufferEXT");
            glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) wglGetProcAddress("glDeleteRenderbuffersEXT");
            glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffersEXT");
            glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorageEXT");
            glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) wglGetProcAddress("glGetRenderbufferParameterivEXT");
            glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) wglGetProcAddress("glIsFramebufferEXT");
            glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebufferEXT");
            glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffersEXT");
            glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffersEXT");
            glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");
            glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC) wglGetProcAddress("glFramebufferTexture1DEXT");
            glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2DEXT");
            glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC) wglGetProcAddress("glFramebufferTexture3DEXT");
            glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbufferEXT");
            glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
            glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmapEXT");
        }
        else
        {
            _logfe("OpenGL: GL Framebuffers extension not found");
            return 0;
        }
    }
    return 1;
}
#endif

static int init_glew()
{
#if (SEN_PLATFORM != SEN_PLATFORM_MACOS)
    GLenum GlewInitResult = glewInit();
    if (GLEW_OK != GlewInitResult)
    {
        _logfe((char *)glewGetErrorString(GlewInitResult));
        return 0;
    }

    if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
    {
        _logfi("GLSL +");
    }
    else
    {
        _logfe("GLSL support required");
    }

    if (glewIsSupported("GL_VERSION_2_0"))
    {
        _logfi("OpenGL 2.0 +");
    }
    else
    {
        _logfe("OpenGL 2.0 support required");
    }

  #if (SEN_PLATFORM == SEN_PLATFORM_WINDOWS)
    if(! glew_bind())
    {
        _logfe("GL Framebuffer support required");
        return 0;
    }
  #endif
#endif 

  return 1;   
}

static const void* signal_touchesBegin     = NULL;
static const void* signal_touchesEnd       = NULL;
static const void* signal_touchesMove      = NULL;
static const void* signal_reload           = NULL;
static const void* signal_keyDown          = NULL;
static const void* signal_resize           = NULL;
static const void* signal_enterBackground  = NULL;
static const void* signal_enterForeground  = NULL;

static void 
init_signals()
{
  signal_resize       = sen_signal_get_name("resize", "platform");
  signal_reload       = sen_signal_get_name("reload", "platform");
  signal_touchesBegin = sen_signal_get_name("touchesBegin", "platform");
  signal_touchesEnd   = sen_signal_get_name("touchesEnd", "platform");
  signal_touchesMove  = sen_signal_get_name("touchesMove", "platform");
  signal_keyDown      = sen_signal_get_name("keyDown", "platform");
  signal_enterBackground  = sen_signal_get_name("enterBackground", "platform");
  signal_enterForeground  = sen_signal_get_name("enterForeground", "platform");
}

static void 
destroy_signals()
{
}

static void 
error_callback(int id, const char* err)
{
  _logfe("Error[id = %d]: %s", id, err);
}

static void key_callback(GLFWwindow* window, 
                         int key, 
                         int scancode, 
                         int action, 
                         int mods)
{

  if (GLFW_PRESS != action) return;

  if (key == GLFW_KEY_ESCAPE) 
  {
      glfwSetWindowShouldClose(mainWindow, GL_TRUE);
      return;
  }
   
#ifdef SEN_DEBUG
  if (key == GLFW_KEY_SPACE) 
  {
      sen_signal_emit( signal_reload, NULL );
  }
#endif
 
  sen_signal_emit( signal_keyDown, & key );
}

static void 
iconify_callback(GLFWwindow* window, int iconified)
{
  if (iconified == GL_TRUE)
  {
    sen_signal_emit( signal_enterBackground, NULL );
  }
  else
  {
    sen_signal_emit( signal_enterForeground, NULL );
  }
}

#define MAX_BUTTONS 32
static int buttons[MAX_BUTTONS];
static int mouse_captured = 0;
static float mouseX = 0.0f;
static float mouseY = 0.0f;

static void 
mouse_callback(GLFWwindow* window, int button, int action, int modify)
{
  touch_data_t td = {1, &button, &mouseX, &mouseY};

  if ( button < 0 || button >=MAX_BUTTONS ) return;

  if (GLFW_PRESS == action) {
    buttons[button] = 1;
    sen_signal_emit( signal_touchesBegin, &td );
  }
  else if (GLFW_RELEASE == action){
    buttons[button] = 0;
    sen_signal_emit( signal_touchesEnd, &td );
  }
}

static void 
mouseMove_callback(GLFWwindow* window, double x, double y)
{
  int i;
  mouseX = (float)x;
  mouseY = (float)y;

  for(i = 0; i < MAX_BUTTONS; ++i)
    if (buttons[i]) {
      touch_data_t td = {1, &i, &mouseX, &mouseY};
      sen_signal_emit( signal_touchesMove, &td );
    }
}

static void 
mouseScroll_callback(GLFWwindow* window, double x, double y)
{
}

