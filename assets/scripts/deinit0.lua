local ffi = require "ffi"
local C   = ffi.C

local inspect = require 'sen.inspect'

ffi.cdef[[
void
sen_settings_set_str(const char* name, const char* str);

typedef struct desktop_app_config_t_2 {
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
} desktop_app_config_t_2;

desktop_app_config_t_2* sen_desktop_app_get_config();
]]

local out = {}
local p =  C.sen_desktop_app_get_config() 

out.resizable=p.resizable
out.width=p.width
out.height=p.height
out.is_retina_monitor=p.is_retina_monitor
out.is_retina_on=p.is_retina_on
out.retina_scale=p.retina_scale
out.zoom=p.zoom
out.windowPosX=p.windowPosX
out.windowPosY=p.windowPosY
out.title=ffi.string(p.title)

out.redBits=p.redBits
out.greenBits=p.greenBits
out.blueBits=p.blueBits
out.alphaBits=p.alphaBits
out.depthBits=p.depthBits
out.stencilBits=p.stencilBits

out.gl_ver_major=p.gl_ver_major
out.gl_ver_minor=p.gl_ver_minor
out.gl_profile=p.gl_profile
out.gl_forward_compat=p.gl_forward_compat

C.sen_settings_set_str('.desktop', inspect(out))
--print (inspect(out))
