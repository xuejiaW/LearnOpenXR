#include "OpenXRRenderer.h"
#include "ScenesRendering/Scenes/SceneRenderer.h"
#include "ScenesRendering/Scenes/IScene.h"
#include "Utils/XRMathUtils.h"
#include <DebugOutput.h>
#include "OpenXR/OpenXRCoreMgr.h"
#include "OpenXR/OpenXRDisplayMgr.h"
#include "OpenXR/OpenXRSessionMgr.h"

OpenXRRenderer::OpenXRRenderer(GraphicsAPI_Type apiType)
    : m_apiType(apiType), m_sceneRenderer(std::make_unique<SceneRenderer>(apiType))
{}

OpenXRRenderer::~OpenXRRenderer() = default;

void OpenXRRenderer::SetScene(std::shared_ptr<IScene> scene)
{
    m_scene = scene;
    m_sceneRenderer->SetScene(scene);
}

void OpenXRRenderer::Initialize()
{
    if (m_scene) {
        m_scene->Initialize();
    }
    m_sceneRenderer->CreateResources();
}

void OpenXRRenderer::RenderFrame()
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

void OpenXRRenderer::RenderLayer()
{
    const int viewCount = OpenXRDisplayMgr::RefreshViewsData();
    OpenXRDisplayMgr::GenerateRenderLayerInfo();

    for (int i = 0; i < viewCount; ++i)
    {
        RenderView(i);
    }
}

void OpenXRRenderer::RenderView(int viewIndex)
{
    if (!m_sceneRenderer)
    {
        XR_TUT_LOG_ERROR("No scene renderer set for OpenXRRenderer");
        return;
    }

    void* colorImage = nullptr;
    void* depthImage = nullptr;
    OpenXRDisplayMgr::AcquireSwapChainImages(viewIndex, colorImage, depthImage);

    OpenXRDisplayMgr::RefreshProjectionLayerViews(viewIndex);

    OpenXRCoreMgr::GetGraphicsAPI()->BeginRendering();

    SetupRenderState(viewIndex, colorImage, depthImage);

    XrMatrix4x4f viewProj = XRMathUtils::CreateViewProjectionMatrix(m_apiType, OpenXRDisplayMgr::views[viewIndex], m_nearPlane, m_farPlane);

    m_sceneRenderer->Render(viewProj);

    OpenXRCoreMgr::GetGraphicsAPI()->EndRendering();

    OpenXRDisplayMgr::ReleaseSwapChainImages(viewIndex);
}

void OpenXRRenderer::SetupRenderState(int viewIndex, void* colorImage, void* depthImage)
{
    const uint32_t& width = OpenXRDisplayMgr::m_ActiveViewConfigurationViews[viewIndex].recommendedImageRectWidth;
    const uint32_t& height = OpenXRDisplayMgr::m_ActiveViewConfigurationViews[viewIndex].recommendedImageRectHeight;

    GraphicsAPI::Viewport viewport = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    GraphicsAPI::Rect2D scissor = {{0, 0}, {width, height}};

    if (OpenXRDisplayMgr::m_ActiveEnvironmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE)
    {
        OpenXRCoreMgr::GetGraphicsAPI()->ClearColor(colorImage, 0.17f, 0.17f, 0.17f, 1.00f);
    }
    OpenXRCoreMgr::GetGraphicsAPI()->ClearDepth(depthImage, 1.0f);

    void* defaultPipeline = m_sceneRenderer->GetDefaultPipeline();
    if (defaultPipeline) {
        OpenXRCoreMgr::GetGraphicsAPI()->SetRenderAttachments(&colorImage, 1, depthImage, width, height, defaultPipeline);
        OpenXRCoreMgr::GetGraphicsAPI()->SetViewports(&viewport, 1);
        OpenXRCoreMgr::GetGraphicsAPI()->SetScissors(&scissor, 1);
    }
}

void OpenXRRenderer::Cleanup()
{
    m_sceneRenderer->DestroyResources();
}
