#ifndef __layer_H_
#define __layer_H_
#include "config.h"
#include "node.h"

typedef struct layer_t {
  node_t super;
} layer_t;

SEN_DECL layer_t*
sen_layer_new(const char* name);

SEN_DECL void
sen_layer_delete(void* self);



#endif
