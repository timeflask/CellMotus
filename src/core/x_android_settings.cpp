extern "C" {
  #include "config.h"
  #include "asset.h"
  #include "macro.h"
  #include "utils.h"
  #include "logger.h"
  #include "settings.h"
}
#include "x_android.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Settings"

char*
sen_settings_get_str(const char* key, const char* defaultValue)
{
  jni_method_t t;

  if (!sen_android_get_static_method(&t,
                                     "org/sen/lib/SENHandler",
                                     "getKeyString",
                                     "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"))
    return sen_strdup(defaultValue);


  jstring stringArg1 = t.env->NewStringUTF(key);
  jstring stringArg2 = t.env->NewStringUTF(defaultValue);
  jstring str = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID, stringArg1, stringArg2);

  char* ret = sen_jstring_dup(str);

  t.env->DeleteLocalRef(t.classID);
  t.env->DeleteLocalRef(stringArg1);
  t.env->DeleteLocalRef(stringArg2);
  t.env->DeleteLocalRef(str);

  return ret;
}

void
sen_settings_set_str(const char* key, const char* value)
{
  jni_method_t t;

  if (!sen_android_get_static_method(&t,
                                     "org/sen/lib/SENHandler",
                                     "setKeyString",
                                     "(Ljava/lang/String;Ljava/lang/String;)V"))
    return ;


  jstring stringArg1 = t.env->NewStringUTF(key);
  jstring stringArg2 = t.env->NewStringUTF(value);
  t.env->CallStaticVoidMethod(t.classID, t.methodID, stringArg1, stringArg2);

  t.env->DeleteLocalRef(t.classID);
  t.env->DeleteLocalRef(stringArg1);
  t.env->DeleteLocalRef(stringArg2);
}

