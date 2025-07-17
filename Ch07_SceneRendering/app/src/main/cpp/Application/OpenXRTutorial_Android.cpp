#include "OpenXRTutorial.h"
#include <DebugOutput.h> // For XR_TUTORIAL_GRAPHICS_API
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h> // For XrLoaderInitInfoAndroidKHR

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
        return;
    }
    while (true) {
        struct android_poll_source *source = nullptr;
        int events = 0;
        const int timeoutMilliseconds = (!androidAppState.resumed && androidApp->destroyRequested == 0) ? -1 : 0;
        if (ALooper_pollOnce(timeoutMilliseconds, nullptr, &events, (void **)&source) >= 0) {
            if (source != nullptr) {
                source->process(androidApp, source);
            }
        } else {
            break;
        }
    }
}
#endif