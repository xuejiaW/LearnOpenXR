#pragma once

#include <GraphicsAPI.h>
#include "../ScenesRendering/TableFloorScene.h"
#include "../ScenesRendering/Scenes/SceneRenderer.h"

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
    void InitializeOpenXR();
    void InitializeSceneRendering();

    void ShutDownOpenXR();
    void PollSystemEvents();
    
    void ProcessInputLogic();
    void ApplyHapticFeedback(int handIndex);

    std::unique_ptr<SceneRenderer> m_sceneRenderer;
    std::shared_ptr<IScene> m_scene;
};