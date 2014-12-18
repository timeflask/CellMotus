#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "platform.h"
#include "view.h"
#include "engine.h"
#include "x_glfw_desktop_app.h"
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

static const char PLATFORM_NAME[] = "Posix/OpenGL GLUT";
const char*
sen_platform_name()
{
  return PLATFORM_NAME;
}

