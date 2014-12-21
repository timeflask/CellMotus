#include "sen.h"
#include "x_glfw_desktop_app.h"
int main() {

  sen_platform_locate_assets("CellMotus");
  return sen_desktop_app_run(NULL);
}