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

asset_t*
asset_new(const char* path);

void
asset_delete(asset_t* self);

int
asset_exists(const char* path);

void
sen_assets_set_root(const char* path);

const char*
sen_assets_get_root();


#endif

