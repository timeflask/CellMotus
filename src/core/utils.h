#ifndef __utils_H_
#define __utils_H_
#include "config.h"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
char *sen_strdup (const char *s);
char *sen_strndup (const char *s, size_t n);
wchar_t *sen_strdupW (const wchar_t *s);
wchar_t *sen_strdupToW (const char* s);

#endif
