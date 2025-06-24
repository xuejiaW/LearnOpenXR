#pragma once

#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>
#include <openxr/openxr.h>
#include "../OpenXR/OpenXRDisplay/RenderLayerInfo.h"
#include "TableFloorScene.h"
#include "OpenXRRenderer.h"

#if defined(__ANDROID__)
#include <android_native_app_glue.h>
#endif

class OpenXRTutorial
{
public:
    OpenXRTutorial(GraphicsAPI_Type apiType);
    ~OpenXRTutorial();

    void Run();

    static GraphicsAPI_Type m_apiType;

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
    void InitializeOpenXR();
    void ShutdownOpenXR();

    OpenXRRenderer* m_renderer = nullptr;
    TableFloorScene* m_scene = nullptr;
};

