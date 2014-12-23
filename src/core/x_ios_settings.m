#include "config.h"
#include "asset.h"
#include "macro.h"
#include "utils.h"
#include "logger.h"
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Settings"

char*
sen_settings_get_str(const char* key, const char* defaultValue)
{
  return sen_strdup(defaultValue);
}

void
sen_settings_set_str(const char* key, const char* value)
{
}
