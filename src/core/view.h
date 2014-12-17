#ifndef __view_H_
#define __view_H_
#include "config.h"
#include "lmath.h"

struct gl_context_attr
{
    int r, g, b, a, d, s;
};


void
sen_view_set_resolution(float w, float h);

void
sen_view_set_screen(float x, float y, float w, float h);

const vec4*
sen_view_get_viewport();

void
sen_view_set_screenv4(const vec4* vp);

void
sen_view_reset();

const struct gl_context_attr*
sen_view_get_attributes();

void
sen_view_set_attributes(const struct gl_context_attr* attrs);

//----------------------------------------------------------------------
void
sen_views_init();

void
sen_views_destroy();

#endif
