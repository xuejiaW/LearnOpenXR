#include "OpenXRRenderer.h"
#include <DebugOutput.h>
#include "OpenXR/OpenXRCoreMgr.h"
#include "OpenXR/OpenXRDisplayMgr.h"
#include "OpenXR/OpenXRSessionMgr.h"
#include "ScenesRendering/Scenes/IScene.h"
#include "ScenesRendering/Scenes/SceneRenderer.h"
#include "Utils/XRMathUtils.h"

OpenXRRenderer::OpenXRRenderer(GraphicsAPI_Type apiType) : m_apiType(apiType), m_sceneRenderer(std::make_unique<SceneRenderer>(apiType)) {}

OpenXRRenderer::~OpenXRRenderer() = default;

void OpenXRRenderer::SetScene(std::shared_ptr<IScene> scene)
{
    m_scene = scene;
    m_sceneRenderer->SetScene(scene);
}

void OpenXRRenderer::Initialize()
{
    if (m_scene)
    {
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
        const int viewCount = OpenXRDisplayMgr::RefreshViewsData();

        for (int i = 0; i < viewCount; ++i)
        {
            RenderView(i);
        }

        OpenXRDisplayMgr::UpdateRenderLayerInfo();
    }

    OpenXRSessionMgr::EndFrame(shouldRender);
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

    RenderSettings settings{colorImage,
                            depthImage,
                            OpenXRDisplayMgr::m_ActiveViewConfigurationViews[viewIndex].recommendedImageRectWidth,
                            OpenXRDisplayMgr::m_ActiveViewConfigurationViews[viewIndex].recommendedImageRectHeight,
                            OpenXRDisplayMgr::m_ActiveEnvironmentBlendMode,
                            m_sceneRenderer->GetDefaultPipeline()};

    XrMatrix4x4f viewProj = XRMathUtils::CreateViewProjectionMatrix(m_apiType, OpenXRDisplayMgr::views[viewIndex], m_nearPlane, m_farPlane);

    m_sceneRenderer->Render(viewProj, settings);

    OpenXRDisplayMgr::ReleaseSwapChainImages(viewIndex);
}

void OpenXRRenderer::Cleanup() { m_sceneRenderer->DestroyResources(); }
