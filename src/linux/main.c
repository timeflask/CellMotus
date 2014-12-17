#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include "sen.h"

#include "GL/freeglut.h"
#include <limits.h>
#include <unistd.h>

//#include <arm_neon.h>
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */
#include <time.h>
typedef struct timespec timespec;
timespec diff(timespec start, timespec end)
{
  timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}

timespec time1, time2;
void start_bench() {
  clock_gettime(CLOCK_MONOTONIC, &time1);
}
void stop_bench() {
  clock_gettime(CLOCK_MONOTONIC, &time2);
  printf("%lld.%.9ld\n", (long long)diff(time1,time2).tv_sec, diff(time1,time2).tv_nsec);
}

#define TEST1
#ifdef TEST1
void test1()
{
#ifdef DEBUG
  _logfi("def DEBUG");
#endif
#ifndef NDEBUG
  _logfi("udef NDEBUG");
#endif
#ifdef NDEBUG
  _logfi("DEF NDEBUG");
#endif
#ifdef _DEBUG
  _logfi("def _DEBUG");
#endif
}
#endif

//#define TEST2
#ifdef TEST2
void test2()
{
  _logfi("-------------- TEST 2 START -----------------------");

  _logfi("-------------- TEST 2 END   -----------------------");
}
#endif

//#define TEST3
#ifdef TEST3
event_t e1,e2;
dispatcher_t d1,d2;
int lp1 (object_t* obj, event_t* e)
{
 // sen_assert(e);
  //_logfi("Got event [%s]", object_name(e));
  //event_post(&d1, &e2);

  return 0;
}
int lp2 (object_t* obj, event_t* e)
{
  //sen_assert(e);
 // _logfi("Got event [%s]", object_name(e));
//  event_post(&d1, &e1);
  return 0;
}

int llp1 (object_t* obj, event_t* e)
{
  //lua_getglobal(sen_lua(), "llp1");
  //lua_pcall(sen_lua(), 0, 0, 0);

  return 0;
}
int llp2 (object_t* obj, event_t* e)
{
  //lua_getglobal(sen_lua(), "llp2");
  //lua_pcall(sen_lua(), 0, 0, 0);
  return 0;
}


void test3()
{
  _logfi("-------------- TEST 3 START -----------------------");
  /*event_init(&e1); event_init(&e2);
  object_set_name(&e1, "event_1");
  object_set_name(&e2, "event_2");
  dispatcher_init(&d1); dispatcher_init(&d2);
  //object_set_name(&d1, "disp_1");
  //object_set_name(&d2, "disp_2");
  event_send(&d1, &e1);
  dispatcher_listen(&d1, "event_1", &d2, &llp1);
  dispatcher_listen(&d1, "event_2", &d2, &llp2);

  sen_run_lua("test.lua");


  event_send(&d1, &e2);
  event_send(&d1, &e1);
*/
  sen_run_lua("lua/test.lua");

/*
  int i;
  start_bench();
  for (i = 0; i< 1000000; i++) {
    event_send(&d1, &e1);
  }
  stop_bench();
*/


  //event_clean(&e1); event_clean(&e2);
  //dispatcher_clean(&d1); dispatcher_clean(&d2);

  //lua_run("test.lua");

  _logfi("-------------- TEST 3 END   -----------------------");
}
#endif


#define TEST
#ifdef TEST
void test()
{
  _logfi("-------------- TEST  START -----------------------");
  _logfi("-------------- TEST  END   -----------------------");
}
#endif

//#define TEST4
#ifdef TEST4
void test4()
{
  _logfi("-------------- TEST 4 START -----------------------");


  if (lua_pcall(L, 0, 0, 0))
    bail(L, "lua_pcall() failed");


  int temp;
  int i;
  start_bench();
  for (i = 0; i< 442000000; i++)
    temp+=temp;
  stop_bench();

  //char buff[20];
  //strftime(buff, 20, "%s", diff(time1,time2));
  //_logfi("sec = %s",buff);
  //_logfi("sec = %lld  nsec=%l", (long long) diff(time1,time2).tv_sec, diff(time1,time2).tv_nsec);
  //cout<<diff(time1,time2).tv_sec<<":"<<diff(time1,time2).tv_nsec<<endl;




  _logfi("-------------- TEST 4 END   -----------------------");
}
#endif


//#define TEST5
#ifdef TEST5
void test5()
{
  _logfi("-------------- TEST5  START -----------------------");
  sen_timer_t t;
  timer_restart(&t);

  int temp;
  int i;
  timer_restart(&t);
  for (i = 0; i< 40333333; i++)
    temp+=temp;
  _logfi("elapsed %f", timer_update(&t));
  _logfi("elapsed %f", timer_update(&t));

  for (i = 0; i< 140333333; i++)
    sen_process();

  _logfi("-------------- TEST5  END   -----------------------");
}
#endif

//#define TEST6
#ifdef TEST6

typedef struct {
    float x, y, z;    // position
    float s, t;       // texture
    float r, g, b, a; // color
} vertex_t;

void add_text( vertex_buffer_t * buffer, texture_font_t * font,
               wchar_t * text, vec4 * color, vec2 * pen )
{
    size_t i;
    float r = color->red, g = color->green, b = color->blue, a = color->alpha;
    for( i=0; i<wcslen(text); ++i )
    {
        texture_glyph_t *glyph = texture_font_get_glyph( font, text[i] );
        if( glyph != NULL )
        {
            int kerning = 0;
            if( i > 0)
            {
                kerning = texture_glyph_get_kerning( glyph, text[i-1] );
            }
            pen->x += kerning;
            int x0  = (int)( pen->x + glyph->offset_x );
            int y0  = (int)( pen->y + glyph->offset_y );
            int x1  = (int)( x0 + glyph->width );
            int y1  = (int)( y0 - glyph->height );
            //_logfi( "x0 = %d, y0 = %d, x1=%d, y1 = %d", x0, y0,x1,y1);
            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;
            GLushort indices[6] = {0,1,2, 0,2,3};
            vertex_t vertices[4] = { { x0,y0,0,  s0,t0,  r,g,b,a },
                                     { x0,y1,0,  s0,t1,  r,g,b,a },
                                     { x1,y1,0,  s1,t1,  r,g,b,a },
                                     { x1,y0,0,  s1,t0,  r,g,b,a } };
            vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );
            pen->x += glyph->advance_x;
        }
    }
}
vertex_buffer_t *buffer, *quad;
mat4   model, view, projection;
GLuint shader;
GLuint shader2;
static const float rect[] = {-200.0f, -200.0f, 0.0f, 0.0f,
                         -200.0f,  200.0f, 0.0f, 1.0f,
                          200.0f, -200.0f, 1.0f, 0.0f,
                          200.0f,  200.0f, 1.0f, 1.0f};
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
    mat4_set_translation(&view, width/2, height/2, 0);
}
void idle( void )
{
    glutPostRedisplay();
}

void renderFunction()
{
  sen_process();
  glClearColor( 0.2,0.2, 0.2, 1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glEnable( GL_BLEND );
  glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
/*
  glUseProgram( shader );
  {
      glUniform1i( glGetUniformLocation( shader, "texture" ),
                   0 );
      glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),
                          1, 0, model.data);
      glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),
                          1, 0, view.data);
      glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ),
                          1, 0, projection.data);
      vertex_buffer_render( buffer, GL_TRIANGLES );
  }  glUseProgram( shader );
*/

  mat4_translate(&model, 0.001f, 0,0);
  int i;
  for (i=0; i <10; ++i) {
  glUseProgram( shader2 );
  {

      glUniform1i( glGetUniformLocation( shader, "texture" ),
                   0 );
      glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),
                          1, 0, model.data);
      glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),
                          1, 0, view.data);
      glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ),
                          1, 0, projection.data);
      vertex_buffer_render( quad, GL_TRIANGLE_STRIP );
  }
  }
  glutSwapBuffers( );

}
void test6(int argc, char** argv)
{
  _logfi("-------------- TEST6  START -----------------------");
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500,500);
  glutInitWindowPosition(100,100);
  glutCreateWindow("OpenGL - First window demo");
  glutIdleFunc( idle );
  glutReshapeFunc( reshape );
  srand(time(0));
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf( stderr, "Error: %s\n", glewGetErrorString(err) );
      exit( EXIT_FAILURE );
  }
  fprintf( stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION) );

  texture_font_t *font = 0;
  texture_atlas_t *atlas = texture_atlas_new( 512, 512, 1 );
  const char * filename = "mecha.ttf";
  wchar_t *text = L"A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";
  buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
  font = texture_font_new( atlas, filename, 18 );
  texture_font_load_glyphs( font, text );
  vec2 pen = {{5,5}};
  vec4 black = {{0,0,0,1}};
  add_text( buffer, font, text, &black, &pen );
  vertex_buffer_print(buffer);
  _logfi( "Font filename              : %s\n", filename );
  _logfi( "Font size                  : %.1f\n", 18.0);
  _logfi( "Number of glyphs           : %ld\n", wcslen(text) );
  _logfi( "Texture size               : %ldx%ldx%ld\n",
           atlas->width, atlas->height, atlas->depth );
  _logfi( "Texture occupancy          : %.2f%%\n",
          100.0*atlas->used/(float)(atlas->width*atlas->height) );
  shader = shader_load("v3f-t2f-c4f.vert",
                       "v3f-t2f-c4f.frag");
  mat4_set_identity( &projection );
  mat4_set_identity( &model );
  mat4_set_identity( &view );

  //ivec4 reg = texture_atlas_get_region(atlas, img->width, img->height);
  //texture_atlas_set_region(atlas, reg.x, reg.y, img->width, img->height, img->raw_data, img->width*img->stride);
  _logfi( "Texture size               : %ldx%ldx%ld\n",
           atlas->width, atlas->height, atlas->depth );
  _logfi( "Texture occupancy          : %.2f%%\n",
          100.0*atlas->used/(float)(atlas->width*atlas->height) );


  quad = vertex_buffer_new("vertex:2f,tex_coord:2f");
  vertex_buffer_push_back_vertices(quad, rect, 4);
  shader2 = shader_load("1.vert",
                       "1.frag");
  image_t* img = image_new("test.png");
  _logfi( "Image: %s\n w,h = %d, %d\nchannels= %d\ncbp=%d\ndepth=%d\n",
     img->filename,
     img->width, img->height,
     img->channels,
     img->channel_bit_depth,
     img->channels*img->channel_bit_depth/8 );
  texture_atlas_t *atlas2 = texture_atlas_new( 512, 512, 1);
  //img->

  ivec4 reg = texture_atlas_get_region(atlas2, img->width, img->height);
  texture_atlas_set_region(atlas2, reg.x, reg.y, img->width, img->height, img->raw_data, img->stride);
  reg = texture_atlas_get_region(atlas2, img->width, img->height);
  texture_atlas_set_region(atlas2, reg.x, reg.y, img->width, img->height, img->raw_data, img->stride);
  reg = texture_atlas_get_region(atlas2, img->width, img->height);
  texture_atlas_set_region(atlas2, reg.x, reg.y, img->width, img->height, img->raw_data, img->stride);
  reg = texture_atlas_get_region(atlas2, img->width, img->height);
  texture_atlas_set_region(atlas2, reg.x, reg.y, img->width, img->height, img->raw_data, img->stride);
  reg = texture_atlas_get_region(atlas2, img->width, img->height);
  texture_atlas_set_region(atlas2, reg.x, reg.y, img->width, img->height, img->raw_data, img->stride);
  reg = texture_atlas_get_region(atlas2, img->width, img->height);
  texture_atlas_set_region(atlas2, reg.x, reg.y, img->width, img->height, img->raw_data, img->stride);
  reg = texture_atlas_get_region(atlas2, img->width, img->height);
  texture_atlas_set_region(atlas2, reg.x, reg.y, img->width, img->height, img->raw_data, img->stride);

  _logfi( "Texture size               : %ldx%ldx%ld\n",
           atlas2->width, atlas2->height, atlas2->depth );
  _logfi( "Texture occupancy          : %.2f%%\n",
          100.0*atlas2->used/(float)(atlas2->width*atlas2->height) );
  texture_atlas_upload( atlas2 );
  image_delete(img);


  texture_font_delete( font );
  //glBindTexture( GL_TEXTURE_2D, atlas->id );
  glBindTexture( GL_TEXTURE_2D, atlas2->id );

  glutDisplayFunc(renderFunction);
  glutMainLoop();
  _logfi("-------------- TEST6  END   -----------------------");
}
#endif

#define TEST7
#ifdef TEST7
static const void* signal_resize   = NULL;


void reshape(int width, int height)
{
  vec2 size = {{(float)width,(float)height}};
  sen_signal_emit( signal_resize, &size );
}

int g_exit_flag = 0;

void renderFunction()
{
  sen_process();
/*
  vec2 start = {{ -1, 0}};
  vec2 end = {{ 1, 0}};
  sen_shapes_line(&start, &end);
  vec2 start2 = {{ 0, 1}};
  vec2 end2 = {{ 0, -1}};
  sen_shapes_line(&start2, &end2);
*/
  glutSwapBuffers( );
  usleep(8000);
}
void idle( void )
{
  if (g_exit_flag)
    glutLeaveMainLoop();
  else
    glutPostRedisplay();
}
static const void* signal_reload       = NULL;
static const void* signal_keyDown  = NULL;
void
keyboard( unsigned char key, int x, int y )
{
  UNUSED(x);UNUSED(y);
    if ( key == 27 )
    {
        glutLeaveMainLoop();
    }
    else if (key == 32) {
      sen_signal_emit( signal_reload, NULL );
    }
    int kcode = key;
    sen_signal_emit( signal_keyDown, & kcode );
}

static const void* signal_touchesBegin = NULL;
static const void* signal_touchesEnd   = NULL;
static const void* signal_touchesMove  = NULL;

static const int WIDTH = 640;
static const int HEIGHT =480;

void Init() {
//  sen_assets_set_root("../");
  sen_init(WIDTH,HEIGHT);
  signal_resize   = sen_signal_get_name("resize",          "platform");
  signal_reload   = sen_signal_get_name("reload",          "platform");
  signal_touchesBegin   = sen_signal_get_name("touchesBegin",    "platform");
  signal_touchesEnd     = sen_signal_get_name("touchesEnd",      "platform");
  signal_touchesMove    = sen_signal_get_name("touchesMove",     "platform");
  signal_keyDown= sen_signal_get_name("keyDown",     "platform");

  gl_check_error();

}
void Deinit() {

  sen_destroy();
}

static int buttons[32];

void mouse_func(int button, int state, int x, int y) {

  float fx = (float)x;
  float fy = (float)y;
  touch_data_t td = {1, &button, &fx, &fy};
  if (GLUT_DOWN == state) {
    buttons[button] = 1;
    sen_signal_emit( signal_touchesBegin, &td );
  }
  else if (GLUT_UP == state){
    buttons[button] = 0;
    sen_signal_emit( signal_touchesEnd, &td );
  }
}

void mouse_move_func(int x, int y){
  int i;
  for (i = 0; i < 32;++i) {
    if (!buttons[i]) continue;
    float fx = (float)x;
    float fy = (float)y;
    touch_data_t td = {1, &i, &fx, &fy};
    sen_signal_emit( signal_touchesMove, &td );
  }
}

void test7(int argc, char** argv)
{
  memset(buttons,0,sizeof(buttons));
  srand(time(0));  _logfi("-------------- TEST7  START -----------------------");
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WIDTH,HEIGHT);
  glutInitWindowPosition(100,100);
  glutCreateWindow("OpenGL - First window demo");
  glutReshapeFunc( reshape );
  glutIdleFunc( idle );
  glutMouseFunc(mouse_func);
  glutMotionFunc(mouse_move_func);
  glutCloseFunc( Deinit );
  glutKeyboardFunc( keyboard );

  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf( stderr, "Error: %s\n", glewGetErrorString(err) );
      exit( EXIT_FAILURE );
  }

  fprintf( stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION) );
  Init();
  glutDisplayFunc(renderFunction);
  glutMainLoop();
  //Deinit();
  _logfi("-------------- TEST7  END   -----------------------");
}
#endif

int main(int argc, char** argv)
{


  //lua_run("sen.lua");


#ifdef TEST1
  test1();
#endif
#ifdef TEST2
  test2();
#endif
#ifdef TEST3
  test3();
#endif
#ifdef TEST4
  test4();
#endif
#ifdef TEST5
  test5();
#endif

#ifdef TEST6
  test6(argc, argv);
#endif
#ifdef TEST7
  test7(argc, argv);
#endif


  //sen_destroy();
  return 0;
}


