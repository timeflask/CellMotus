#ifndef __types_H_
#define __types_H_
#include "config.h"
#include "macro.h"
typedef struct {
    float x, y, z, w;
    float s, t;
    float r, g, b, a;
} V4F_T2F_C4F;

typedef struct {
    float x, y, z, w;
    float r, g, b, a;
} V4F_C4F;

#endif
