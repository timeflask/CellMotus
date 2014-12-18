#ifndef __view_H_
#define __view_H_
#include "config.h"
#include "lmath.h"

struct gl_context_attr
{
    int r, g, b, a, d, s;
};


SEN_DECL void
sen_view_set_resolution(float w, float h);

SEN_DECL void
sen_view_set_screen(float x, float y, float w, float h);

SEN_DECL const vec4*
sen_view_get_viewport();

SEN_DECL void
sen_view_set_screenv4(const vec4* vp);

SEN_DECL void
sen_view_reset();

SEN_DECL const struct gl_context_attr*
sen_view_get_attributes();

SEN_DECL void
sen_view_set_attributes(const struct gl_context_attr* attrs);

//----------------------------------------------------------------------
SEN_DECL void
sen_views_init();

SEN_DECL void
sen_views_destroy();

#endif
