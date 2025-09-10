#include <DebugOutput.h>
#include "Application/OpenXRTutorial.h"

static void OpenXRTutorial_Main(GraphicsAPI_Type apiType)
{
    XR_TUT_LOG("OpenXR Tutorial Ch00_BaseProject");

    XR_TUT_LOG("Graphics API Type: " << apiType);

    OpenXRTutorial app(apiType);
    app.Run();
}

#if defined(__ANDROID__)
void android_main(struct android_app *app)
{
    JNIEnv *env;
    app->activity->vm->AttachCurrentThread(&env, nullptr);

    app->userData = &OpenXRTutorial::androidAppState;
    app->onAppCmd = OpenXRTutorial::AndroidAppHandleCmd;
    OpenXRTutorial::androidApp = app;
    OpenXRTutorial_Main(VULKAN);
}
#else
int main(int argc, char** argv)
{
    OpenXRTutorial_Main(VULKAN);
    return 0;
}
#endif

