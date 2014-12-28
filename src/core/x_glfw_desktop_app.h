#ifndef __X_glfw_desktop_app_H_
#define __X_glfw_desktop_app_H_
#include "config.h"

typedef struct desktop_app_config_t {
  int resizable;
  int width;
  int height;
  int is_retina_monitor;
  int is_retina_on;
  int retina_scale;
  float zoom;
  int windowPosX;
  int windowPosY;
  char title[64];

  int redBits;
  int greenBits;
  int blueBits;
  int alphaBits;
  int depthBits;
  int stencilBits;

  int gl_ver_major;
  int gl_ver_minor;
  int gl_profile;
  int gl_forward_compat;

  int aux_buffers;

  int max_fps;
  int decorated;
} desktop_app_config_t;

SEN_DECL
void sen_desktop_app_default_config(desktop_app_config_t* config);

SEN_DECL
desktop_app_config_t* sen_desktop_app_get_config();

int sen_desktop_app_run(const desktop_app_config_t* config);
void sen_desktop_app_exit();


#endif
