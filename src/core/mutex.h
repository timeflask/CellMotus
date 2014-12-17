#ifndef __mutex_H_
#define __mutex_H_
#include <stdlib.h>

void*
mutex_new();

void
mutex_delete(void* self);

void
mutex_lock(void* self);

int
mutex_try_lock(void* self);

void
mutex_unlock(void* self);

#endif
