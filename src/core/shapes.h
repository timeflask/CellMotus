#ifndef __shapes_H_
#define __shapes_H_
#include "config.h"
#include "lmath.h"


SEN_DECL void
sen_shapes_init();

SEN_DECL void
sen_shapes_destroy();

SEN_DECL void
sen_shapes_reload();

SEN_DECL void
sen_shapes_line(const vec2* start, const vec2* end);

#endif
