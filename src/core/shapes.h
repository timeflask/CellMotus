#ifndef __shapes_H_
#define __shapes_H_
#include "config.h"
#include "lmath.h"


void
sen_shapes_init();

void
sen_shapes_destroy();

void
sen_shapes_reload();

void
sen_shapes_line(const vec2* start, const vec2* end);

#endif
