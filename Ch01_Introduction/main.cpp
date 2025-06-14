#define XR_DOCS_CHAPTER_VERSION XR_DOCS_CHAPTER_1_4

#include <DebugOutput.h>
#include <GraphicsAPI_D3D11.h>
#include <GraphicsAPI_D3D12.h>
#include <GraphicsAPI_OpenGL.h>
#include <GraphicsAPI_OpenGL_ES.h>
#include <GraphicsAPI_Vulkan.h>
#include <OpenXRDebugUtils.h>

class OpenXRTutorial {
public:
    OpenXRTutorial(GraphicsAPI_Type apiType) {
    }
    ~OpenXRTutorial() = default;

    void Run() {
    }

#if defined(__ANDROID__)
public:
    static android_app *androidApp;

    struct AndroidAppState {
        ANativeWindow *nativeWindow = nullptr;
        bool resumed = false;
    };
    static AndroidAppState androidAppState;

    static void AndroidAppHandleCmd(struct android_app *app, int32_t cmd) {
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

private:
    void PollSystemEvents() {
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
#else
    void PollSystemEvents() {
        return;
    }
#endif

private:
    bool m_applicationRunning = true;
    bool m_sessionRunning = false;
};

void OpenXRTutorial_Main(GraphicsAPI_Type apiType) {
    DebugOutput debugOutput;  // This redirects std::cerr and std::cout to the IDE's output or Android Studio's logcat.
    XR_TUT_LOG("OpenXR Tutorial Chapter 1");

    OpenXRTutorial app(apiType);
    app.Run();
}

#if !defined(__ANDROID__) && defined(_WIN32)
int main(int argc, char **argv) {
    OpenXRTutorial_Main(XR_TUTORIAL_GRAPHICS_API);
}
#elif (__ANDROID__)
// XR_DOCS_TAG_BEGIN_android_main___ANDROID__
android_app *OpenXRTutorial::androidApp = nullptr;
OpenXRTutorial::AndroidAppState OpenXRTutorial::androidAppState = {};

void android_main(struct android_app *app) {
    JNIEnv *env;
    app->activity->vm->AttachCurrentThread(&env, nullptr);

    XrInstance m_xrInstance = XR_NULL_HANDLE;
    PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
    OPENXR_CHECK(xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction *)&xrInitializeLoaderKHR), "Failed to get InstanceProcAddr for xrInitializeLoaderKHR.");
    if (!xrInitializeLoaderKHR) {
        return;
    }

    XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid{XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR};
    loaderInitializeInfoAndroid.applicationVM = app->activity->vm;
    loaderInitializeInfoAndroid.applicationContext = app->activity->clazz;
    OPENXR_CHECK(xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR *)&loaderInitializeInfoAndroid), "Failed to initialize Loader for Android.");

    app->userData = &OpenXRTutorial::androidAppState;
    app->onAppCmd = OpenXRTutorial::AndroidAppHandleCmd;

    OpenXRTutorial::androidApp = app;
    OpenXRTutorial_Main(XR_TUTORIAL_GRAPHICS_API);
}
#endif
