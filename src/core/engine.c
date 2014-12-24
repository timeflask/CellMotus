#include "engine.h"
#include "logger.h"
#include "macro.h"
#include "asset.h"
#include "hrtimer.h"
#include "luas.h"
#include "opengl.h"
#include "texture-manager.h"
#include "shader.h"
#include "view.h"
#include "camera.h"
#include "signals.h"
#include "scene.h"
#include "render.h"
#include "scheduler.h"
#include "node.h"
#include "utils.h"
#include "input.h"
#include "shapes.h"
#include "audio.h"
#include "image.h"
//#include <sys/time.h>
//#include <time.h>


//static sen_timer_t      g_timer;
static float            g_fps = 0.0f;
static camera_t*        g_camera  = NULL;
static scheduler_t*     g_scheduler = NULL;
extern scene_t*         g_scene;
//static struct timeval * g_last_dt = NULL;
static double           g_prev_dt = 0.0f;
static double           fps_accumulator = 0.0;
static int              frames = 0;
static int              g_drop_dt = 1;
static double           dt = 0;
static const float      FPS_INTERVAL = 1.5f;

extern char* g_assetsRoot;

float sen_fps()
{
  return g_fps;
}

camera_t* sen_camera()
{
  return g_camera;
}

scheduler_t* sen_scheduler()
{
  return g_scheduler;
}



static int g_status = SEN_STATUS_DEAD;

int sen_status()
{
  return g_status;
}


void sen_reload()
{
  sen_textures_reload();
  sen_shaders_reload();
  sen_shapes_reload();
  sen_scenes_reload();
  sen_render_reload();
}

static int on_reload(object_t* _null, void* _null1, object_t* _null2, const char* sig)
{
  UNUSED(_null); UNUSED(_null1); UNUSED(_null2); UNUSED(sig);
  sen_reload();
  return 0;
}


static int on_bg(object_t* _null, void* _null1, object_t* _null2, const char* sig)
{
  UNUSED(_null); UNUSED(_null1); UNUSED(_null2); UNUSED(sig);
  _logfi("Entring background - stop animation");
  g_status |= SEN_STATUS_STOPPED;
  return 0;
}

static int on_fg(object_t* _null, void* _null1, object_t* _null2, const char* sig)
{
  UNUSED(_null); UNUSED(_null1); UNUSED(_null2); UNUSED(sig);
  _logfi("Entering foreground - start animation");
  g_drop_dt = 1;
  g_status &= ~SEN_STATUS_STOPPED;
  return 0;
}

//#define TEST

#ifdef TEST
static vertex_buffer_t* quad;
static const float rect[] = {-200.0f, -200.0f, 0, 1, 0.0f, 0.0f,
                         -200.0f,  200.0f,0, 1,  0.0f, 1.0f,
                          200.0f, -200.0f,0, 1,  1.0f, 0.0f,
                          200.0f,  200.0f,0, 1,  1.0f, 1.0f};
static const shader_t* prog = NULL;
unsigned int tex_id;
void test_init()
{
  image_t* img = image_new("assets/images/cell128.png");
  glGenTextures( 1, &tex_id );
  sen_shaders_load("test", "vs_pos4f_tex2f", "fs_tex2f");
  prog = sen_shaders_get("test");
  glBindTexture( GL_TEXTURE_2D, tex_id );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, img->raw_data);

  image_delete(img);

  quad = vertex_buffer_new("a_pos:4f,a_tex_coords:2f");
  vertex_buffer_push_back_vertices(quad, rect, 4);
}
void test_destroy()
{
  vertex_buffer_delete(quad);
}
void test_draw()
{
  camera_t* cam = sen_camera();
  glBindTexture( GL_TEXTURE_2D, tex_id );
  sen_shader_use(prog);
  sen_uniform_1iN(prog, "u_tex0", 0);
  sen_uniform_m4fN(prog, "u_mvp", cam->proj.data  );

  vertex_buffer_render( quad, GL_TRIANGLE_STRIP );
}
#endif

void
sen_init(float w, float h)
{
  if (g_status & SEN_STATUS_INITIALIZED) return;

  _logfi("Init soup engine");
  //timer_restart(&g_timer);

  sen_signals_manager_init();
  sen_input_init();
  sen_views_init();
  sen_view_set_screen(0,0,w,h);

  sen_texture_manager_init(512);
  sen_shaders_manager_init();

  sen_render_init();
  sen_lua_init();

  sen_audio_init();

  g_camera = sen_camera_new("mainCamera");
  g_scheduler = sen_scheduler_new("mainScheduler");

  //g_last_dt = malloc(sizeof(struct timeval));

  sen_signal_connect_name("platform", "reload", &on_reload, "engine");
  sen_signal_connect_name("platform", "enterBackground", &on_bg, "engine");
  sen_signal_connect_name("platform", "enterForeground", &on_fg, "engine");

  sen_shapes_init();

#ifdef TEST
  test_init();
#endif

  if (asset_exists("assets/scripts/boot.lua"))
  {
    _logfi("LUA boot found");
    sen_lua_execFile("assets/scripts/boot.lua");
    _logfi("LUA boot finished");
  }
  else
  {
    _logfw("LUA BOOT NOT FOUND, Enjoy ANSI C");
  }


  g_status |= SEN_STATUS_INITIALIZED;
}

void sen_destroy() {
  if (g_status == SEN_STATUS_DEAD) return;

#ifdef TEST
  test_destroy();
#endif

  sen_input_destroy();
  sen_lua_close();
  sen_shapes_destroy();
  sen_audio_destroy();
  sen_scheduler_delete(g_scheduler);
  sen_camera_destroy(g_camera);
  sen_shaders_manager_destroy();
  sen_texture_manager_destroy();
  sen_views_destroy();
  sen_render_destroy();
  sen_signals_manager_destroy();

  if (g_assetsRoot) free(g_assetsRoot);

  //free(g_last_dt);
  g_status = SEN_STATUS_DEAD;
}

static size_t g_renderedItems = 0;
static size_t g_updateEntries = 0;

static void show_stats()
{

  ++frames;
  fps_accumulator += dt;
  if (fps_accumulator > FPS_INTERVAL ) {
    g_fps = (float) (frames / fps_accumulator);
    frames = 0;
    fps_accumulator = 0;

    _logfi("FPS: %.1f, "
        "updated: %u, "
        "rendered: %u "
        "nodes: %u",
        g_fps, g_updateEntries, g_renderedItems, sen_nodes_total());


  }
}

static scene_t* prev = NULL;
static int render_clear, render_flush;
void  sen_process_update()
{
  
  double now;
  node_t* cam;
 
  render_clear = 0;
  render_flush = 0;
  if ((g_status&SEN_STATUS_STOPPED) || g_status == SEN_STATUS_DEAD) return;
  render_clear = 1;

  now = sen_timer_now();
  cam = (node_t*)sen_camera();

  sen_audio_update();
  
  if (g_drop_dt) {
    g_drop_dt = 0;
    dt = 0;
  }
  else
  {
    dt = max(0, now - g_prev_dt);
  }
  
#ifdef SEN_DEBUG
  if (dt > 0.25)
    dt = 1/60.0f;
#else
  if (dt > 0.25)
    dt = 0.25;
#endif
  g_prev_dt = now;
  if (dt < F_EPSILON) return;
  
  if ((cam->updated & SEN_NODE_UPDATE_MODEL) || ( prev != g_scene)) {
    if ( prev != g_scene && prev)
      update_scene_bbox((object_t*)prev, NULL);
    else
      update_scene_bbox((object_t*)g_scene, NULL);
  }
  prev = g_scene;
  g_updateEntries = sen_scheduler_update(g_scheduler, dt);
  render_flush = 1;
  
}

void  sen_process_draw()
{
  if (!render_clear)
    return;
  
  sen_render_clear();
  
  if (!render_flush)
    return;
  
#ifdef TEST
  test_draw();
#else
  if (prev!=g_scene) return;
  
  sen_render_node((node_t*)g_scene);
  
  g_renderedItems= sen_render_flush(1);
#endif
  
  //sen_render_clear();
  //sen_shapes_line(&start, &end);
  //sen_shapes_line(&start2, &end2);
  
  show_stats();
}

void sen_process()
{
  sen_process_update();
  sen_process_draw();
}


