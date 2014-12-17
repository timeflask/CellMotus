#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "object.h"
#include "macro.h"
#include "logger.h"
#include "utils.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Object"

static uint32_t uniqCounter = 0;

void
sen_object_set_name(void* _self, const char* _name)
{
  object_t* self = (object_t* )_self;
  sen_assert(self);
  memset(self->name, 0, sizeof(self->name));
  if (_name) {
    memcpy(self->name, _name, min (strlen(_name), sizeof(self->name) - 1 ) );
  }
  else {
    sprintf((char*)(self->name), "obj_%u", self->uid);
  }
}

void
sen_object_init(void* _self, const char* _name, void* _parent)
{
  object_t* self = (object_t* )_self;
  sen_assert(self);

  self->uid = ++uniqCounter;
  self->parent = _parent;
  sen_object_set_name(_self, _name);
 // _logfi(" + Object [%s]", self->name);
}

const char*
sen_object_get_name(void* _self)
{
  object_t* self = (object_t* )_self;
  sen_assert(self);
  return (const char*) (self->name);
}
