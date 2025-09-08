#include "OpenXRTutorial.h"

#include <GraphicsAPI_Vulkan.h>
#include <openxr/openxr.h>

#include "../OpenXR/OpenXRCoreMgr.h"
#include "../OpenXR/OpenXRDisplayMgr.h"
#include "../OpenXR/OpenXRInputMgr.h"
#include "../OpenXR/OpenXRRenderMgr.h"
#include "../OpenXR/OpenXRSessionMgr.h"
#include "../OpenXR/OpenXRSpaceMgr.h"
#include "../Engine/Components/Rendering/Camera.h"

GraphicsAPI_Type OpenXRTutorial::m_apiType = UNKNOWN;

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType)
{
    m_apiType = apiType;
}

OpenXRTutorial::~OpenXRTutorial() = default;

void OpenXRTutorial::Run()
{
    InitializeOpenXR();
    InitializeSceneRendering();
    while (true)
    {
        PollSystemEvents();
        OpenXRSessionMgr::PollEvent();
        if (OpenXRSessionMgr::IsSessionRunning())
        {
            OpenXRSessionMgr::WaitFrame();
            OpenXRSessionMgr::BeginFrame();

            if (OpenXRSessionMgr::IsShouldProcessInput())
            {

                OpenXRInputMgr::Tick(OpenXRSessionMgr::frameState.predictedDisplayTime,
                                     OpenXRSpaceMgr::activeSpaces);
            }

            const bool shouldRender = OpenXRSessionMgr::IsShouldRender();

            if (shouldRender)
            {
                OpenXRRenderMgr::RefreshViewsData();
                for (int i = 0; i != static_cast<int>(OpenXRDisplayMgr::GetViewsCount()); ++i)
                {
                    OpenXRDisplayMgr::StartRenderingView(i);

                    m_tableFloorScene->Update(0.016f);

                    OpenXRDisplayMgr::StopRenderingView();
                }
                OpenXRRenderMgr::UpdateRenderLayerInfo();
            }

            OpenXRSessionMgr::EndFrame(shouldRender);
        }
    }
}

void OpenXRTutorial::InitializeSceneRendering()
{
    m_tableFloorScene = std::make_unique<TableFloorScene>();
    m_tableFloorScene->Initialize();
    
    Camera::SetGraphicsAPIType(m_apiType);
}

void OpenXRTutorial::InitializeOpenXR()
{
    OpenXRCoreMgr::CreateInstance();
    OpenXRCoreMgr::GetSystemID();

    OpenXRCoreMgr::CreateSession(m_apiType);

    OpenXRInputMgr::CreateActionSet("main_action_set", "Main Action Set", 0);
    OpenXRInputMgr::SetupActions();
    OpenXRInputMgr::SetupBindings();
    OpenXRInputMgr::AttachActionSet();
    OpenXRInputMgr::CreateHandPoseActionSpace();
    
    OpenXRDisplayMgr::GetActiveViewConfigurationType();
    OpenXRDisplayMgr::GetViewConfigurationViewsInfo();
    OpenXRDisplayMgr::CreateSwapchains();
    OpenXRDisplayMgr::CreateSwapchainImages();
    OpenXRDisplayMgr::CreateSwapchainImageViews();
    OpenXRSpaceMgr::CreateReferenceSpace();
}

void OpenXRTutorial::ShutDownOpenXR()
{
    // Shutdown the refactored input manager
    OpenXRInputMgr::Shutdown();

    OpenXRCoreMgr::DestroySession();
    OpenXRCoreMgr::DestroyInstance();
    OpenXRDisplayMgr::DestroySwapchainsRelatedData();
    OpenXRSpaceMgr::DestroyReferenceSpace();
}