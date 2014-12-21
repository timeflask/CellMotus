#include "sen.h"
#include "x_glfw_desktop_app.h"
#include "opengl.h"
#include "glfw3.h"
#include "math.h"


#if (SEN_PLATFORM == SEN_PLATFORM_WIN32)
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
#define MAX_BUTTONS 32
static int buttons[MAX_BUTTONS];
static float mouseX = 0.0f;
static float mouseY = 0.0f;
static int is_retina_monitor = 0;
static int is_retina_on      = 0;
static int retina_scale      = 1;
static float zoom            = 1.0f;
static int windowPosX = 100;
static int windowPosY = 100;

static const void* signal_touchesBegin     = NULL;
static const void* signal_touchesEnd       = NULL;
static const void* signal_touchesMove      = NULL;
static const void* signal_reload           = NULL;
static const void* signal_keyDown          = NULL;
static const void* signal_resize           = NULL;
static const void* signal_enterBackground  = NULL;
static const void* signal_enterForeground  = NULL;
static const void* signal_scroll           = NULL;

static int init_gl();
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
size_callback(GLFWwindow *window, int width, int height);

static void
pos_callback(GLFWwindow *windows, int x, int y);

static void
scroll_callback(GLFWwindow* window, double x, double y);

static void 
frame_size_callback(GLFWwindow *window, int width, int height);

static void 
destroy();

static int 
init()
{

  int frameBufferW, frameBufferH;
  const struct gl_context_attr* attrs =
  sen_view_get_attributes();

  if (mainWindow) return 0;

  _logfi("Desktop app initialization");

  glfwSetErrorCallback( error_callback );
  if (! glfwInit() )
    return 0;
  
  glfwWindowHint(GLFW_RESIZABLE,GL_TRUE);
  glfwWindowHint(GLFW_RED_BITS,attrs->r);
  glfwWindowHint(GLFW_GREEN_BITS,attrs->g);
  glfwWindowHint(GLFW_BLUE_BITS,attrs->b);
  glfwWindowHint(GLFW_ALPHA_BITS,attrs->a);
  glfwWindowHint(GLFW_DEPTH_BITS,attrs->d);
  glfwWindowHint(GLFW_STENCIL_BITS,attrs->s);
  
  
  mainWindow = glfwCreateWindow(960, 640, "Cell Motus", 0, 0);

  if (!mainWindow)
  {
      glfwTerminate();
      return 0;
  }
  glfwSetWindowPos(mainWindow, windowPosX, windowPosY);

  glfwSetWindowIconifyCallback(mainWindow, iconify_callback);
  glfwSetKeyCallback(mainWindow, key_callback);
  glfwSetMouseButtonCallback(mainWindow, mouse_callback);
  glfwSetCursorPosCallback(mainWindow, mouseMove_callback);
  glfwSetWindowSizeCallback(mainWindow, size_callback);
  glfwSetFramebufferSizeCallback(mainWindow, frame_size_callback);
  glfwSetScrollCallback(mainWindow, scroll_callback);
  glfwSetWindowPosCallback(mainWindow, pos_callback);

  glfwMakeContextCurrent(mainWindow);


  if (!init_gl())
  {
    destroy();
    return 0;
  }
  
  glfwGetFramebufferSize(mainWindow, &frameBufferW, &frameBufferH);
  _logfi("Window Frame buffer size %dx%d", frameBufferW, frameBufferH);
  sen_init(frameBufferW, frameBufferH);
  init_signals();

  return 1;
  //glfwSwapInterval(1);
}

static double max_fps_t = 1/60.0;
static void 
loop()
{
  double ldt = 0.0f;
  double sdt = 0.0f;
  while (!glfwWindowShouldClose(mainWindow))
  {
    glfwSetTime(0.0f);
    sen_process();
    glfwSwapBuffers(mainWindow);
    glfwPollEvents();
    ldt = glfwGetTime();
    sdt = max_fps_t - ldt;
    if (sdt > 0) {
      //_logfi("%f", (sdt * 1000));
#if (SEN_PLATFORM == SEN_PLATFORM_WIN32)
      Sleep( (DWORD)(sdt * 1000) );
#endif
    }
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
  if (!init()) return EXIT_FAILURE;
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

static int glew_bind()
{
#if (SEN_PLATFORM == SEN_PLATFORM_WIN32)
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
#endif
    return 1;
}

static int init_gl()
{
  float ver;
#if (SEN_PLATFORM != SEN_PLATFORM_MAC)
  GLenum GlewInitResult;
#endif
  _logfi("Init OpenGL...");
  ver = atof( (const char*)glGetString(GL_VERSION) );
  _logfi("OpenGL v.%.1f", ver);

  if ( ver < 2.0 )
  {
    _logfe("OpenGL 2.0 or higher is required, update your drivers.");
    return 0;
  }


#if (SEN_PLATFORM != SEN_PLATFORM_MAC)
    glewExperimental = GL_TRUE;
    GlewInitResult = glewInit();
    _logfi("Init GLEW...");
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
        return 0;
    }

    if (glewIsSupported("GL_VERSION_2_0"))
    {
        _logfi("OpenGL 2.0 +");
    }
    else
    {
        _logfe("OpenGL 2.0 support required");
        return 0;
    }

  #if (SEN_PLATFORM == SEN_PLATFORM_WIN32)
    if(! glew_bind())
    {
        _logfe("GL Framebuffer support required");
        return 0;
    }
  #endif
#endif 
  return 1;   
}

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
  signal_scroll           = sen_signal_get_name("scroll", "input");
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
size_callback(GLFWwindow *window, int width, int height)
{
  vec2 size = {{(float)width,(float)height}};
  _logfi("resize %dx%d", width, height);
  if (!width || !height) return;
  sen_signal_emit( signal_resize, &size );
}

static void 
frame_size_callback(GLFWwindow *window, int width, int height)
{

  const vec4* vp = sen_view_get_viewport();
  float frameW = vp->z;
  float frameH = vp->w;
  float fX = frameW/width*retina_scale*zoom;
  float fY = frameH/height*retina_scale*zoom;

  if (frameW<=0 || frameH<=0) return;

  if (fabs(fX - 0.5f) < F_EPSILON &&
      fabs(fY - 0.5f) < F_EPSILON )
  {
      is_retina_monitor = 1;
      retina_scale = is_retina_on ? 1 : 2;
      glfwSetWindowSize(mainWindow,
                        (int)(frameW * 0.5f * retina_scale * zoom) ,
                        (int)(frameH * 0.5f * retina_scale * zoom));
  }
  else if(fabs(fX - 2.0f) < F_EPSILON &&
          fabs(fY - 2.0f) < F_EPSILON)
  {
      is_retina_monitor = 0;
      retina_scale = 1;
      glfwSetWindowSize(mainWindow,
          (int)(frameW * retina_scale * zoom),
          (int)(frameH * retina_scale * zoom));
  }

  //vec2 size = {{(float)width,(float)height}};
  //sen_signal_emit( signal_resize, &size );
}

static void
pos_callback(GLFWwindow *windows, int x, int y)
{
  windowPosX = x;
  windowPosY = y;
}

static
void scroll_callback(GLFWwindow* window, double x, double y)
{
  struct scroll_data {double x; double y;} data = {x,y};
  sen_signal_emit( signal_scroll, &data);
}
