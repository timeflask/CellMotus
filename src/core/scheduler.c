#include "scheduler.h"
#include "macro.h"
#include "utils.h"
#include "logger.h"
#include "khash.h"
#include <stdio.h>

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Scheduler"

typedef struct scheduler_entry_t
{
  object_t*                 node;
  scheduler_update_callback callback;
  double                    interval;
  int                       repeat;
  double                    delay;
  int                       pause;
  //------------------------------
  int                       bEveryFrame;
  int                       bUseDelay;
  int                       bRunForever;
  double                    elapsed;
  int                       execTimes;
  //-------------------------------
  int                       markedForDeath;
  //-------------------------------
  char*                     key;
} scheduler_entry_t;


static uint32_t uniqCounter = 0;

scheduler_entry_t*
scheduler_entry_new(object_t*                 node,
                    scheduler_update_callback callback,
                    const char*               key,
                    double                    interval,
                    int                       repeat,
                    double                    delay,
                    int                       pause)
{
  sen_assert(node);
  sen_assert(callback);
  struct_malloc(scheduler_entry_t, self);

  if (key)
    self->key = sen_strdup(key);
  else
  {
    self->key = (char*)malloc(16);
    sprintf((char*)(self->key), "sk_%u", ++uniqCounter);
  }

  _logfi("new scheduler entry, node[%s] key[%s], I=%f, R=%d, D=%f, PAUSE=%d", node->name, self->key, interval, repeat, delay, pause );

  self->node = node;
  self->callback = callback;
  self->interval = interval;
  self->pause = pause;
  self->delay = delay;
  self->repeat = repeat;

  self->bEveryFrame = self->interval <= 0;
  self->bUseDelay = self->delay > 0;
  self->bRunForever = self->repeat < 0;
  self->elapsed = -1;
  self->execTimes = 0;
  self->markedForDeath = 0;


  return self;
}

void
scheduler_entry_delete(scheduler_entry_t* self)
{
  _logfi("delete scheduler entry, key[%s]", self->key );
  free(self->key);
  free(self);
}

int
scheduler_entry_update(scheduler_entry_t* self, double dt)
{
  if (self->markedForDeath) return 1;
  if (self->elapsed < 0)
  {
    self->execTimes = 0;
    self->elapsed   = 0;
  }
  else
  {
    if (self->bRunForever && !self->bUseDelay)
    {
        self->elapsed += dt;
        if (self->elapsed >= self->interval)
        {
          self->markedForDeath = self->callback(self->node, self->elapsed, self->key);
          self->elapsed = 0;

        }
    }
    else
    {
      self->elapsed += dt;
      if (self->bUseDelay)
      {
        if(self->elapsed >= self->delay )
        {
          self->markedForDeath =  self->callback(self->node, self->elapsed, self->key);
          self->elapsed -= self->delay;
          self->execTimes += 1;
          self->bUseDelay = 0;
        }
      }
      else
      {
        if (self->elapsed >= self->interval)
        {
          self->markedForDeath = self->callback(self->node, self->elapsed, self->key);
          self->elapsed = 0;
          self->execTimes += 1;

        }
      }
      if (!self->bRunForever &&  self->execTimes > self->repeat)
      {
        self->markedForDeath = 1;
      }
    }
  }
  return self->markedForDeath;
}
//---------------------------------------------------------------------
typedef struct scheduler_node_t
{
  khash_t(hmsp)* entries;
  int            pause;
  int            markedForDeath;
} scheduler_node_t;

scheduler_node_t*
scheduler_node_new()
{
  struct_malloc(scheduler_node_t, self);
  self->entries = kh_init(hmsp);
  self->pause = 0;
  self->markedForDeath = 0;
  return self;
}
scheduler_node_t*
scheduler_node_delete(scheduler_node_t* self)
{
  sen_assert(self);
  scheduler_entry_t* entry;
  kh_foreach_value(self->entries, entry, scheduler_entry_delete(entry) );
  kh_destroy(hmsp, self->entries);
  return self;
}
static int g_update_break = 0;
static int g_total_updated = 0;
void
scheduler_node_update(scheduler_node_t* self, double dt)
{
  if (self->pause) return;

  scheduler_entry_t* entry;
  kh_foreach_value( self->entries, entry,
      //if (g_update_break) break;
      if (!entry->pause && !entry->markedForDeath) {
        scheduler_entry_update(entry, dt);
        g_total_updated ++;
      }
  );
  kh_foreach_value( self->entries, entry,
        if (entry->markedForDeath) {
          _logfi("scheduler entry [%s] is marked for death", entry->key);
          scheduler_entry_delete( kh_val( self->entries, __i ) );
          kh_del(hmsp, self->entries, __i );
        }
  );

  self->markedForDeath = kh_size(self->entries) <= 0;
}

//---------------------------------------------------------------------
scheduler_t*
sen_scheduler_new(const char* name)
{
  _logfi("init scheduler");
  struct_malloc(scheduler_t, self);

  self->scale = 1.0f;
  self->nodes      = kh_init(hmip);
  sen_object_init(self, name, NULL);

  return self;
}

void
sen_scheduler_clear_nodes (scheduler_t* self)
{
  khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
  scheduler_node_t* node;
  kh_foreach_value(nodes, node, scheduler_node_delete(node) );
  kh_clear(hmip, nodes);
}

void
sen_scheduler_delete(scheduler_t* self)
{
  _logfi("delete scheduler");

  sen_scheduler_clear_nodes (self);
  kh_destroy(hmip, (khash_t(hmip) *)self->nodes);

  free(self);
}


void
sen_scheduler_add(scheduler_t*              self,
                  object_t*                 node_self,
                  scheduler_update_callback callback,
                  const char*               key,
                  double                    interval,
                  int                       repeat,
                  double                    delay,
                  int                       pause)
{
  sen_assert(node_self);
  sen_assert(callback);

  khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
  scheduler_node_t* node;
  uint32_t uid = node_self->uid;
  khiter_t k = kh_get(hmip, nodes, uid );
  if ( k != kh_end(nodes) )
  {
    node = kh_val(nodes, k);
    if (key) {
      khiter_t k2 = kh_get(hmsp, node->entries, key);
      if (k2 != kh_end(node->entries)) {
        //_logfi(" !entry key[%s] for node[%s] already exist, skip", key,(char*)  node_self->name);

        return;
      }

    }
  }
  else
  {
    _logfi("new scheduler node obj[%s] uid[%u]", (char*) node_self->name, uid);
    node = scheduler_node_new();
    kh_insert(hmip, nodes, uid, node);
  }

  scheduler_entry_t* new_entry =
  scheduler_entry_new(node_self,
                      callback,
                      key,
                      interval,
                      repeat,
                      delay,
                      pause);

  kh_insert(hmsp, node->entries, new_entry->key, new_entry);
 // g_update_break = 1;
}

int sen_scheduler_is_running(scheduler_t* self,
                              object_t* node_self,
                              const char* key)
{
  khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
  uint32_t uid = node_self->uid;
  khiter_t k = kh_get(hmip, nodes, uid );
  if (k == kh_end(nodes)) return 0;
  scheduler_node_t* node = kh_val(nodes, k);
  return key ?
      kh_get(hmsp, node->entries, key) != kh_end(node->entries) :
      kh_size(node->entries) > 0;
}

void
sen_scheduler_remove_node(scheduler_t* self,
                          const char*  key,
                          khiter_t k)
{
  khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
  scheduler_node_t* node = kh_val(nodes, k);
  if (key == NULL)
  {
    scheduler_node_delete(node);
    kh_del(hmip, nodes, k );
  }
  else
  {
    khiter_t k2 = kh_get(hmsp, node->entries, key);
    if (k2 == kh_end(node->entries)) {
      _logfe("scheduler node has no entries with key[%s]",  key);
    }
    else
    {
      scheduler_entry_delete( kh_val( node->entries, k2 ) );
      kh_del(hmsp, node->entries, k2 );
    }
  }
}

void
sen_scheduler_remove(scheduler_t*              self,
                     object_t*                 node_self,
                     const char*               key)
{
  if (node_self==NULL && key==NULL) {
    sen_scheduler_clear_nodes(self);
  }
  else if (node_self != NULL ) {
    khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
    khiter_t k = kh_get(hmip, nodes, node_self->uid );
    if ( k != kh_end(nodes) )
      sen_scheduler_remove_node(self, key, k);
  }
  else
  {
    khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
    khint_t i;
    for (i = kh_begin(nodes); i != kh_end(nodes); ++i) {
      if (!kh_exist(nodes,i)) continue;
      sen_scheduler_remove_node(self, key, i);
    }
  }
  g_update_break = 1;
}

static void
sen_scheduler_pause_node(scheduler_t* self,
                          const char*  key,
                          khiter_t k,
                          int pause)
{
  khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
  scheduler_node_t* node = kh_val(nodes, k);
  if (key == NULL)
  {
    node->pause = pause;
    scheduler_entry_t* entry;
    kh_foreach_value(node->entries, entry, entry->pause = pause );
  }
  else
  {
    khiter_t k2 = kh_get(hmsp, node->entries, key);
    if (k2 == kh_end(node->entries)) {
      _logfe("scheduler node has no entries with key[%s]",  key);
    }
    else
    {
      scheduler_entry_t* e = kh_val( node->entries, k2 );
      e->pause = pause;
    }
  }
}

static inline void _sen_scheduler_pause(scheduler_t* self,
                                        object_t* obj,
                                        const char* key,
                                        int pause)
{
  khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
  scheduler_node_t* node;
  if (obj==NULL && key==NULL) {
    kh_foreach_value(nodes, node, node->pause=pause );
  }
  else if (obj != NULL ) {
    khiter_t k = kh_get(hmip, nodes, obj->uid );
    if ( k != kh_end(nodes) )
      sen_scheduler_pause_node(self, key, k, pause);
  }
  else
  {
    khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
    khint_t i;
    for (i = kh_begin(nodes); i != kh_end(nodes); ++i) {
      if (!kh_exist(nodes,i)) continue;
      sen_scheduler_pause_node(self, key,  i, pause);
    }
  }
}

void sen_scheduler_pause(scheduler_t* self,
                         object_t* obj,
                         const char* key)
{
  _logfi("Scheduler paused");
  _sen_scheduler_pause(self, obj, key, 1);
}

void sen_scheduler_resume(scheduler_t* self,
                          object_t* obj,
                          const char* key)
{
  _logfi("Scheduler resumed");
  _sen_scheduler_pause(self, obj, key, 0);
}


int
sen_scheduler_update(void* _self, double dt)
{
  scheduler_t* self = (scheduler_t*)_self;
  khash_t(hmip) *nodes = (khash_t(hmip) *)self->nodes;
  scheduler_node_t* node;

  g_total_updated = 0;
  if (self->scale != 1.0f)
      dt *= self->scale;
  g_update_break  = 0;
  kh_foreach_value( nodes, node,
      //if (g_update_break) break;
      scheduler_node_update(node, dt); );

  return g_total_updated ;
}
