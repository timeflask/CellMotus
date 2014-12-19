#include "sen.h"
#include "x_glfw_desktop_app.h"
#include <Windows.h>
#include <tchar.h>

static int runner()
{
  sen_assets_set_root("../");
  return sen_desktop_app_run(); 
}

#ifdef SEN_DEBUG
#pragma comment( linker, "/subsystem:console" )
int main(int argc, TCHAR *argv[])
{
  return runner();
}
#else
#pragma comment( linker, "/subsystem:windows" )
int CALLBACK WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{
  runner();
  return 0;
}
#endif

