#pragma once

#include <jni.h>

#include "../../App.h"
#include "../../AppPlatform_android.h"
#include "../Platform.h"

class AndroidPlatformAdapter : public Platform
{
public:
    AndroidPlatformAdapter();

    int onLoad(JavaVM* vm);
    void registerActivity(JNIEnv* env, jobject activity);
    void unregisterActivity(JNIEnv* env);

    void bindToContext(AppContext& context);
    AppPlatform_android& appPlatform();

    void configureStoragePaths(JNIEnv* env, App* app);
    virtual void pointerDown(int pointerId, int x, int y);
    virtual void pointerUp(int pointerId, int x, int y);
    virtual void pointerMove(int pointerId, int x, int y);

private:
    jobject _activity;
    AppPlatform_android _appPlatform;
};
