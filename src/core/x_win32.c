#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "platform.h"
#include "view.h"
#include "engine.h"
#include "x_glfw_desktop_app.h"
#include "asset.h"
#include <stdio.h>
#include <windows.h>

int
sen_platform_dpi()
{
  return 160;
}
static const char* size_names[] = {
    "small", "normal", "large", "xlarge",
};
const char*
sen_platform_screen_size_name()
{
  const vec4* vp = sen_view_get_viewport();

  float sz = min ( vp->z - vp->x, vp->w - vp->y);

  if (sz >= 720) return size_names[3];
  if (sz >= 480) return size_names[2];
  if (sz >= 320) return size_names[1];

  return size_names[0];
}

//extern int g_exit_flag;
void sen_exit()
{
  sen_desktop_app_exit();
  //g_exit_flag = 1;
}

static const char PLATFORM_NAME[] = "Win32/OpenGL";
const char*
sen_platform_name()
{
  return PLATFORM_NAME;
}


int
sen_platform_locate_assets(const char* alias)
{
  // read registry


  // find in file sys
  char buff2[ MAX_PATH ];
  char res[ MAX_PATH ];
  char* p = NULL;
  char buffer[128];
  GetModuleFileName( NULL, buff2, MAX_PATH );
  p = strrchr ( buff2, '\\' );
  if (p) *(p+1) = '\0';
  

#define _assets_found (asset_exists("assets/scripts/boot.lua"))
#define _assets_check(s) \
  do { sprintf(res, "%s%s", buff2, s); sen_assets_set_root(res); if (_assets_found) { \
    const char* ar = sen_assets_get_root(); \
    _logfi("Found assets at [%s]", ((ar && *ar) ? ar : "current folder"));\
    return 1;\
  }} while(0)\

  _assets_check("");
  _assets_check("../");
  _assets_check("../../");
  if (alias && *alias) {
    sprintf(buffer, "../%s/", alias);
    _assets_check(buffer);
    sprintf(buffer, "../../%s/", alias);
    _assets_check(buffer);
  }

  return 0;
}