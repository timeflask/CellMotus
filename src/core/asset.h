#ifndef __asset_H_
#define __asset_H_
#include <stdlib.h>
#include <stddef.h>

typedef struct {
  const void*  data;
  size_t       size;
  char*        path;

  void*        handle;
} asset_t;

SEN_DECL asset_t*
asset_new(const char* path);

SEN_DECL void
asset_delete(asset_t* self);

SEN_DECL int
asset_exists(const char* path);

SEN_DECL void
sen_assets_set_root(const char* path);

SEN_DECL const char*
sen_assets_get_root();

#if SEN_PLATFORM == SEN_PLATFORM_IOS
const char* sen_assets_get_full_path(const char* path);
#endif

#if SEN_PLATFORM == SEN_PLATFORM_MAC
char* sen_assets_get_full_path(const char* path);
#endif

#endif

