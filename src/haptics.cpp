// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "haptics.hpp"

#include <SDL3/SDL.h>

#ifdef SDL_PLATFORM_ANDROID

#include <jni.h>

bool haptics_supported()
{
    return true;
}

void haptics_pulse(Pulse pulse)
{
    JNIEnv* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    jobject activity = static_cast<jobject>(SDL_GetAndroidActivity());
    if (!env || !activity) {
        return;
    }
    jclass activity_class = env->GetObjectClass(activity);
    jmethodID get_service =
        env->GetMethodID(activity_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring name = env->NewStringUTF("vibrator");
    jobject vibrator = env->CallObjectMethod(activity, get_service, name);
    if (vibrator && !env->ExceptionCheck()) {
        jclass vibrator_class = env->GetObjectClass(vibrator);
        if (SDL_GetAndroidSDKVersion() >= 26) {
            jclass effect_class = env->FindClass("android/os/VibrationEffect");
            jmethodID create = env->GetStaticMethodID(effect_class, "createOneShot", "(JI)Landroid/os/VibrationEffect;");
            jobject effect = env->CallStaticObjectMethod(effect_class, create, static_cast<jlong>(pulse.ms),
                                                         static_cast<jint>(pulse.amplitude));
            jmethodID vibrate = env->GetMethodID(vibrator_class, "vibrate", "(Landroid/os/VibrationEffect;)V");
            env->CallVoidMethod(vibrator, vibrate, effect);
            env->DeleteLocalRef(effect);
            env->DeleteLocalRef(effect_class);
        } else {
            jmethodID vibrate = env->GetMethodID(vibrator_class, "vibrate", "(J)V");
            env->CallVoidMethod(vibrator, vibrate, static_cast<jlong>(pulse.ms));
        }
        env->DeleteLocalRef(vibrator_class);
    }
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
    env->DeleteLocalRef(vibrator);
    env->DeleteLocalRef(name);
    env->DeleteLocalRef(activity_class);
}

#else

bool haptics_supported()
{
    return false;
}

void haptics_pulse(Pulse)
{
}

#endif
