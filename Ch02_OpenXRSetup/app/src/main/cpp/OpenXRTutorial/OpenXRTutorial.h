#pragma once

#include <GraphicsAPI.h>

#if defined(__ANDROID__)
#include <android_native_app_glue.h>
#endif

class OpenXRTutorial
{
  public:
    OpenXRTutorial(GraphicsAPI_Type apiType);
    ~OpenXRTutorial();

    void Run();

#if defined(__ANDROID__)
  public:
    static android_app *androidApp;

    struct AndroidAppState
    {
        ANativeWindow *nativeWindow = nullptr;
        bool resumed = false;
    };
    static AndroidAppState androidAppState;

    static void AndroidAppHandleCmd(struct android_app *app, int32_t cmd);
#endif

  private:
    void PollSystemEvents();

  private:
    bool m_applicationRunning = true;
    bool m_sessionRunning = false;
    GraphicsAPI_Type m_apiType;
};
