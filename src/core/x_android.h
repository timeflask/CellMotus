#ifndef __X_android_H_
#define __X_android_H_
#include "config.h"
#include <jni.h>

typedef struct jni_method_t
{
  JNIEnv *    env;
  jclass      classID;
  jmethodID   methodID;
} jni_method_t;

void
sen_android_set_javavm(JavaVM *javaVM);

JavaVM*
sen_androud_get_javavm();

JNIEnv*
sen_androud_get_env();

int
sen_android_set_classLoader(jobject activityInstance);

int
sen_android_get_static_method(jni_method_t *methodinfo,
                              const char *className,
                              const char *methodName,
                              const char *paramCode);

int
sen_android_get_method(jni_method_t *methodinfo,
                       const char *className,
                       const char *methodName,
                       const char *paramCode);

char*
sen_jstring_dup(jstring str);



#endif
