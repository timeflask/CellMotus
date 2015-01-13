#ifndef __signals_H_
#define __signals_H_
#include "config.h"
#include "object.h"


typedef int (*signal_callback_t)(object_t*, void* data, object_t*, const char*);
                                 // self     // data    // emmiter  //sig name

SEN_DECL const void*
sen_signal_get(const char* signal_name,
               object_t* emitter);

SEN_DECL const void*
sen_signal_get_name(const char* signal_name,
                    const char* emmiter_name);

SEN_DECL void
sen_signal_release(const void* signal);

SEN_DECL void
sen_signal_release_emitter(object_t* emitter);

SEN_DECL void
sen_signal_release_emitter_name(const char* name);

SEN_DECL void
sen_signal_emit(const void* signal,
                void* data);

SEN_DECL void
sen_signal_connect(const char*       emitter_name,
                   const char*       signal_name,
                   signal_callback_t proc,
                   struct object_t*  listener);

SEN_DECL void
sen_signal_connect_lua(const char*       emitter_name,
                       const char*       signal_name,
                       struct object_t*  listener);

SEN_DECL void
sen_signal_connect_name(const char*       emitter_name,
                        const char*       signal_name,
                        signal_callback_t proc,
                        const char*       listener_name);
SEN_DECL void
sen_signal_disconnect(object_t*         listener,
                      const char*       signal_name,
                      const char*       emitter_name);

SEN_DECL void
sen_signal_disconnect_name(const char*       listener_name,
                           const char*       signal_name,
                           const char*       emitter_name);


// ---------------------------------------------------------------------
SEN_DECL void
sen_signals_manager_init();

SEN_DECL void
sen_signals_manager_destroy();

#endif
