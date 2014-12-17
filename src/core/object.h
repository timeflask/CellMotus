#ifndef __object_H_
#define __object_H_
#include "config.h"

typedef struct object_t {
  char             name[32];
  uint32_t         uid;
  struct object_t* parent;
} object_t;


void
sen_object_init(void* _self, const char* _name, void* _parent);

void
sen_object_set_name(void* _self, const char* _name);

const char*
sen_object_get_name(void* _self);


///

#define obj_name_(p) ((const char*)(((object_t*)(p))->name))
#define obj_id_(p) ((const char*)(((object_t*)(p))->uid))

#endif
