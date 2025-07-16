#include "OpenXRTutorial.h"
#include "../ScenesRendering/TableFloorScene.h"
#include <DebugOutput.h>

#include <GraphicsAPI_Vulkan.h>
#include "../OpenXR/OpenXRCoreMgr.h"
#include "../OpenXR/OpenXRDisplayMgr.h"
#include "../OpenXR/OpenXRSessionMgr.h"

GraphicsAPI_Type OpenXRTutorial::m_apiType = UNKNOWN;

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType)
{
    m_apiType = apiType;
    m_scene = std::make_shared<TableFloorScene>();
    m_renderer = std::make_unique<OpenXRRenderer>(apiType);
}

OpenXRTutorial::~OpenXRTutorial() = default;

void OpenXRTutorial::Run()
{
    InitializeOpenXR();

    m_renderer->SetScene(m_scene);
    m_renderer->Initialize();

    while (OpenXRSessionMgr::applicationRunning)
    {
        PollSystemEvents();
        OpenXRSessionMgr::PollEvent();
        if (OpenXRSessionMgr::IsSessionRunning())
        {
            m_renderer->RenderFrame();
        }
    }

    m_renderer->Cleanup();

    ShutdownOpenXR();
}

void OpenXRTutorial::InitializeOpenXR()
{
    OpenXRCoreMgr::CreateInstance();
    OpenXRCoreMgr::CreateDebugMessenger();
    OpenXRCoreMgr::GetSystemID();
    OpenXRCoreMgr::CreateSession(m_apiType);
    OpenXRDisplayMgr::GetViewConfigurationViews();
    OpenXRDisplayMgr::CreateSwapchains();
    OpenXRDisplayMgr::GetEnvironmentBlendModes();
    OpenXRCoreMgr::CreateReferenceSpaces();
}

void OpenXRTutorial::ShutdownOpenXR()
{
    OpenXRCoreMgr::DestroyReferenceSpace();
    OpenXRDisplayMgr::DestroySwapchains();
    OpenXRCoreMgr::DestroySession();
    OpenXRCoreMgr::DestroyDebugMessenger();
    OpenXRCoreMgr::DestroyInstance();
}
