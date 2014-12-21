#include "sen.h"
#include "x_glfw_desktop_app.h"
#include <Windows.h>
#include <tchar.h>


#ifdef SEN_DEBUG
#pragma comment( linker, "/subsystem:console" )
int main(int argc, TCHAR *argv[])
{
  sen_platform_locate_assets("CellMotus");
  return sen_desktop_app_run(NULL); 
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
  sen_platform_locate_assets("CellMotus");
  sen_desktop_app_run(NULL); 
  return 0;
}
#endif

