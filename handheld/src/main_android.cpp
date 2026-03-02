#include "App.h"
#include "platform/android/AndroidPlatformAdapter.h"

// Horrible, I know. / A
#ifndef MAIN_CLASS
#include "main.cpp"
#endif

#include <pthread.h>

// References for JNI
static pthread_mutex_t g_activityMutex = PTHREAD_MUTEX_INITIALIZER;
static AndroidPlatformAdapter gPlatform;

extern "C" {
JNIEXPORT jint JNICALL
	JNI_OnLoad( JavaVM * vm, void * reserved )
	{
		pthread_mutex_init(&g_activityMutex, 0);
		pthread_mutex_lock(&g_activityMutex);

		LOGI("Entering OnLoad %d\n", pthread_self());
		return gPlatform.onLoad(vm);
	}

	// Register/save a reference to the java main activity instance
	JNIEXPORT void JNICALL
	Java_com_mojang_minecraftpe_MainActivity_nativeRegisterThis(JNIEnv* env, jobject clazz) {
		LOGI("@RegisterThis %d\n", pthread_self());
		gPlatform.registerActivity(env, clazz);

		pthread_mutex_unlock(&g_activityMutex);
	}

	// Unregister/delete the reference to the java main activity instance
	JNIEXPORT void JNICALL
	Java_com_mojang_minecraftpe_MainActivity_nativeUnregisterThis(JNIEnv* env, jobject clazz) {
		LOGI("@UnregisterThis %d\n", pthread_self());
		gPlatform.unregisterActivity(env);

		pthread_mutex_destroy(&g_activityMutex);
	}

	JNIEXPORT void JNICALL
	Java_com_mojang_minecraftpe_MainActivity_nativeStopThis(JNIEnv* env, jobject clazz) {
			LOGI("Lost Focus!");
	}
}

static void internal_process_input(struct android_app* app, struct android_poll_source* source) {
	AInputEvent* event = NULL;
	if (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
		LOGV("New input event: type=%d\n", AInputEvent_getType(event));
		bool isBackButtonDown = AKeyEvent_getKeyCode(event) == AKEYCODE_BACK && AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN;
		if(!(gPlatform.appPlatform().isKeyboardVisible() && isBackButtonDown)) {
			if (AInputQueue_preDispatchEvent(app->inputQueue, event)) {
				return;
			}
		}
		int32_t handled = 0;
		if (app->onInputEvent != NULL) handled = app->onInputEvent(app, event);
		AInputQueue_finishEvent(app->inputQueue, event, handled);
	} else {
		LOGE("Failure reading next input event: %s\n", strerror(errno));
	}
}

void
android_main( struct android_app* state )
{
    struct ENGINE engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset( (void*)&engine, 0, sizeof(engine) );
    state->userData     = (void*)&engine;
    state->onAppCmd     = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    state->destroyRequested = 0;

    pthread_mutex_lock(&g_activityMutex);
    pthread_mutex_unlock(&g_activityMutex);

    //LOGI("socket-stuff\n");
    //socketDesc = initSocket(1999);

    App* app = new MAIN_CLASS();

    engine.userApp      = app;
    engine.app          = state;
    engine.is_inited    = false;
    engine.appContext.doRender = true;
    gPlatform.bindToContext(engine.appContext);

    JNIEnv* env = state->activity->env;
    state->activity->vm->AttachCurrentThread(&env, NULL);
    gPlatform.configureStoragePaths(env, app);
    state->activity->vm->DetachCurrentThread();

    if( state->savedState != NULL )
    {
        // We are starting with a previous saved state; restore from it.
       app->loadState(state->savedState, state->savedStateSize);
    }

    bool inited = false;
    bool teardownPhase = false;
	gPlatform.appPlatform()._nativeActivity = state->activity;
    // our 'main loop'
    while( 1 )
    {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;
		
        while( (ident = ALooper_pollAll( 0, NULL, &events, (void**)&source) ) >= 0 )
        {
            // Process this event.
            // This will call the function pointer android_app::onInputEvent() which in our case is
            // engine_handle_input()
            if( source != NULL ) {
                if(source->id == 2) {
					// Back button is intercepted by the ime on android 4.1/4.2 resulting in the application stopping to respond.
					internal_process_input( state, source );
				} else {
					source->process( state, source );
				}
            }

        }
         // Check if we are exiting.
         if( state->destroyRequested )
         {
             //engine_term_display( &engine );
             delete app;
             return;
         }

		 if (!inited && engine.is_inited) {
			 app->init(engine.appContext);
			 app->setSize(engine.width, engine.height);
			 inited = true;
		 }

        if (inited && engine.is_inited && engine.has_focus) {
            app->update();
        } else {
            sleepMs(50);
        }    

        if (!teardownPhase && app->wantToQuit()) {
            teardownPhase = true;
            LOGI("tearing down!");
            ANativeActivity_finish(state->activity);
        }
    }
}
