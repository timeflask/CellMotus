#ifndef __utils_H_
#define __utils_H_
#include "config.h"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
char *sen_strdup (const char *s);
wchar_t *sen_strdupW (const wchar_t *s);
wchar_t *sen_strdupToW (const char* s);

#ifdef __APPLE__
    char * strndup( const char *s1, size_t n);
#elif defined(_WIN32) || defined(_WIN64)
    char * strndup( const char *s1, size_t n);
#endif // _WIN32 || _WIN64


#endif
