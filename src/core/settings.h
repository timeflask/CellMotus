#ifndef __settings_H_
#define __settings_H_
#include "config.h"


SEN_DECL char*
sen_settings_get_str(const char* name, const char* default_str);

SEN_DECL void
sen_settings_set_str(const char* name, const char* str);

#if SEN_PLATFORM==SEN_PLATFORM_LINUX
void
sen_settings_set_apath(const char* apath);
#endif

#endif
