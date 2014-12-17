#ifndef __settings_H_
#define __settings_H_
#include "config.h"


char*
sen_settings_get_str(const char* name, const char* default_str);

void
sen_settings_set_str(const char* name, const char* str);

#endif
