#include "config.h"
#include "asset.h"
#include "macro.h"
#include "utils.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>

#import "Foundation/Foundation.h"
 char* g_assetsRoot = NULL;

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Asset"


const char* sen_assets_get_full_path(const char* path) {
  sen_assert(path);
  char* assets_sub = strstr(path, "assets/" );
  if (assets_sub == path)
    path += strlen("assets/");
     
  NSMutableString* relative_path = 
      [[NSMutableString alloc] initWithString:@"/assets/"];
  
  [relative_path appendString:
    [[NSString alloc] initWithCString:path encoding:NSASCIIStringEncoding]];
  return
    [[[NSBundle mainBundle] pathForResource:relative_path ofType:nil] 
     cStringUsingEncoding:NSASCIIStringEncoding ];
}

void
asset_init(asset_t* self, const char* path)
{
  FILE* stream;
  size_t rsize = 0;
  const char* fp;
  sen_assert(self);
  sen_assert(path);
  fp = sen_assets_get_full_path(path);
  sen_assert(fp);
  self->path = sen_strdup(fp);

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
  return sen_assets_get_full_path(_path) != NULL;
}

void
sen_assets_set_root(const char* path)
{

}

static const char empty_path[] = "";

const char*
sen_assets_get_root()
{
  return empty_path;
}
