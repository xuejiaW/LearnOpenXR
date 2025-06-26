#include "OpenXRRenderer.h"
#include "../Utils/XRMathUtils.h"
#include <DebugOutput.h>
#include <OpenXRDebugUtils.h>
#include "../OpenXR/OpenXRCoreMgr.h"
#include "../OpenXR/OpenXRDisplayMgr.h"
#include "../OpenXR/OpenXRSessionMgr.h"

OpenXRRenderer::OpenXRRenderer(GraphicsAPI_Type apiType)
    : m_apiType(apiType)
{}

OpenXRRenderer::~OpenXRRenderer() = default;

void OpenXRRenderer::SetSceneRenderer(SceneRenderer* sceneRenderer)
{
    m_sceneRenderer = sceneRenderer;
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

    OpenXRCoreMgr::GetGraphicsAPI()->SetRenderAttachments(&colorImage, 1, depthImage, width, height,
                                                     m_sceneRenderer->GetPipeline());
    OpenXRCoreMgr::GetGraphicsAPI()->SetViewports(&viewport, 1);
    OpenXRCoreMgr::GetGraphicsAPI()->SetScissors(&scissor, 1);
}
