#ifndef __logger_H_
#define __logger_H_
#include "config.h"
#include "macro.h"
void sen_printf(const char* fmt, ...); // sen_logi , tag == "Engine"
void sen_logv(const char* tag, const char* text, ...);
void sen_logd(const char* tag, const char* text, ...);
void sen_logi(const char* tag, const char* text, ...);
void sen_logw(const char* tag, const char* text, ...);
void sen_loge(const char* tag, const char* text, ...);


#endif
