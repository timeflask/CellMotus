#include "sen.h"
#include "x_glfw_desktop_app.h"
#include "opengl.h"
#include <string.h>

#if (SEN_PLATFORM == SEN_PLATFORM_WIN32)
  //#define GLFW_DLL
#endif

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

#if (SEN_PLATFORM == SEN_PLATFORM_MAC)
#ifndef GLFW_EXPOSE_NATIVE_NSGL
#define GLFW_EXPOSE_NATIVE_NSGL
#endif
#ifndef GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include "glfw3native.h"
#endif 

#if (SEN_PLATFORM == SEN_PLATFORM_LINUX)
#include <time.h>
#endif

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:GLFW"


static GLFWwindow* mainWindow = NULL;
#define MAX_BUTTONS 32
static int buttons[MAX_BUTTONS];
static float mouseX = 0.0f;
static float mouseY = 0.0f;

static desktop_app_config_t g_cfg;

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

static const char init_script[] = "assets/scripts/init0.lua";
static const char deinit_script[] = "assets/scripts/deinit0.lua";




static int 
init(const desktop_app_config_t* config)
{

  int frameBufferW, frameBufferH;

  if (mainWindow) return 0;

  _logfi("Desktop app initialization");

  sen_desktop_app_default_config(&g_cfg);
  if (config) 
    memcpy( &g_cfg,config,sizeof(desktop_app_config_t));
  else
  {
    sen_lua_init();
    if (asset_exists(init_script))
      sen_lua_execFile(init_script);
  }

  glfwSetErrorCallback( error_callback );
  if (! glfwInit() )
    return 0;
  
  glfwWindowHint(GLFW_RESIZABLE,g_cfg.resizable);
  glfwWindowHint(GLFW_RED_BITS, g_cfg.redBits);
  glfwWindowHint(GLFW_GREEN_BITS,g_cfg.greenBits);
  glfwWindowHint(GLFW_BLUE_BITS,g_cfg.blueBits);
  glfwWindowHint(GLFW_ALPHA_BITS,g_cfg.alphaBits);
  glfwWindowHint(GLFW_DEPTH_BITS,g_cfg.depthBits);
  glfwWindowHint(GLFW_STENCIL_BITS,g_cfg.stencilBits);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,g_cfg.gl_ver_major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,g_cfg.gl_ver_minor);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,g_cfg.gl_forward_compat);
  glfwWindowHint(GLFW_DECORATED,g_cfg.decorated);

  if (g_cfg.gl_profile == 1)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
  else if (g_cfg.gl_profile == 2)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  else
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
  glfwWindowHint(GLFW_AUX_BUFFERS, g_cfg.aux_buffers);
  glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
  
  
  mainWindow = glfwCreateWindow(g_cfg.width, g_cfg.height, g_cfg.title, 0, 0);

  if (!mainWindow)
  {
      glfwTerminate();
      return 0;
  }
  glfwSetWindowPos(mainWindow, g_cfg.windowPosX, g_cfg.windowPosY);

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
  
  glfwShowWindow(mainWindow);
  return 1;
  //glfwSwapInterval(1);
}

static void 
loop()
{
  double ldt = 0.0f;
  double sdt = 0.0f;
  double max_fps_t = g_cfg.max_fps > 0 ?  1/(double)(g_cfg.max_fps) : 10000;

#if (SEN_PLATFORM == SEN_PLATFORM_LINUX)
  struct timespec tw;
  struct timespec tr;

#endif

  while (!glfwWindowShouldClose(mainWindow))
  {
    glfwSetTime(0.0f);
    sen_process();
    glfwSwapBuffers(mainWindow);
    glfwPollEvents();
    ldt = glfwGetTime();
    sdt = max_fps_t - ldt;
    if (sdt > 0) {
#if (SEN_PLATFORM == SEN_PLATFORM_WIN32)
      Sleep( (DWORD)(sdt * 1000) );
#elif (SEN_PLATFORM == SEN_PLATFORM_LINUX)
      tw.tv_sec = 0;
      tw.tv_nsec = sdt * 1000000000;
      nanosleep (&tw, &tr);
#endif
    }
  }
}

static void 
destroy()
{
  if (asset_exists(deinit_script))
    sen_lua_execFile(deinit_script);

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

int sen_desktop_app_run(const desktop_app_config_t* config)
{

  if (!init(config)) return EXIT_FAILURE;
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


#if (SEN_PLATFORM == SEN_PLATFORM_WIN32)
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

static int init_gl()
{
  float ver;
#if (SEN_PLATFORM != SEN_PLATFORM_MAC)
  GLenum GlewInitResult;
#endif
  _logfi("Init OpenGL...");
  ver = atof( (const char*)glGetString(GL_VERSION) );
  _logfi("OpenGL v.%.1f", ver);
  _logfi("OpenGL vendor: %s", (const char*)glGetString(GL_VENDOR));


  if ( ver < 2.0 )
  {
    _logfe("OpenGL 2.0 or higher is required, update your drivers.");
    return 0;
  }


#if (SEN_PLATFORM != SEN_PLATFORM_MAC)
   // glewExperimental = GL_TRUE;
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
  float fX = frameW/width*g_cfg.retina_scale*g_cfg.zoom;
  float fY = frameH/height*g_cfg.retina_scale*g_cfg.zoom;

  if (frameW<=0 || frameH<=0) return;
  
  g_cfg.width = width;
  g_cfg.height = height;

  if (fabs(fX - 0.5f) < F_EPSILON &&
      fabs(fY - 0.5f) < F_EPSILON )
  {
      g_cfg.is_retina_monitor = 1;
      g_cfg.retina_scale = g_cfg.is_retina_on ? 1 : 2;
      glfwSetWindowSize(mainWindow,
                        (int)(frameW * 0.5f * g_cfg.retina_scale * g_cfg.zoom) ,
                        (int)(frameH * 0.5f * g_cfg.retina_scale * g_cfg.zoom));
  }
  else if(fabs(fX - 2.0f) < F_EPSILON &&
          fabs(fY - 2.0f) < F_EPSILON)
  {
      g_cfg.is_retina_monitor = 0;
      g_cfg.retina_scale = 1;
      glfwSetWindowSize(mainWindow,
          (int)(frameW * g_cfg.retina_scale * g_cfg.zoom),
          (int)(frameH * g_cfg.retina_scale * g_cfg.zoom));
  }

  //vec2 size = {{(float)width,(float)height}};
  //sen_signal_emit( signal_resize, &size );
}

static void
pos_callback(GLFWwindow *windows, int x, int y)
{
  g_cfg.windowPosX = x;
  g_cfg.windowPosY = y;
}

static
void scroll_callback(GLFWwindow* window, double x, double y)
{
  struct scroll_data {double x; double y;} data = {x,y};
  sen_signal_emit( signal_scroll, &data);
}

void sen_desktop_app_default_config(desktop_app_config_t* cfg)
{
  cfg->resizable = 1;
  cfg->is_retina_monitor = 0;
  cfg->is_retina_on      = 0;
  cfg->retina_scale = 1;
  cfg->zoom = 1.0f;
  cfg->windowPosX = 100;
  cfg->windowPosY = 100;
  cfg->width = 800;
  cfg->height = 600;
  sprintf(cfg->title, "SEN Engine game");

  cfg->redBits = 8;
  cfg->blueBits = 8;
  cfg->greenBits = 8;
  cfg->alphaBits = 8;
  cfg->depthBits = 24;
  cfg->stencilBits = 8;

  cfg->gl_ver_major = 2;
  cfg->gl_ver_minor = 0;
  cfg->gl_profile = 0;
  cfg->gl_forward_compat = 0;
  cfg->aux_buffers = 0;
  cfg->max_fps = 60;
  cfg->decorated = GL_TRUE;
}

desktop_app_config_t* sen_desktop_app_get_config()
{
  return &g_cfg;
}
