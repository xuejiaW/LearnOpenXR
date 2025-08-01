#include "OpenXRTutorial.h"

#include <GraphicsAPI_Vulkan.h>
#include <openxr/openxr.h>

#include "XRMathUtils.h"

#include "../OpenXR/OpenXRCoreMgr.h"
#include "../OpenXR/OpenXRDisplayMgr.h"
#include "../OpenXR/OpenXRRenderMgr.h"
#include "../OpenXR/OpenXRSessionMgr.h"
#include "../OpenXR/OpenXRSpaceMgr.h"
#include "../Engine/Components/Rendering/Camera.h"

GraphicsAPI_Type OpenXRTutorial::m_apiType = UNKNOWN;

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType)
{
    m_apiType = apiType;
}

OpenXRTutorial::~OpenXRTutorial() {}

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
    OpenXRDisplayMgr::GetActiveViewConfigurationType();
    OpenXRDisplayMgr::GetViewConfigurationViewsInfo();
    OpenXRDisplayMgr::CreateSwapchains();
    OpenXRDisplayMgr::CreateSwapchainImages();
    OpenXRDisplayMgr::CreateSwapchainImageViews();
    OpenXRSpaceMgr::CreateReferenceSpace();
}

void OpenXRTutorial::ShutDownOpenXR()
{
    OpenXRCoreMgr::DestroySession();
    OpenXRCoreMgr::DestroyInstance();
    OpenXRDisplayMgr::DestroySwapchainsRelatedData();
    OpenXRSpaceMgr::DestroyReferenceSpace();
}