#include "OpenXRTutorial.h"

#include <GraphicsAPI_Vulkan.h>
#include <openxr/openxr.h>

#include "DebugOutput.h"
#include "XRMathUtils.h"

#include "../OpenXR/OpenXRCoreMgr.h"
#include "../OpenXR/OpenXRDisplayMgr.h"
#include "../OpenXR/OpenXRInputMgr.h"
#include "../OpenXR/OpenXRRenderMgr.h"
#include "../OpenXR/OpenXRSessionMgr.h"
#include "../OpenXR/OpenXRSpaceMgr.h"

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

            OpenXRInputMgr::Tick(OpenXRSessionMgr::frameState.predictedDisplayTime, 
                                OpenXRSpaceMgr::activeSpaces);

            ProcessInputLogic();

            const bool shouldRender = OpenXRSessionMgr::IsShouldRender();

            if (shouldRender)
            {
                OpenXRRenderMgr::RefreshViewsData();
                for (int i = 0; i != static_cast<int>(OpenXRDisplayMgr::GetViewsCount()); ++i)
                {
                    void* colorImage = nullptr;
                    void* depthImage = nullptr;
                    OpenXRDisplayMgr::AcquireAndWaitSwapChainImages(i, colorImage, depthImage);

                    RenderSettings settings{colorImage, depthImage,
                                            OpenXRDisplayMgr::activeViewConfigurationViews[i].recommendedImageRectWidth,
                                            OpenXRDisplayMgr::activeViewConfigurationViews[i].recommendedImageRectHeight,
                                            XR_ENVIRONMENT_BLEND_MODE_OPAQUE, m_sceneRenderer->GetDefaultPipeline()};

                    XrMatrix4x4f viewProj = XRMathUtils::CreateViewProjectionMatrix(m_apiType, OpenXRRenderMgr::views[i], 0.05f, 1000.0f);

                    m_sceneRenderer->Render(viewProj, settings);

                    OpenXRDisplayMgr::ReleaseSwapChainImages(i);
                }
                OpenXRRenderMgr::UpdateRenderLayerInfo();
            }

            OpenXRSessionMgr::EndFrame(shouldRender);
        }
    }
}

void OpenXRTutorial::InitializeSceneRendering()
{
    m_scene = std::make_shared<TableFloorScene>();
    m_scene->Initialize();
    m_sceneRenderer = std::make_unique<SceneRenderer>(m_apiType);
    m_sceneRenderer->SetScene(m_scene);
    m_sceneRenderer->CreateResources();

    // Initialize the refactored input manager
    OpenXRInputMgr::Initialize();
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
    // Shutdown the refactored input manager
    OpenXRInputMgr::Shutdown();

    OpenXRCoreMgr::DestroySession();
    OpenXRCoreMgr::DestroyInstance();
    OpenXRDisplayMgr::DestroySwapchainsRelatedData();
    OpenXRSpaceMgr::DestroyReferenceSpace();
}

void OpenXRTutorial::ProcessInputLogic()
{
    for (int handIndex = 0; handIndex < 2; ++handIndex)
    {
        if (OpenXRInputMgr::GetSelectDown(handIndex))
        {
            XR_TUT_LOG("Hand " << handIndex << " select button pressed! Applying haptic feedback...");
            ApplyHapticFeedback(handIndex);
        }

        bool poseActive;
        XrPosef pose = OpenXRInputMgr::GetHandPose(handIndex, &poseActive);
        if (poseActive)
        {
            XR_TUT_LOG("Hand " << handIndex << " pose - Position: (" << pose.position.x << ", " << pose.position.y 
                      << ", " << pose.position.z << "), Orientation: (" << pose.orientation.x << ", " 
                      << pose.orientation.y << ", " << pose.orientation.z << ", " << pose.orientation.w << ")");
        }
    }
}

void OpenXRTutorial::ApplyHapticFeedback(int handIndex)
{
    float amplitude = 0.5f;
    XrDuration duration = 100000000; // 100ms in nanoseconds
    
    OpenXRInputMgr::TriggerHapticFeedback(handIndex, amplitude, duration);
}
