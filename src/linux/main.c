#include "sen.h"
#include "x_glfw_desktop_app.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>

const char *homedir;
char buff[4096];

int main() {
  sen_platform_locate_assets("CellMotus");
  
  if ((homedir = getenv("HOME")) == NULL) {
    homedir = getpwuid(getuid())->pw_dir;
  }

  sprintf(buff, "%s/%s", homedir, ".CellMotus");
  sen_settings_set_apath(buff);
  return sen_desktop_app_run(NULL);
}
