#include "config.h"
#include "asset.h"
#include "macro.h"
#include "utils.h"
#include "logger.h"
#include <string.h>
#include <android/asset_manager_jni.h>
#include <stdio.h>
AAssetManager* g_AssetManager = NULL;
 char* g_assetsRoot = NULL;

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Asset"

void
asset_init(asset_t* self, const char* path)
{
  sen_assert(g_AssetManager);
  sen_assert(self);
  sen_assert(path);
  _logfi("Read asset from %s", path);

  char* assets_sub = strstr(path, "assets/" );
  if (assets_sub == path)
    path += strlen("assets/");

  self->path = sen_strdup(path);
  self->handle = AAssetManager_open(g_AssetManager, path, AASSET_MODE_STREAMING);
  sen_assert(self->handle);
  self->size = AAsset_getLength(self->handle);
  sen_assert(self->size);
  self->data = AAsset_getBuffer(self->handle);
  sen_assert(self->data);
  _logfi("+asset[%p]: %s, data=%p, size=%d", self->handle, self->path, self->data, self->size);
}

asset_t*
asset_new(const char* path)
{
  sen_assert(path);
  struct_malloc(asset_t, self);
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
  if (self->data != NULL)
      AAsset_close(self->handle);
  free(self);
}

int
asset_exists(const char* _path)
{
  if ( _path == NULL || *_path == '\0') return 0;
  int bFound = 0;
  char* path = (char*)_path;

  if (path[0] != '/')
  {
    char* assets_sub = strstr(path, "assets/" );
    if (assets_sub == path)
      path += strlen("assets/");
    AAsset* A =  AAssetManager_open(g_AssetManager, path, AASSET_MODE_UNKNOWN);
    if (A) {
      bFound = 1;
      AAsset_close(A);
    }
  }
  else
  {
    FILE *fp = fopen(path, "r");
    if(fp)
    {
        bFound = 1;
        fclose(fp);
    }
  }
  return bFound;

}

void
sen_assets_set_root(const char* path)
{
  UNUSED(path);
}

static const char empty_path[] = "";

const char*
sen_assets_get_root()
{
  return g_assetsRoot ? g_assetsRoot : empty_path;
}
