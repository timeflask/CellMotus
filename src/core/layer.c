#include "layer.h"
#include "macro.h"
#include "logger.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Layer"

layer_t*
sen_layer_new(const char* name)
{
  struct_malloc(layer_t, self);
  _logfi("-layer new");
  sen_node_init((node_t*)self, name, NULL);
  return self;
}

void
sen_layer_delete(void* self)
{
  _logfi("-layer destroy");
  sen_node_clean((node_t*)self);
  free(self);
}
