#include "signals.h"
#include "macro.h"
#include "logger.h"
#include "khash.h"
#include "utils.h"


#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Signals"

typedef struct slot_t
{
  object_t*         obj;
  char*             name;
  signal_callback_t proc;
} slot_t;

slot_t*
slot_new(object_t* listener, const char* name, signal_callback_t cb)
{
  sen_assert(listener || name);
  sen_assert(cb);
  struct_malloc(slot_t, self);
  if (listener) {
    self->obj = listener;
    self->name = sen_strdup(listener->name);
  }
  else
  {
    sen_assert(name);
    self->obj = NULL;
    self->name = sen_strdup(name);
  }
  _logfi("+slot [%s] obj", self->name);

  self->proc = cb;
  return self;
}

void
slot_destroy(slot_t* self)
{
  sen_assert(self);
  _logfi("-slot [%s] obj", (char*)self->name);
  free(self->name);
  free(self);
}
//---------------------------------------------------------------------------------

typedef struct sig_t {
  char*          name;
  khash_t(hmsp) *slots;
  int            refcount;
  void*          node;
}sig_t;

sig_t*
signal_new(const char* _name, void* parent)
{
  struct_malloc(sig_t, self);
  _logfi("new signal [%s]", _name);
  self->name = strdup(_name);
  self->slots = kh_init(hmsp);
  self->refcount = 0;
  self->node = parent;
  return self;
}

void
signal_destroy(sig_t* self)
{
  sen_assert(self);
  _logfi("-delete signal [%s]", self->name);

  slot_t* slot;
  kh_foreach_value(self->slots, slot, slot_destroy(slot) );
  kh_destroy(hmsp, self->slots);

  free(self->name);
  free(self);
}

static inline slot_t*
signal_find_slot(sig_t* self, const char* listener_name)
{
  sen_assert(listener_name);
  khiter_t pos = kh_get(hmsp, self->slots, listener_name);
  return pos != kh_end(self->slots) ? kh_val(self->slots, pos) : 0;
}
//-----------------------------------------------------------------------

typedef struct emitter_node_t {
  char          *name;
  object_t      *emitter;
  khash_t(hmsp) *signals;
} emitter_node_t;

static const char* UNKNOWN_EMITTER = "@@unknown";

emitter_node_t*
emitter_node_new (object_t *emitter,  const char* emitter_name)
{
  struct_malloc(emitter_node_t, self);


  if (emitter) {
    self->name = strdup((char*)emitter->name);
    self->emitter = emitter;
    _logfi("new emitter node [%s]", self->name );
  }
  else
  {
    sen_assert(emitter_name);
    self->name = strdup(emitter_name);
    _logfi("new anon emitter node [%s]", self->name );
    self->emitter = 0;
  }
  self->signals = kh_init(hmsp);
  return self;
}

void
emitter_node_destroy(emitter_node_t* self)
{
  sen_assert(self);
  _logfi(" -emitter node [%s]", self->name );
  free(self->name);
  sig_t* sig;
  kh_foreach_value(self->signals, sig, signal_destroy( sig) );
  kh_destroy(hmsp, self->signals);
  free(self);
}

static inline sig_t*
emitter_node_find_signal(emitter_node_t* self, const char* signal_name)
{
  sen_assert(signal_name);
  khiter_t pos = kh_get(hmsp, self->signals, signal_name);
  return pos != kh_end(self->signals) ? kh_val(self->signals, pos) : 0;
}
//--------------------------------------------------------------------------
typedef struct signals_t {
  khash_t(hmsp) *emitters;
}signals_t;

static signals_t* g_self = 0;

signals_t*
signals_new()
{
  struct_malloc(signals_t, self);
  self->emitters = kh_init(hmsp);

  return self;
}

void
signals_destroy(signals_t* self)
{
  emitter_node_t* node;
  kh_foreach_value(self->emitters, node, emitter_node_destroy( node) );
  kh_destroy(hmsp, self->emitters);
  free(self);
}

static inline emitter_node_t*
signals_find_emitter (const char* emitter_name)
{
  sen_assert(emitter_name);
  khiter_t pos = kh_get(hmsp, g_self->emitters, emitter_name);
  return pos != kh_end(g_self->emitters) ? kh_val(g_self->emitters, pos) : 0;
}

// --------------------------------------------------------------------
const void*
sen_signal_get(const char* signal_name, object_t* emitter)
{
  sen_assert(g_self);
  sen_assert(signal_name);
  const char* e_name = emitter ? (const char*)emitter->name : UNKNOWN_EMITTER;
  emitter_node_t* node = signals_find_emitter(e_name);
  sig_t* signal = 0;
  if (node) {

    if ((node->emitter == 0) && emitter) {
      node->emitter = emitter;
    }
    signal = emitter_node_find_signal(node, signal_name);
  }
  else {
    node = emitter_node_new(emitter, e_name);
    kh_insert(hmsp, g_self->emitters, node->name, node);
  }

  if (signal == 0) {
    signal = signal_new(signal_name, node);
    kh_insert(hmsp, node->signals, signal->name, signal);
  }
  signal->refcount++;
  return signal;
}

const void*
sen_signal_get_name(const char* signal_name,
                    const char* emmiter_name)
{
  sen_assert(g_self);
  sen_assert(signal_name);
  const char* e_name = emmiter_name ? emmiter_name : UNKNOWN_EMITTER;
  emitter_node_t* node = signals_find_emitter(e_name);
  sig_t* signal = 0;
  if (node) {
    signal = emitter_node_find_signal(node, signal_name);
  }
  else {
    node = emitter_node_new(0, e_name);
    kh_insert(hmsp, g_self->emitters, node->name, node);
  }

  if (signal == 0) {
    signal = signal_new(signal_name, node);
    kh_insert(hmsp, node->signals, signal->name, signal);
  }
  signal->refcount++;
  return signal;

}

void
sen_signal_release(const void* _signal)
{
  sen_assert(_signal);
  sig_t* signal = (sig_t*)_signal;
  if (signal->refcount) signal->refcount--;
  if (signal->refcount == 0 && signal->slots->size == 0) {
    emitter_node_t* node = (emitter_node_t*)signal->node;
    khiter_t pos = kh_get(hmsp, node->signals, signal->name);
    if (pos != kh_end(node->signals)) {
      signal_destroy(signal);
      kh_del(hmsp, node->signals, pos);
      if (kh_size(node->signals) == 0) {
        khiter_t pos = kh_get(hmsp, g_self->emitters, node->name);
        if (pos != kh_end(g_self->emitters)) {
          emitter_node_destroy(node);
          kh_del(hmsp, g_self->emitters, pos);
        }
      }
    }
  }
}

void
sen_signal_release_emitter_name(const char* name)
{
  sen_assert(name);
  khiter_t pos = kh_get(hmsp, g_self->emitters, name);
  if (pos != kh_end(g_self->emitters)){
    emitter_node_t * node = kh_val(g_self->emitters, pos);
    emitter_node_destroy(node);
    kh_del(hmsp, g_self->emitters, pos);
  }
}

void
sen_signal_release_emitter(object_t* emitter)
{
  sen_assert(emitter);
  sen_signal_release_emitter_name((char*)emitter->name);
}

void
sen_signal_emit(const void* _signal, void* data)
{
  sen_assert(_signal);
  sig_t* signal = (sig_t*)_signal;
  slot_t* slot;
  kh_foreach_value(signal->slots, slot,
      if (
      (*slot->proc) ( slot->obj, data, ((emitter_node_t*)signal->node)->emitter, signal->name )
      ) {
        _logfi("SIGNAL[%s] EMMIT BREAK", signal->name);
        break;
      }

  );
}

static inline void
_sen_signal_connect(const char*       emitter_name,
                    const char*       signal_name,
                    signal_callback_t proc,
                    object_t*         listener,
                    const char*       listener_name)
{
  if (!g_self) return;
  sen_assert(signal_name);
  sen_assert(proc);
  sen_assert(listener || listener_name);

  const char* e_name = emitter_name ? emitter_name : UNKNOWN_EMITTER;
  const char* l_name = listener ? (char*)listener->name : listener_name;

  emitter_node_t* node = signals_find_emitter (e_name);
  sig_t* signal = 0;
  if (node) {
    signal = emitter_node_find_signal(node, signal_name);
  }
  else {
    node = emitter_node_new(0, e_name);
    kh_insert(hmsp, g_self->emitters, node->name, node);
  }

  if (signal == 0) {
    signal = signal_new(signal_name, node);
    kh_insert(hmsp, node->signals, signal->name, signal);
  }
  else
  {
    if ( signal_find_slot(signal, l_name) ) {
      _logfi("signal [%s] slot [%s] already connected", signal_name, l_name);
      return;
    }
  }
  _logfi("SLOT[%s] connected to SIGNAL[%s] on [%s]", l_name, signal_name, e_name);
  slot_t* new_slot = slot_new(listener, listener_name, proc);
  kh_insert(hmsp, signal->slots, (char*) new_slot->name, new_slot );
}

void
sen_signal_connect(const char*       emitter_name,
                   const char*       signal_name,
                   signal_callback_t proc,
                   object_t*         listener)
{
  _sen_signal_connect(emitter_name, signal_name, proc, listener, NULL);
}

void
sen_signal_connect_name(const char*       emitter_name,
                        const char*       signal_name,
                        signal_callback_t proc,
                        const char*       listener_name)
{
  _sen_signal_connect(emitter_name, signal_name, proc, NULL, listener_name);
}

static void
_sen_signal_disconnect(object_t*         listener,
                       const char*       listener_name,
                       const char*       signal_name,
                       const char*       emitter_name)
{
  sen_assert(listener || listener_name);
  const char* l_name = listener ? (char*)listener->name : listener_name;
  if (emitter_name) {
    emitter_node_t* node = signals_find_emitter (emitter_name);
    if (!node) return;
    if (signal_name) {
      sig_t* signal = emitter_node_find_signal(node, signal_name);
      if (!signal) return;
      khiter_t pos = kh_get(hmsp, signal->slots, l_name);
      if (pos != kh_end(signal->slots)) {
        slot_destroy( kh_val(signal->slots, pos) );
        kh_del(hmsp, signal->slots, pos);
      }
    }
    else {
      sig_t* signal;
      kh_foreach_value(node->signals, signal,
          khiter_t pos = kh_get(hmsp, signal->slots, l_name);
          if (pos != kh_end(signal->slots)) {
            slot_destroy( kh_val(signal->slots, pos) );
            kh_del(hmsp, signal->slots, pos);
          }
       );
    }
  }
  else
  {
    emitter_node_t* node;
    kh_foreach_value(g_self->emitters, node,
        _sen_signal_disconnect(listener, listener_name, signal_name, node->name)
    );
  }

}

void
sen_signal_disconnect(object_t*         listener,
                      const char*       signal_name,
                      const char*       emitter_name)
{
  _sen_signal_disconnect(listener, NULL, signal_name, emitter_name);
}

void
sen_signal_disconnect_name(const char*       listener_name,
                           const char*       signal_name,
                           const char*       emitter_name)
{
  _sen_signal_disconnect(NULL, listener_name, signal_name, emitter_name);
}
// --------------------------------------------------------------------
void
sen_signals_manager_init()
{
  if (g_self == 0) {
    _logfi("init signals manager");
    g_self = signals_new();
  }
}

void
sen_signals_manager_destroy()
{
  if (g_self) {
    _logfi("destroy signals manager");
    signals_destroy(g_self);
  }
}
