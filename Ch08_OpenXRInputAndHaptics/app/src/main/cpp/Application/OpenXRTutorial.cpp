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

                for (int handIndex = 0; handIndex < 2; ++handIndex)
                {
                    if (OpenXRInputMgr::GetSelectDown(handIndex))
                    {
                        OpenXRInputMgr::TriggerHapticFeedback(handIndex, 0.5f, 100000000);
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
}

void OpenXRTutorial::InitializeOpenXR()
{
    OpenXRCoreMgr::CreateInstance();
    OpenXRCoreMgr::GetSystemID();

    OpenXRInputMgr::CreateActionSet("main_action_set", "Main Action Set", 0);
    OpenXRInputMgr::SetupActions();
    OpenXRInputMgr::SetupBindings();
    OpenXRInputMgr::SubmitAllBindings();

    OpenXRCoreMgr::CreateSession(m_apiType);

    OpenXRInputMgr::AttachActionSets();
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