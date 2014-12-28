local ffi = require "ffi"
local C   = ffi.C

ffi.cdef[[
char* sen_settings_get_str(const char* name, const char* default_str);
void __free(void* p);    

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

desktop_app_config_t* sen_desktop_app_get_config();
]]

local  cfg = loadstring( "return  "..
             ffi.string( 
               ffi.gc(C.sen_settings_get_str('.desktop', '{}'), C.__free) ) 
           )()

local out = C.sen_desktop_app_get_config()

for k,v in pairs(cfg) do
  if out[k] then out[k]=v end
end  


           