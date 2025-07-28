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
#include "../ScenesRendering/Core/GameObject.h"
#include "../ScenesRendering/Rendering/Camera.h"

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

                    // Get camera from scene
                    GameObject* cameraObject = m_tableFloorScene->GetScene()->FindGameObject("Camera");
                    if (cameraObject) {
                        Camera* camera = cameraObject->GetComponent<Camera>();
                        if (camera) {
                            // Set up camera render settings
                            XR_TUT_LOG("sss update camera");
                            Camera::RenderSettings settings;
                            settings.colorImage = colorImage;
                            settings.depthImage = depthImage;
                            settings.width = OpenXRDisplayMgr::activeViewConfigurationViews[i].recommendedImageRectWidth;
                            settings.height = OpenXRDisplayMgr::activeViewConfigurationViews[i].recommendedImageRectHeight;
                            settings.blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
                            settings.clearColor = {0.17f, 0.17f, 0.17f, 1.0f};
                            
                            // IMPORTANT: Create a default pipeline to avoid crash
                            // For now, set pipeline to nullptr to skip SetRenderAttachments call
                            // This prevents the Vulkan crash but may not render correctly
                            settings.pipeline = nullptr;
                            
                            camera->SetRenderSettings(settings);
                            
                            // Set up view and projection matrices
                            XrMatrix4x4f viewMatrix, projMatrix;
                            
                            // Create view matrix from pose (position and orientation)
                            // For VR, view matrix = inverse of head transform
                            XrMatrix4x4f rotationMatrix, translationMatrix;
                            
                            // Create rotation matrix from head orientation
                            XrMatrix4x4f_CreateFromQuaternion(&rotationMatrix, &OpenXRRenderMgr::views[i].pose.orientation);
                            
                            // Create translation matrix from head position
                            XrMatrix4x4f_CreateTranslation(&translationMatrix, 
                                -OpenXRRenderMgr::views[i].pose.position.x,
                                -OpenXRRenderMgr::views[i].pose.position.y, 
                                -OpenXRRenderMgr::views[i].pose.position.z);
                            
                            // CRITICAL FIX: For view matrix, we need inverse transform
                            // View matrix = inverse(rotation) * inverse(translation)
                            // Since rotation is orthogonal, inverse = transpose
                            XrMatrix4x4f rotationInverse;
                            XrMatrix4x4f_Transpose(&rotationInverse, &rotationMatrix);
                            
                            // Combine: first apply rotation inverse, then translation inverse
                            XrMatrix4x4f_Multiply(&viewMatrix, &rotationInverse, &translationMatrix);
                            
                            XrMatrix4x4f_CreateProjectionFov(&projMatrix, m_apiType, OpenXRRenderMgr::views[i].fov, 0.05f, 1000.0f);
                            
                            camera->SetViewMatrix(viewMatrix);
                            camera->SetProjectionMatrix(projMatrix);
                        }
                    }

                    // Update scene with delta time
                    m_tableFloorScene->Update(0.016f); // Assuming ~60fps

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
    m_tableFloorScene = std::make_unique<TableFloorScene>();
    m_tableFloorScene->Initialize();
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