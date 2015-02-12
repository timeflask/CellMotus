#include "sen.h"
#include "x_glfw_desktop_app.h"
#include <Windows.h>
#include <tchar.h>

//#define SEN_AI_TESTING
#ifdef SEN_AI_TESTING
#include "ai.h"
#endif

#ifdef SEN_DEBUG
#pragma comment( linker, "/subsystem:console" )
int main(int argc, TCHAR *argv[])
{
#ifdef SEN_AI_TESTING
  cm_ai_genLevel();
  //printf("string: %s", cm_ai_genLevel() );
  return 0; 
#else
  sen_platform_locate_assets("CellMotus");
  return sen_desktop_app_run(NULL); 
#endif
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

