JIT_PATH := /home/moomct/dev/sotas/srs/3rdparty/luajit-2.0/src
LOCAL_PATH : $(JIT_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := libluajit
LOCAL_SRC_FILES := libluajit.a
LOCAL_EXPORT_C_INCLUDE_DIRS := $(JIT_PATH)
include $(PREBUILD_STATIC_LIBRARY)



# $(call my-dir)
