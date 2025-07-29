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
#include "../Engine/Core/GameObject.h"
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
                    // 设置当前渲染的视图索引
                    OpenXRDisplayMgr::StartRenderingView(i);

                    // Tutorial 只需要调用 Scene 的 Update，Camera 会自动处理所有事情
                    m_tableFloorScene->Update(0.016f); // Assuming ~60fps

                    // 清除当前视图索引
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
    
    // 设置 Camera 的全局 Graphics API 类型
    Camera::SetGraphicsAPIType(m_apiType);
}

void OpenXRTutorial::InitializeOpenXR()
{
    OpenXRCoreMgr::CreateInstance();
    OpenXRCoreMgr::GetSystemID();

    OpenXRInputMgr::CreateActionSet("main_action_set", "Main Action Set", 0);
    OpenXRInputMgr::SetupActions();
    OpenXRInputMgr::SetupBindings();

    OpenXRCoreMgr::CreateSession(m_apiType);

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