#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "platform.h"
#include "view.h"
#include "engine.h"
#include "x_glfw_desktop_app.h"
#include "asset.h"
#include <stdio.h>

#include "CoreFoundation/CoreFoundation.h"

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

void sen_exit()
{
  sen_desktop_app_exit();
}

static const char PLATFORM_NAME[] = "MAC/OpenGL GLFW";
const char*
sen_platform_name()
{
  return PLATFORM_NAME;
}

int
sen_platform_locate_assets(const char* alias)
{

    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    char buffer[PATH_MAX];
    char buffer2[PATH_MAX];

    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        return 0;
    }
    CFRelease(resourcesURL);

  // find in file sys

#define _assets_found (asset_exists("assets/scripts/boot.lua"))
#define _assets_check(s) \
  do {  sprintf(buffer2, "%s/%s", path,s); sen_assets_set_root(buffer2); if (_assets_found) { \
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

