#include "OpenXRTutorial.h"
#include <DebugOutput.h>
#include <OpenXRDebugUtils.h>

#include <GraphicsAPI_Vulkan.h>
#include <openxr/openxr.h>
#include "../OpenXR//OpenXRCoreMgr.h"
#include "../OpenXR/OpenXRDisplayMgr.h"
#include "../OpenXR/OpenXRSessionMgr.h"

GraphicsAPI_Type OpenXRTutorial::m_apiType = UNKNOWN;

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType)
{
    m_apiType = apiType;
    m_scene = new TableFloorScene(apiType);
}

OpenXRTutorial::~OpenXRTutorial()
{
    if (m_scene)
    {
        delete m_scene;
        m_scene = nullptr;
    }
}

void OpenXRTutorial::Run()
{
    OpenXRCoreMgr::CreateInstance();
    OpenXRCoreMgr::CreateDebugMessenger();
    OpenXRCoreMgr::GetSystemID();
    OpenXRCoreMgr::CreateSession();
    OpenXRDisplayMgr::GetViewConfigurationViews();
    OpenXRDisplayMgr::CreateSwapchains();
    OpenXRDisplayMgr::GetEnvironmentBlendModes();
    OpenXRCoreMgr::CreateReferenceSpaces();

    m_scene->CreateResources();

    while (OpenXRSessionMgr::applicationRunning)
    {
        PollSystemEvents();
        OpenXRSessionMgr::PollEvent();
        if (OpenXRSessionMgr::IsSessionRunning())
        {
            RenderFrame();
        }
    }

    m_scene->DestroyResources();

    OpenXRCoreMgr::DestroyReferenceSpace();
    OpenXRDisplayMgr::DestroySwapchains();
    OpenXRCoreMgr::DestroySession();
    OpenXRCoreMgr::DestroyDebugMessenger();
    OpenXRCoreMgr::DestroyInstance();
}

void OpenXRTutorial::RenderFrame()
{
    OpenXRSessionMgr::WaitFrame();
    OpenXRSessionMgr::BeginFrame();

    const bool shouldRender = OpenXRSessionMgr::ShouldRender();
    if (shouldRender)
    {
        RenderLayer();
    }

    OpenXRSessionMgr::EndFrame(shouldRender);
}

void OpenXRTutorial::RenderLayer()
{
    const int viewCount = OpenXRDisplayMgr::RefreshViewsData();
    OpenXRDisplayMgr::GenerateRenderLayerInfo();

    for (int i = 0; i < viewCount; ++i)
    {
        void* colorImage = nullptr;
        void* depthImage = nullptr;
        OpenXRDisplayMgr::AcquireSwapChainImages(i, colorImage, depthImage);
        SwapchainInfo& colorSwapchainInfo = OpenXRDisplayMgr::m_ColorSwapchainInfos[i];
        SwapchainInfo& depthSwapchainInfo = OpenXRDisplayMgr::m_DepthSwapchainInfos[i];

        const uint32_t& width = OpenXRDisplayMgr::m_ActiveViewConfigurationViews[i].recommendedImageRectWidth;
        const uint32_t& height = OpenXRDisplayMgr::m_ActiveViewConfigurationViews[i].recommendedImageRectHeight;
        GraphicsAPI::Viewport viewport = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
        GraphicsAPI::Rect2D scissor = {{0, 0}, {width, height}};

        OpenXRDisplayMgr::RefreshProjectionLayerViews(i);

        OpenXRCoreMgr::graphicsAPI->BeginRendering();

        if (OpenXRDisplayMgr::m_ActiveEnvironmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE)
        {
            OpenXRCoreMgr::graphicsAPI->ClearColor(colorImage, 0.17f, 0.17f, 0.17f, 1.00f);
        }
        OpenXRCoreMgr::graphicsAPI->ClearDepth(depthImage, 1.0f);

        OpenXRCoreMgr::graphicsAPI->SetRenderAttachments(&colorImage, 1, depthImage, width,
                                                         height, m_scene->GetPipeline());
        OpenXRCoreMgr::graphicsAPI->SetViewports(&viewport, 1);
        OpenXRCoreMgr::graphicsAPI->SetScissors(&scissor, 1);

        XrMatrix4x4f proj;
        XrMatrix4x4f_CreateProjectionFov(&proj, m_apiType, OpenXRDisplayMgr::views[i].fov, 0.05f, 1000.0f);

        XrMatrix4x4f toView;
        XrVector3f scale1m{1.0f, 1.0f, 1.0f};
        XrMatrix4x4f_CreateTranslationRotationScale(&toView, &OpenXRDisplayMgr::views[i].pose.position, &OpenXRDisplayMgr::views[i].pose.orientation,
                                                    &scale1m);
        XrMatrix4x4f view;
        XrMatrix4x4f_InvertRigidBody(&view, &toView);

        XrMatrix4x4f viewProj;
        XrMatrix4x4f_Multiply(&viewProj, &proj, &view);

        m_scene->SetViewHeight(m_viewHeightM);
        m_scene->Render(viewProj);

        OpenXRCoreMgr::graphicsAPI->EndRendering();

        // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
        XrSwapchainImageReleaseInfo releaseInfo{};
        releaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
        OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
        OPENXR_CHECK(xrReleaseSwapchainImage(depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");

    }
}