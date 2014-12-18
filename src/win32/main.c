#include "sen.h"
#include "x_glfw_desktop_app.h"
int main() {  
  sen_assets_set_root("../");
  return sen_desktop_app_run(); 
}