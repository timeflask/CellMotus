LOCAL_PATH := $(call my-dir)
PROJECT_ROOT_PATH := $(LOCAL_PATH)/../../
CORE_RELATIVE_PATH := ../../core/
EXT_PATH := $(PROJECT_ROOT_PATH)ext

#LOCAL_C_INCLUDES += $(EXT_PATH)/png/include

include $(CLEAR_VARS)

LOCAL_MODULE    := game
LOCAL_CFLAGS    := -Wall -Wextra
LOCAL_CPPFLAGS += -fexceptions
 
LOCAL_SRC_FILES :=\
jni.cpp \
$(CORE_RELATIVE_PATH)engine.c \
$(CORE_RELATIVE_PATH)object.c \
$(CORE_RELATIVE_PATH)vector.c  \
$(CORE_RELATIVE_PATH)utils.c \
$(CORE_RELATIVE_PATH)x_android_logger.c \
$(CORE_RELATIVE_PATH)x_android_asset.c \
$(CORE_RELATIVE_PATH)x_posix_timer.c \
$(CORE_RELATIVE_PATH)lmath.c \
$(CORE_RELATIVE_PATH)vertex-attribute.c \
$(CORE_RELATIVE_PATH)vertex-buffer.c \
$(CORE_RELATIVE_PATH)shader.c \
$(CORE_RELATIVE_PATH)texture-atlas.c \
$(CORE_RELATIVE_PATH)image.c \
$(CORE_RELATIVE_PATH)texture.c \
$(CORE_RELATIVE_PATH)texture-manager.c \
$(CORE_RELATIVE_PATH)node.c \
$(CORE_RELATIVE_PATH)sprite.c \
$(CORE_RELATIVE_PATH)view.c \
$(CORE_RELATIVE_PATH)camera.c \
$(CORE_RELATIVE_PATH)signals.c \
$(CORE_RELATIVE_PATH)font.c \
$(CORE_RELATIVE_PATH)edtaa3func.c \
$(CORE_RELATIVE_PATH)label.c \
$(CORE_RELATIVE_PATH)scheduler.c \
$(CORE_RELATIVE_PATH)luas.c \
$(CORE_RELATIVE_PATH)scene.c \
$(CORE_RELATIVE_PATH)layer.c \
$(CORE_RELATIVE_PATH)render.c \
$(CORE_RELATIVE_PATH)input.c \
$(CORE_RELATIVE_PATH)shapes.c \
$(CORE_RELATIVE_PATH)quad.c \
$(CORE_RELATIVE_PATH)x_android.cpp \
$(CORE_RELATIVE_PATH)x_android_audio.cpp \
$(CORE_RELATIVE_PATH)x_android_settings.cpp \


#$(CORE_RELATIVE_PATH)x_posix_mutex.c \
                   
LOCAL_C_INCLUDES += $(PROJECT_ROOT_PATH)core/
#LOCAL_C_INCLUDES += $(PROJECT_ROOT_PATH)3rdparty/linmath/
#LOCAL_C_INCLUDES += $(PROJECT_ROOT_PATH)3rdparty/luajit-2.0/src
#LOCAL_C_INCLUDES += /home/moomcy/install/android-ext/include
#LOCAL_C_INCLUDES += /home/moomcy/install/android-ext/include/freetype2/


LOCAL_STATIC_LIBRARIES := libpng
LOCAL_STATIC_LIBRARIES += libluajit
LOCAL_STATIC_LIBRARIES += libfreetype 


LOCAL_LDLIBS := -lGLESv2 -llog -landroid 
#LOCAL_LDLIBS += -lEGL
#LOCAL_LDLIBS += $(PROJECT_ROOT_PATH)3rdparty/luajit-2.0/src/libluajit.a
#LOCAL_LDLIBS += /home/moomcy/tmp/$(TARGET_ARCH_ABI)/lib/libluajit-5.1.a
#LOCAL_LDLIBS += /home/moomcy/dev/sotas/src/3rdparty/ft/libfreetype.a

# -pthread

#PLATFORM_PREFIX := /home/moomcy/install/android-ext/
#LUAJIT_PATH := $(PROJECT_ROOT_PATH)3rdparty/luajit-2.0/src
#LOCAL_LDLIBS += /home/moomcy/install/android-ext/lib/libfreetype.a


include $(BUILD_SHARED_LIBRARY)

#$(call import-add-path,$(PROJECT_ROOT_PATH)3rdparty)
#$(call import-module,ft)
#$(call import-module,libpng)
#$(call import-module,luajit-2.0/src)


#$(call import-add-path,$(EXT_PATH)png/libs)
$(call import-add-path,$(EXT_PATH))
$(call import-module, png/libs/android)
$(call import-module, ft2/libs/android)
$(call import-module, luajit/libs/android)


