NDK_TOOLCHAIN_VERSION := 4.9
APP_PLATFORM := android-10
APP_ABI := armeabi armeabi-v7a x86
#APP_ABI := all
#APP_OPTIM := debug
APP_OPTIM := release

#PLATFORM_PREFIX := /home/moomcy/install/android-ext
#APP_CFLAGS += -I$(PLATFORM_PREFIX)/include/freetype2/
#APP_CFLAGS += -I$(PLATFORM_PREFIX)/include/

ifeq ($(APP_OPTIM),debug)
  APP_CFLAGS := -O0 -DSEN_DEBUG -g $(APP_CFLAGS)
else
  APP_CFLAGS := -O2 -DNDEBUG -g $(APP_CFLAGS)
endif 