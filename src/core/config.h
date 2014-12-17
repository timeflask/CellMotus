#ifndef __config_H_
#define __config_H_
#include <stdlib.h>

#if defined(__BORLANDC__)
    typedef unsigned char uint8_t;
    typedef __int64 int64_t;
    typedef unsigned long uintptr_t;
#elif defined(_MSC_VER)
    typedef unsigned char uint8_t;
    typedef __int64 int64_t;
    typedef __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;
#else
    #include <stdint.h>
#endif


#define SEN_PLATFORM_0            0
#define SEN_PLATFORM_IOS          1
#define SEN_PLATFORM_ANDROID      2
#define SEN_PLATFORM_WIN32        3
#define SEN_PLATFORM_LINUX        4
#define SEN_PLATFORM_MACOS        5
#define SEN_PLATFORM_WINPHONE8    6

#define SEN_PLATFORM SEN_PLATFORM_0

#if defined(ANDROID)
  #undef SEN_PLATFORM
  #define SEN_PLATFORM SEN_PLATFORM_ANDROID
#endif

#if defined(LINUX)
  #undef SEN_PLATFORM
  #define SEN_PLATFORM SEN_PLATFORM_LINUX
#endif


#if (SEN_PLATFORM == SEN_PLATFORM_ANDROID)
  extern struct AAssetManager* g_AssetManager;
#endif




#define SEN_LOG_TAG "SEN"
#ifdef SEN_DEBUG
  #define SEN_LOGGER_ENABLE
  #define SEN_EVENT_STACK_CHECK
#endif

//#define SEN_LOGGER_ENABLE

#endif

