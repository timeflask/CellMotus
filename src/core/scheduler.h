#ifndef __scheduler_H_
#define __scheduler_H_
#include "config.h"
#include "node.h"

typedef int (*scheduler_update_callback)(void*, double, const char*);

typedef struct scheduler_t {
  object_t super;
  float    scale;
  void     *nodes;
}scheduler_t;

scheduler_t*
sen_scheduler_new(const char* name);

void
sen_scheduler_delete(scheduler_t* self);

void
sen_scheduler_add(scheduler_t*              self,
                  object_t*                 obj,
                  scheduler_update_callback callback,
                  const char*               key,
                  double                    interval,
                  int                       repeat,
                  double                    delay,
                  int                       pause);

void
sen_scheduler_remove(scheduler_t*              self,
                     object_t*                 obj,
                     const char*               key);

void sen_scheduler_pause(scheduler_t* self,
                         object_t* obj,
                         const char* key);

void sen_scheduler_resume(scheduler_t* self,
                          object_t* obj,
                          const char* key);

int sen_scheduler_is_running(scheduler_t* self,
                              object_t* obj,
                              const char* key
                              );

extern
scheduler_t* sen_scheduler();
//---------------------------------------------------------------------------
int
sen_scheduler_update(void* self, double dt);

#endif
