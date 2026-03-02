#include "AndroidPlatformAdapter.h"

#include "../input/Multitouch.h"

AndroidPlatformAdapter::AndroidPlatformAdapter()
    : _activity(0)
{
}

int AndroidPlatformAdapter::onLoad(JavaVM* vm)
{
    return _appPlatform.init(vm);
}

void AndroidPlatformAdapter::registerActivity(JNIEnv* env, jobject activity)
{
    _activity = (jobject)env->NewGlobalRef(activity);
}

void AndroidPlatformAdapter::unregisterActivity(JNIEnv* env)
{
    if (_activity) {
        env->DeleteGlobalRef(_activity);
        _activity = 0;
    }
}

void AndroidPlatformAdapter::bindToContext(AppContext& context)
{
    _appPlatform.instance = _activity;
    _appPlatform.initConsts();
    context.platform = &_appPlatform;
}

AppPlatform_android& AndroidPlatformAdapter::appPlatform()
{
    return _appPlatform;
}

void AndroidPlatformAdapter::configureStoragePaths(JNIEnv* env, App* app)
{
    if (!env || !app) {
        return;
    }
    jclass clazz = env->FindClass("android/os/Environment");
    jmethodID method = env->GetStaticMethodID(clazz, "getExternalStorageDirectory", "()Ljava/io/File;");

    jobject file = env->CallStaticObjectMethod(clazz, method);
    jclass fileClass = env->GetObjectClass(file);
    jmethodID fileMethod = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jobject pathString = env->CallObjectMethod(file, fileMethod);

    const char* str = env->GetStringUTFChars((jstring)pathString, NULL);
    app->externalStoragePath = str;
    app->externalCacheStoragePath = str;
    env->ReleaseStringUTFChars((jstring)pathString, str);
}

void AndroidPlatformAdapter::pointerDown(int pointerId, int x, int y)
{
    Multitouch::feed(1, 1, x, y, pointerId);
}

void AndroidPlatformAdapter::pointerUp(int pointerId, int x, int y)
{
    Multitouch::feed(1, 0, x, y, pointerId);
}

void AndroidPlatformAdapter::pointerMove(int pointerId, int x, int y)
{
    Multitouch::feed(0, 0, x, y, pointerId);
}
