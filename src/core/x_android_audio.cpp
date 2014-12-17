extern "C" {
#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "audio.h"
}
#include "x_android.h"


static int get_static_method(jni_method_t *methodinfo,
                             const char *methodName,
                             const char *paramCode)
{
  return
  sen_android_get_static_method(methodinfo,
                                "org/sen/lib/SENHandler",
                                methodName,
                                paramCode);
}

void
sen_music_preload(const char* path)
{
  jni_method_t method;

  if (!get_static_method(&method, "preloadMusic", "(Ljava/lang/String;)V"))
    return ;

  jstring Arg = method.env->NewStringUTF(path);
  method.env->CallStaticVoidMethod(method.classID, method.methodID, Arg);
  method.env->DeleteLocalRef(Arg);
  method.env->DeleteLocalRef(method.classID);

}

void
sen_music_play(const char* path, int loop)
{
  jni_method_t method;

  if (!get_static_method(&method, "playMusic", "(Ljava/lang/String;Z)V"))
    return ;

  jstring Arg = method.env->NewStringUTF(path);
  method.env->CallStaticVoidMethod(method.classID, method.methodID, Arg,(bool)loop);
  method.env->DeleteLocalRef(Arg);
  method.env->DeleteLocalRef(method.classID);

}

void
sen_music_stop(int release)
{
  UNUSED(release);
  jni_method_t method;

  if (!get_static_method(&method, "stopMusic", "()V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);
}

void
sen_music_pause()
{
  jni_method_t method;

  if (!get_static_method(&method, "pauseMusic", "()V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);

}

void
sen_music_resume()
{
  jni_method_t method;

  if (!get_static_method(&method, "resumeMusic", "()V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);
}

void
sen_music_rewind()
{
  jni_method_t method;

   if (!get_static_method(&method, "rewindMusic", "()V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);
}

int
sen_music_is_playing()
{
  jboolean ret = false;
  jni_method_t method;

  if (!get_static_method(&method, "isMusicPlaying", "()Z"))
   return false;

  ret = method.env->CallStaticBooleanMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);
  return (int)ret;
}

float
sen_music_get_vol()
{
  jfloat ret = -1.0f;
  jni_method_t method;

  if (!get_static_method(&method, "getMusicVol", "()F"))
   return ret;

  ret = method.env->CallStaticFloatMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);
  return ret;
}

void
sen_music_set_vol(float volume)
{
  jni_method_t method;

  if (!get_static_method(&method, "setMusicVol", "(F)V"))
   return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID, volume);
  method.env->DeleteLocalRef(method.classID);
}
//-------------------------------------------------------------------------------------------------------
void
sen_sound_preload(const char* path)
{
  jni_method_t method;

  if (!get_static_method(&method, "preloadSound", "(Ljava/lang/String;)V"))
    return ;

  jstring Arg = method.env->NewStringUTF(path);
  method.env->CallStaticVoidMethod(method.classID, method.methodID, Arg);
  method.env->DeleteLocalRef(Arg);
  method.env->DeleteLocalRef(method.classID);
}

void
sen_sound_unload(const char* path)
{
  jni_method_t method;

  if (!get_static_method(&method, "unloadSound", "(Ljava/lang/String;)V"))
    return ;

  jstring Arg = method.env->NewStringUTF(path);
  method.env->CallStaticVoidMethod(method.classID, method.methodID, Arg);
  method.env->DeleteLocalRef(Arg);
  method.env->DeleteLocalRef(method.classID);
}

unsigned int
sen_sound_play(const char* path)
{
  return sen_sound_play_ex(path, 0, 1.0f, 0.0f, 1.0f);
}

unsigned int
sen_sound_play_ex(const char* path,
                  int   loop,
                  float pitch,
                  float pan,
                  float gain)
{
  jni_method_t method;
  unsigned int ret = 0;
  if (!get_static_method(&method, "playSound", "(Ljava/lang/String;ZFFF)I"))
    return ret;

  jstring Arg = method.env->NewStringUTF(path);
  ret = (unsigned int)method.env->CallStaticIntMethod(method.classID, method.methodID, Arg,(bool)loop, pitch, pan, gain);
  method.env->DeleteLocalRef(Arg);
  method.env->DeleteLocalRef(method.classID);

  return ret;
}

void
sen_sound_pause(unsigned int id)
{
  jni_method_t method;

  if (!get_static_method(&method, "pauseSound", "(I)V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID, (int)id);
  method.env->DeleteLocalRef(method.classID);
}

void
sen_sound_pause_all()
{
  jni_method_t method;

  if (!get_static_method(&method, "pauseAllSounds", "()V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);

}

void
sen_sound_resume(unsigned int id)
{
  jni_method_t method;

  if (!get_static_method(&method, "resumeSound", "(I)V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID, (int)id);
  method.env->DeleteLocalRef(method.classID);
}

void
sen_sound_resume_all()
{
  jni_method_t method;

  if (!get_static_method(&method, "resumeAllSounds", "()V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);

}

void
sen_sound_stop(unsigned int id)
{
  jni_method_t method;

  if (!get_static_method(&method, "stopSound", "(I)V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID, (int)id);
  method.env->DeleteLocalRef(method.classID);

}

void
sen_sound_stop_all()
{
  jni_method_t method;

  if (!get_static_method(&method, "stopAllSounds", "()V"))
    return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);
}

float
sen_sound_get_vol()
{
  jfloat ret = -1.0f;
  jni_method_t method;

  if (!get_static_method(&method, "getSoundsVol", "()F"))
   return ret;

  ret = method.env->CallStaticFloatMethod(method.classID, method.methodID);
  method.env->DeleteLocalRef(method.classID);
  return ret;
}

void
sen_sound_set_vol(float volume)
{
  jni_method_t method;

  if (!get_static_method(&method, "setSoundsVol", "(F)V"))
   return ;

  method.env->CallStaticVoidMethod(method.classID, method.methodID, volume);
  method.env->DeleteLocalRef(method.classID);
}
//-------------------------------------------------------------------------------------------------------

void
sen_audio_init()
{

}

void
sen_audio_destroy()
{
  jni_method_t methodInfo;

  if (!get_static_method(&methodInfo, "end", "()V")) {
    return ;
  }

  methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID);
  methodInfo.env->DeleteLocalRef(methodInfo.classID);
}

void
sen_audio_update()
{

}
