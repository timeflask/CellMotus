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
  //UNUSED(key); UNUSED(defaultValue);

  char* path = (char*)key;
  FILE *fp = fopen(path, "rb");
  long fsize;
  char *string;

  if(fp)
  {
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    string = malloc(fsize + 1);
    fread(string, fsize, 1, fp);
    string[fsize] = 0;
    fclose(fp);
    return string;
  }

  return sen_strdup(defaultValue);
}

void
sen_settings_set_str(const char* key, const char* value)
{
  char* path = (char*)key;
  FILE *fp = fopen(path, "w");
  if(fp)
  {
    fprintf(fp, "%s", value);
    fflush(fp);
    fclose(fp);
  }
}

