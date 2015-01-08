#include "config.h"
#include "asset.h"
#include "macro.h"
#include "utils.h"
#include "logger.h"
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>

#if SEN_PLATFORM==SEN_PLATFORM_LINUX
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Settings"

#if SEN_PLATFORM==SEN_PLATFORM_LINUX
static char g_apath[4096];
void
sen_settings_set_apath(const char* apath)
{
  strcpy(g_apath, apath);
}
#endif

char*
sen_settings_get_str(const char* key, const char* defaultValue)
{
  //UNUSED(key); UNUSED(defaultValue);
#if SEN_PLATFORM==SEN_PLATFORM_LINUX
  char path[4096];
  sprintf(path, "%s/%s", g_apath, key);
  FILE *fp = fopen(path, "rb");
  if (!fp)
    fp = fopen(key, "rb");
#else
  char* path = (char*)key;  
  FILE *fp = fopen(path, "rb");
#endif
  
  long fsize, fr;
  char *string;

  if(fp)
  {
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    string = malloc(fsize + 1);
    fr = fread(string,1, fsize, fp);
    //_logfi("%ul %ul", fr, fsize);
    sen_assert(fr==fsize);
    string[fsize] = 0;
    fclose(fp);
    return string;
  }

  return sen_strdup(defaultValue);
}

void
sen_settings_set_str(const char* key, const char* value)
{
#if SEN_PLATFORM==SEN_PLATFORM_LINUX
  char path[4096];

  struct stat st = {0};

  if (stat(g_apath, &st) == -1) {
      mkdir(g_apath, 0700);
  }
  
  sprintf(path, "%s/%s", g_apath, key);
  FILE *fp = fopen(path, "w");
  if (!fp)
    fp = fopen(key, "w");
#else
  char* path = (char*)key;  
  FILE *fp = fopen(path, "w");
#endif
  if(fp)
  {
    fprintf(fp, "%s", value);
    fflush(fp);
    fclose(fp);
  }
}

