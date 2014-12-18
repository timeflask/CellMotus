#ifndef __logger_H_
#define __logger_H_
#include "config.h"
#include "macro.h"
SEN_DECL void sen_printf(const char* fmt, ...); // sen_logi , tag == "Engine"
SEN_DECL void sen_logv(const char* tag, const char* text, ...);
SEN_DECL void sen_logd(const char* tag, const char* text, ...);
SEN_DECL void sen_logi(const char* tag, const char* text, ...);
SEN_DECL void sen_logw(const char* tag, const char* text, ...);
SEN_DECL void sen_loge(const char* tag, const char* text, ...);


#endif
