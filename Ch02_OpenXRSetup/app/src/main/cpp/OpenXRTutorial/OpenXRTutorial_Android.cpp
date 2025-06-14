#include "OpenXRTutorial.h"
#include <DebugOutput.h> // For XR_TUTORIAL_GRAPHICS_API
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h> // For XrLoaderInitInfoAndroidKHR

// Forward declaration from main.cpp
// This might need to be adjusted if OpenXRTutorial_Main is also moved or its declaration changes.
void OpenXRTutorial_Main(GraphicsAPI_Type apiType);

#if defined(__ANDROID__)
android_app *OpenXRTutorial::androidApp = nullptr;
OpenXRTutorial::AndroidAppState OpenXRTutorial::androidAppState = {};

void OpenXRTutorial::AndroidAppHandleCmd(struct android_app *app, int32_t cmd) {
    AndroidAppState *appState = (AndroidAppState *)app->userData;

    switch (cmd) {
    case APP_CMD_START: {
        break;
    }
    case APP_CMD_RESUME: {
        appState->resumed = true;
        break;
    }
    case APP_CMD_PAUSE: {
        appState->resumed = false;
        break;
    }
    case APP_CMD_STOP: {
        break;
    }
    case APP_CMD_DESTROY: {
        appState->nativeWindow = nullptr;
        break;
    }
    case APP_CMD_INIT_WINDOW: {
        appState->nativeWindow = app->window;
        break;
    }
    case APP_CMD_TERM_WINDOW: {
        appState->nativeWindow = nullptr;
        break;
    }
    }
}

void OpenXRTutorial::PollSystemEvents() {
    if (androidApp->destroyRequested != 0) {
        m_applicationRunning = false;
        return;
    }
    while (true) {
        struct android_poll_source *source = nullptr;
        int events = 0;
        const int timeoutMilliseconds = (!androidAppState.resumed && !m_sessionRunning && androidApp->destroyRequested == 0) ? -1 : 0;
        if (ALooper_pollOnce(timeoutMilliseconds, nullptr, &events, (void **)&source) >= 0) {
            if (source != nullptr) {
                source->process(androidApp, source);
            }
        } else {
            break;
        }
    }
}

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
    OpenXRTutorial_Main(XR_TUTORIAL_GRAPHICS_API); // XR_TUTORIAL_GRAPHICS_API needs to be defined
}
#endif
