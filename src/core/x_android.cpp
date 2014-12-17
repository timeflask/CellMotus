#include "x_android.h"
#include <pthread.h>
extern "C" {
#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "platform.h"
#include "engine.h"

}

static JavaVM*       g_java_vm              = NULL;
static jmethodID     g_load_class_method_id = NULL;
static jobject       g_class_loader         = NULL;
static pthread_key_t g_key;

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:ANDROID"

static jclass
get_class_id(const char *className)
{

  if (!className) {
      return NULL;
  }

  JNIEnv* env = sen_androud_get_env();

  jstring jsClassName = env->NewStringUTF(className);

  jclass jcls = (jclass) env->CallObjectMethod(g_class_loader,
                                               g_load_class_method_id,
                                               jsClassName);

  if (!jcls) {
    _logfe("Class [%s] not found", className);
    env->ExceptionClear();
  }

  env->DeleteLocalRef(jsClassName);

  return jcls;
}


void
sen_android_set_javavm(JavaVM *javaVM)
{
  g_java_vm = javaVM;
  pthread_key_create(&g_key, NULL);
}

JavaVM*
sen_androud_get_javavm()
{
  return g_java_vm;
}

static JNIEnv* cache_env(JavaVM* jvm)
{
  JNIEnv* _env = NULL;
  jint ret = jvm->GetEnv((void**)&_env, JNI_VERSION_1_4);
  switch (ret) {
  case JNI_OK :
      pthread_setspecific(g_key, _env);
      return _env;
  case JNI_EDETACHED :
      if (jvm->AttachCurrentThread(&_env, NULL) < 0) {
        _logfe("AttachCurrentThread() failed");
        return NULL;
      }
      else {
          pthread_setspecific(g_key, _env);
          return _env;
      }
  case JNI_EVERSION :
      _logfe("jni ver 1.4 not supported");
      return NULL;
  default :
      _logfe("GetEnv() failed");
      return NULL;
  }
}


JNIEnv*
sen_androud_get_env()
{
  JNIEnv *_env = (JNIEnv *)pthread_getspecific(g_key);
  if (!_env)
    _env = cache_env(g_java_vm);
  return _env;
}

int
get_default_class_loader(jni_method_t *methodinfo,
                         const char   *className,
                         const char   *methodName,
                         const char   *paramCode)
{
    if ( !className || !methodName || !paramCode) {
      return 0;
    }

    JNIEnv *env = sen_androud_get_env();
    if (!env) {
      return 0;
    }

    jclass classID = env->FindClass(className);
    if (! classID) {
      _logfe("class [%s] not found", className);
      env->ExceptionClear();
      return 0;
    }

    jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
    if (! methodID) {
      _logfe("method id of [%s] not found", methodName);
      env->ExceptionClear();
      return 0;
    }

    methodinfo->classID = classID;
    methodinfo->env = env;
    methodinfo->methodID = methodID;

    return 1;
}


int
sen_android_set_classLoader(jobject activityInstance)
{

  jni_method_t _getclassloaderMethod;
  if (!get_default_class_loader(&_getclassloaderMethod,
                                "android/content/Context",
                                "getClassLoader",
                                "()Ljava/lang/ClassLoader;"))
  {
    return 0;
  }

  jobject _c = sen_androud_get_env()->CallObjectMethod(activityInstance,
                                                       _getclassloaderMethod.methodID);

  if (!_c) {
   return 0;
  }

  jni_method_t _m;
  if (!get_default_class_loader(&_m,
                                "java/lang/ClassLoader",
                                "loadClass",
                                "(Ljava/lang/String;)Ljava/lang/Class;"))
  {
    return 0;
  }

  g_class_loader = sen_androud_get_env()->NewGlobalRef(_c);
  g_load_class_method_id = _m.methodID;

  return 1;
}

int
sen_android_get_static_method(jni_method_t *methodinfo,
                              const char *className,
                              const char *methodName,
                              const char *paramCode)
{
  if (!className || !methodName || !paramCode) {
    return 0;
  }

  JNIEnv *env = sen_androud_get_env();
  if (!env) {
    _logfe("Failed to get env");
    return 0;
  }

  jclass classID = get_class_id(className);
  if (! classID) {
    _logfe("class [%s] not found", className);
    env->ExceptionClear();
    return 0;
  }

  jmethodID methodID = env->GetStaticMethodID(classID, methodName, paramCode);
  if (! methodID) {
    _logfe("static method id of [%s] not found", methodName);
    env->ExceptionClear();
    return 0;
  }

  methodinfo->classID = classID;
  methodinfo->env = env;
  methodinfo->methodID = methodID;
  return 1;
}

int
sen_android_get_method(jni_method_t *methodinfo,
                       const char *className,
                       const char *methodName,
                       const char *paramCode)
{
  if (!className || !methodName || !paramCode)
  {
    return 0;
  }

  JNIEnv *env = sen_androud_get_env();
  if (!env) {
    return 0;
  }

  jclass classID = get_class_id(className);
  if (! classID)
  {
    _logfi("class [%s] not Found", className);
    env->ExceptionClear();
    return 0;
  }

  jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
  if (! methodID)
  {
    _logfe("method id of [%s] not Found", methodName);
    env->ExceptionClear();
    return 0;
  }

  methodinfo->classID = classID;
  methodinfo->env = env;
  methodinfo->methodID = methodID;

  return 1;
}

char*
sen_jstring_dup(jstring str)
{
  if (!str)
  {
    return NULL;
  }

  JNIEnv *env = sen_androud_get_env();
  if (!env) {
    return NULL;
  }

  const char* chars = env->GetStringUTFChars(str, NULL);
  char* ret = sen_strdup(chars);
  env->ReleaseStringUTFChars(str, chars);

  return ret;
}
//--------------------------------------------------------------------------------------------------------
int
sen_platform_dpi()
{

  jni_method_t  t;
  jint ret = -1;
  if (sen_android_get_static_method(&t, "org/sen/lib/SENHandler", "getDPI", "()I"))
  {
    ret = t.env->CallStaticIntMethod(t.classID, t.methodID);
    t.env->DeleteLocalRef(t.classID);
  }
  return ret;

  //return 1111111;
}

static const char* size_names[] = {
    "small", "normal", "large", "xlarge",
};

const char*
sen_platform_screen_size_name()
{
  jni_method_t  t;
  jint id = 0;
  if (sen_android_get_static_method(&t, "org/sen/lib/SENHandler", "getSizeIdent", "()I"))
  {
    id = t.env->CallStaticIntMethod(t.classID, t.methodID);
    t.env->DeleteLocalRef(t.classID);
  }


  return size_names[id];
}

static const char PLATFORM_NAME[] = "Android/OpenGL ES 2.0";
const char*
sen_platform_name()
{
  return PLATFORM_NAME;
}

void sen_exit()
{
  jni_method_t  t;
  if (sen_android_get_static_method(&t, "org/sen/lib/SENHandler", "doExit", "()V"))
  {
    t.env->CallStaticVoidMethod(t.classID, t.methodID);
    t.env->DeleteLocalRef(t.classID);
  }
}
