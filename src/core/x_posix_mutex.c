#include "mutex.h"
#include "logger.h"
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Mutex"
void*
mutex_new()
{
  pthread_mutex_t *_mutex = (pthread_mutex_t *)calloc(sizeof(pthread_mutex_t),1);

  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
  if (pthread_mutex_init(_mutex, &attr))
  {
    pthread_mutexattr_destroy(&attr);
    _logfe("cannot create mutex");
    exit(1);
  }
  pthread_mutexattr_destroy(&attr);

  return _mutex;
}

void
mutex_delete(void* self)
{
  sen_assert(self);
  pthread_mutex_destroy(self);
  free(self);
}

void
mutex_lock(void* self)
{
  if (pthread_mutex_lock(self)) {
    _logfe("cannot lock mutex");
    exit(1);
  }
}

int
mutex_try_lock(void* self)
{
  int rc = pthread_mutex_trylock(self);
  if (rc == 0)
    return 1;
  else if (rc == EBUSY)
    return 0;
  else {
    _logfe("cannot lock mutex");
    exit(1);
  }

}

void
mutex_unlock(void* self)
{
  if (pthread_mutex_unlock(self)) {
    _logfe("cannot unlock mutex");
    exit(1);
  }

}


