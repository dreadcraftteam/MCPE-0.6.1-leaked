#include "App.h"
#include "NinecraftApp.h"
#include "AppPlatform.h"
#include "platform/time.h"

#define MAIN_CLASS NinecraftApp

int main(int argc, char** argv)
{
    AppContext appContext;
    appContext.doRender = false;
    appContext.platform = new AppPlatform();

    App* app = new MAIN_CLASS();
    app->init(appContext);

    while (!app->wantToQuit()) {
        app->update();
        sleepMs(16);
    }

    delete app;
    appContext.platform->finish();
    delete appContext.platform;
    return 0;
}
