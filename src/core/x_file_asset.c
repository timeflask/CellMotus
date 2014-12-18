#include "config.h"
#include "asset.h"
#include "macro.h"
#include "utils.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>


#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Asset"
 char* g_assetsRoot = NULL;


void
asset_init(asset_t* self, const char* path)
{
  FILE* stream;
  size_t rsize = 0;
  sen_assert(self);
  sen_assert(path);

  if (g_assetsRoot) {
    int asz = strlen(g_assetsRoot);
    int sz = asz + strlen(path) + 1;
    self->path = (char*) malloc( sz);
    strcpy(self->path, g_assetsRoot);
    self->path+=asz ;
    strcpy(self->path, path);
    self->path-=asz;
  }
  else
    self->path = sen_strdup(path);
  _logfi("Read asset from %s", self->path);

  stream = fopen(self->path, "rb");
  sen_assert (stream != NULL);

  fseek(stream, 0, SEEK_END);
  self->size = ftell(stream);
  fseek(stream, 0, SEEK_SET);

  self->handle = malloc(self->size);
  rsize = fread(self->handle, self->size, 1, stream);

  sen_assert(ferror(stream) == 0);
  fclose(stream);

  self->data = self->handle;
  _logfi("+asset[%p]: %s, data=%p, size=%d [%d]", self->handle, self->path, self->data, self->size, rsize);
}

asset_t*
asset_new(const char* path)
{
  struct_malloc(asset_t, self);
  sen_assert(path);
  memset(self, 0, sizeof(asset_t));
  asset_init(self, path);
  return self;
}

void
asset_delete(asset_t* self)
{
  sen_assert(self);
  if(self->path) {
    _logfi("-asset[%p]", self->handle);
    free(self->path);
  }
  if (self->data != NULL) {
    free((void*)self->handle);
  }
  free(self);
}


int
asset_exists(const char* _path)
{
  char* path = (char*)_path;
  int bFound = 0;
  FILE *fp;

  if ( _path == NULL || *_path == '\0') return 0;
  fp = fopen(path, "r");
  if(fp)
  {
    bFound = 1;
    fclose(fp);
  }
  return bFound;
}

void
sen_assets_set_root(const char* path)
{
  if (g_assetsRoot ) free(g_assetsRoot);
  if (path == NULL)
    g_assetsRoot = NULL;
  else
    g_assetsRoot  = sen_strdup( path);
}

static const char empty_path[] = "";

const char*
sen_assets_get_root()
{
  return g_assetsRoot ? g_assetsRoot : empty_path;
}
