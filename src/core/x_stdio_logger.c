#include "config.h"
#include "logger.h"
#include <stdio.h>
#include <stdarg.h>

#define MAX_BUFF 4096

#define LOG_PRINTF \
  char buffer[MAX_BUFF]; \
  va_list args; \
  va_start (args, fmt); \
  vsprintf (buffer,fmt, args); \
  va_end (args); \
  printf("%s:%s\n", tag, buffer);


void sen_logv(const char *tag, const char *fmt, ...) {
    LOG_PRINTF;
}

void sen_logd(const char *tag, const char *fmt, ...) {
  LOG_PRINTF;
}

void sen_logi(const char *tag, const char *fmt, ...) {
  LOG_PRINTF;
}

void sen_logw(const char *tag, const char *fmt, ...) {
  LOG_PRINTF;
}

void sen_loge(const char *tag, const char *fmt, ...) {
  char buffer[MAX_BUFF];
  va_list args;
  va_start (args, fmt);
  vsprintf (buffer,fmt, args);
  va_end (args);
  fprintf(stderr,"%s:%s\n", tag, buffer);
}

void sen_printf(const char* fmt, ...) {
  char buffer[MAX_BUFF];
  va_list args;
  va_start (args, fmt);
  vsprintf (buffer,fmt, args);
  va_end (args);
  printf("%s:%s:%s\n", "I", SEN_LOG_TAG, buffer);
}
