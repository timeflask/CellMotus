extern "C" {

#include "sen.h"

static const void* signal_toBG         = NULL;
static const void* signal_toFG         = NULL;
static const void* signal_reload       = NULL;
static const void* signal_keyDown      = NULL;
static const void* signal_sensor       = NULL;
static const void* signal_resize       = NULL;
static const void* signal_touchesBegin = NULL;
static const void* signal_touchesEnd   = NULL;
static const void* signal_touchesMove  = NULL;
static const void* signal_touchesCancel= NULL;

}

#include <jni.h>
#include "x_android.h"
#include <android/asset_manager_jni.h>


extern "C" {


JNIEXPORT void JNICALL Java_org_sen_lib_SENRenderer_nativeInit
  (JNIEnv*  env, jclass cls, jint w, jint h)
{
	UNUSED(env); UNUSED(cls); UNUSED(w); UNUSED(h);
	int status = sen_status();

	if (status == SEN_STATUS_DEAD) {

    sen_init((float)w,(float)h);

    signal_toBG           = sen_signal_get_name("enterBackground", "platform");
    signal_toFG           = sen_signal_get_name("enterForeground", "platform");
    signal_reload         = sen_signal_get_name("reload",          "platform");
    signal_keyDown        = sen_signal_get_name("keyDown",         "platform");
    signal_sensor         = sen_signal_get_name("sensor",          "platform");
    signal_resize         = sen_signal_get_name("resize",          "platform");
    signal_touchesBegin   = sen_signal_get_name("touchesBegin",    "platform");
    signal_touchesEnd     = sen_signal_get_name("touchesEnd",      "platform");
    signal_touchesMove    = sen_signal_get_name("touchesMove",     "platform");
    signal_touchesCancel  = sen_signal_get_name("touchesCancel",   "platform");
	}
	else
	  sen_signal_emit( signal_reload, NULL );
}

JNIEXPORT void JNICALL Java_org_sen_lib_SENRenderer_nativeResize
  (JNIEnv * env, jclass cls, jint w, jint h)
{
	UNUSED(env); UNUSED(cls);
	if (signal_resize) {
	  vec2 size = {{(float)w, (float)h}};
	  sen_signal_emit( signal_resize, & size );
	}
}

JNIEXPORT void JNICALL Java_org_sen_lib_SENRenderer_nativeRender
  (JNIEnv * env, jclass cls)
{
  UNUSED(env); UNUSED(cls);

  sen_process();
}

JNIEXPORT void JNICALL Java_org_sen_lib_SENActivity_nativeInitAssets
(JNIEnv * env, jclass cls, jobject java_asset_manager)
{
  UNUSED(cls);
  g_AssetManager = AAssetManager_fromJava(env, java_asset_manager);
}

JNIEXPORT void JNICALL Java_org_sen_lib_SENActivity_nativeInitContext
(JNIEnv * env, jclass cls, jobject context)
{
  UNUSED(env);UNUSED(cls);
  sen_android_set_classLoader(context);
}

JNIEXPORT void Java_org_sen_lib_SENRenderer_nativeSuspend
(JNIEnv * env, jclass cls)
{
  UNUSED(env); UNUSED(cls);
  if (signal_toBG)
    sen_signal_emit( signal_toBG, NULL );

}

JNIEXPORT void JNICALL Java_org_sen_lib_SENRenderer_nativeResume
(JNIEnv * env, jclass cls)
{
  UNUSED(env); UNUSED(cls);
  if (signal_toFG)
    sen_signal_emit( signal_toFG, NULL );
}

JNIEXPORT void JNICALL Java_org_sen_lib_SENRenderer_nativeKeyDown
(JNIEnv * env, jclass cls, jint keyCode) {
  UNUSED(env); UNUSED(cls);
  if (signal_keyDown) {
    int kc = keyCode;
    sen_signal_emit( signal_keyDown, &kc );
  }
}


JNIEXPORT void JNICALL Java_org_sen_lib_SENSensor_nativeSensorChanged
(JNIEnv*  env, jclass cls, jfloat x, jfloat y, jfloat z, jlong timeStamp) {
  UNUSED(env); UNUSED(cls);

  if (signal_sensor) {
    const float gravity = 9.80665f;

    vec4 accel = {{ -((float)x / gravity),
                    -((float)y / gravity),
                    -((float)z / gravity),
                      (float)timeStamp
    }};

    sen_signal_emit( signal_sensor, &accel );
  }
}

JNIEXPORT void Java_org_sen_lib_SENRenderer_nativeTouchesBegin
(JNIEnv * env, jclass cls, jint id, jfloat x, jfloat y) {
  UNUSED(env); UNUSED(cls);
  if (signal_touchesBegin)
  {
    touch_data_t td = {1, &id, &x, &y};
    sen_signal_emit( signal_touchesBegin, &td );
  }
}

JNIEXPORT void JNICALL Java_org_sen_lib_SENRenderer_nativeTouchesEnd
(JNIEnv * env, jclass cls, jint id, jfloat x, jfloat y) {
  UNUSED(env); UNUSED(cls);
  if (signal_touchesEnd)
  {
    touch_data_t td = {1, &id, &x, &y};
    sen_signal_emit( signal_touchesEnd, &td );
  }
}

JNIEXPORT void JNICALL JNICALL Java_org_sen_lib_SENRenderer_nativeTouchesMove
(JNIEnv * env, jclass cls, jintArray ids, jfloatArray xs, jfloatArray ys) {
  UNUSED(env); UNUSED(cls);
  if (signal_touchesMove)
  {
    int size = env->GetArrayLength(ids);
    jint   id[size];
    jfloat x[size];
    jfloat y[size];
    env->GetIntArrayRegion(ids, 0, size, id);
    env->GetFloatArrayRegion(xs, 0, size, x);
    env->GetFloatArrayRegion(ys, 0, size, y);
    touch_data_t td = {size, id, x, y};
    sen_signal_emit( signal_touchesMove, &td );
  }
}

JNIEXPORT void JNICALL Java_org_sen_lib_SENRenderer_nativeTouchesCancel
(JNIEnv * env, jclass cls, jintArray ids, jfloatArray xs, jfloatArray ys) {
  UNUSED(env); UNUSED(cls);
  if (signal_touchesCancel)
  {
    int size = env->GetArrayLength(ids);
    jint   id[size];
    jfloat x[size];
    jfloat y[size];
    env->GetIntArrayRegion(ids, 0, size, id);
    env->GetFloatArrayRegion(xs, 0, size, x);
    env->GetFloatArrayRegion(ys, 0, size, y);
    touch_data_t td = {size, id, x, y};
    sen_signal_emit( signal_touchesCancel, &td );
  }
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
  UNUSED(reserved);
  sen_android_set_javavm(vm);
  return JNI_VERSION_1_4;
}

} // extern C
