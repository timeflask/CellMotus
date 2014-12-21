#ifndef __platform_H_
#define __platform_H_
#include "config.h"

SEN_DECL int
sen_platform_dpi();

SEN_DECL const char*
sen_platform_screen_size_name();

SEN_DECL const char*
sen_platform_name();


#if (SEN_PLATFORM == SEN_PLATFORM_LINUX) 
  SEN_DECL int
  sen_platform_locate_assets(const char* alias);
#endif

#if (SEN_PLATFORM == SEN_PLATFORM_WIN32) 
  SEN_DECL int
  sen_platform_locate_assets(const char* alias);
#endif

#if (SEN_PLATFORM == SEN_PLATFORM_MAC) 
  SEN_DECL int
  sen_platform_locate_assets(const char* alias);
#endif

#endif