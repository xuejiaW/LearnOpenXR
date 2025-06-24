#include <DebugOutput.h>
#include "Application/OpenXRTutorial.h"

static void OpenXRTutorial_Main(GraphicsAPI_Type apiType)
{
    XR_TUT_LOG("OpenXR Tutorial Ch03_Graphics");

    XR_TUT_LOG("Graphics API Type: " << apiType);

    OpenXRTutorial app(apiType);
    app.Run();
}


#if defined(__ANDROID__)
void android_main(struct android_app *app) {
    JNIEnv *env;
    app->activity->vm->AttachCurrentThread(&env, nullptr);

    // It's good practice to ensure OpenXR loader is initialized for Android.
    PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
    // Using XR_NULL_HANDLE for the first parameter as we don't have an instance yet.
    if (XR_FAILED(xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction *)&xrInitializeLoaderKHR))) {
        XR_TUT_LOG_ERROR("Failed to get xrInitializeLoaderKHR function pointer.");
        return;
    }
    if (!xrInitializeLoaderKHR) {
        XR_TUT_LOG_ERROR("xrInitializeLoaderKHR function pointer is null.");
        return;
    }

    XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid{XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR};
    loaderInitializeInfoAndroid.applicationVM = app->activity->vm;
    loaderInitializeInfoAndroid.applicationContext = app->activity->clazz;
    if (XR_FAILED(xrInitializeLoaderKHR((const XrLoaderInitInfoBaseHeaderKHR *)&loaderInitializeInfoAndroid))) {
        XR_TUT_LOG_ERROR("Failed to initialize OpenXR loader for Android.");
        return;
    }

    app->userData = &OpenXRTutorial::androidAppState;
    app->onAppCmd = OpenXRTutorial::AndroidAppHandleCmd;

    OpenXRTutorial::androidApp = app;
    OpenXRTutorial_Main(XR_TUTORIAL_GRAPHICS_API); // XR_TUTORIAL_GRAPHICS_API defined by cmake
}
#else
int main(int argc, char** argv)
{
    OpenXRTutorial_Main(XR_TUTORIAL_GRAPHICS_API); // XR_TUTORIAL_GRAPHICS_API defined by cmake
    return 0;
}
#endif