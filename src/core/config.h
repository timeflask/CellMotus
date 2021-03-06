#ifndef __config_H_
#define __config_H_
#include <stdlib.h>
#include <stdint.h>


#define SEN_PLATFORM_0            0
#define SEN_PLATFORM_IOS          1
#define SEN_PLATFORM_ANDROID      2
#define SEN_PLATFORM_WIN32        3
#define SEN_PLATFORM_LINUX        4
#define SEN_PLATFORM_MAC          5
#define SEN_PLATFORM_WINPHONE8    6

#define SEN_PLATFORM SEN_PLATFORM_LINUX

#define SEN_DECL

#if defined(ANDROID)
  #undef SEN_PLATFORM
  #define SEN_PLATFORM SEN_PLATFORM_ANDROID
#endif

#if defined(LINUX) && !defined(__APPLE__)
  #undef SEN_PLATFORM
  #define SEN_PLATFORM SEN_PLATFORM_LINUX
#endif

#if defined(WIN32)
  #undef SEN_PLATFORM
  #define SEN_DECL __declspec(dllexport)
  #define SEN_PLATFORM SEN_PLATFORM_WIN32
#endif

#if defined(__APPLE__)
  #undef SEN_PLATFORM
  #define SEN_PLATFORM SEN_PLATFORM_MAC
#endif


#if defined(SEN_TARGET_IOS)
  #undef SEN_PLATFORM
  #define SEN_PLATFORM SEN_PLATFORM_IOS
#endif

#if (SEN_PLATFORM == SEN_PLATFORM_ANDROID)
  extern struct AAssetManager* g_AssetManager;
#endif

#include "platform-config.h"

#define SEN_LOG_TAG "SEN"
#ifdef SEN_DEBUG
  #define SEN_LOGGER_ENABLE
  #define SEN_EVENT_STACK_CHECK
#endif

//#define SEN_LOGGER_ENABLE

#endif

