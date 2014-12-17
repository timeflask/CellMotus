#include "config.h"
#include "logger.h"
#include <android/log.h>

#define ANDROID_LOG_VPRINT(priority)\
  va_list arg_ptr; \
  va_start(arg_ptr, fmt); \
  __android_log_vprint(priority, tag, fmt, arg_ptr); \
  va_end(arg_ptr);

void sen_logv(const char *tag, const char *fmt, ...) {
	ANDROID_LOG_VPRINT(ANDROID_LOG_VERBOSE);
}

void sen_logd(const char *tag, const char *fmt, ...) {
	ANDROID_LOG_VPRINT(ANDROID_LOG_DEBUG);
}

void sen_logi(const char *tag, const char *fmt, ...) {
	ANDROID_LOG_VPRINT(ANDROID_LOG_INFO);
}

void sen_logw(const char *tag, const char *fmt, ...) {
	ANDROID_LOG_VPRINT(ANDROID_LOG_WARN);
}

void sen_loge(const char *tag, const char *fmt, ...) {
	ANDROID_LOG_VPRINT(ANDROID_LOG_ERROR);
}

void sen_printf(const char* fmt, ...) {
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	__android_log_vprint(ANDROID_LOG_INFO, "SEN", fmt, arg_ptr);
	va_end(arg_ptr);
}
