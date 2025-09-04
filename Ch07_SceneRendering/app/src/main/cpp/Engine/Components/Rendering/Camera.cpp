#include "Camera.h"
#include <DebugOutput.h>
#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>
#include "../../../OpenXR/OpenXRCoreMgr.h"
#include "../../../OpenXR/OpenXRDisplayMgr.h"
#include "../../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "../../../OpenXR/OpenXRRenderMgr.h"
#include "../../Core/GameObject.h"
#include "../../Core/Scene.h"
#include "../Core/Transform.h"

GraphicsAPI_Type Camera::s_globalApiType = UNKNOWN;

Camera::Camera()
{
    XrMatrix4x4f_CreateIdentity(&m_ProjectionMatrix);
    XrMatrix4x4f_CreateIdentity(&m_ViewProjectionMatrix);

    Scene::SetActiveCamera(this);
}

void Camera::SetFieldOfView(const XrFovf& fov)
{
    m_FieldOfView = fov;
    m_ProjectionDirty = true;
    m_ViewProjectionDirty = true;
}

void Camera::SetProjectionParameters(float nearPlane, float farPlane)
{
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    m_ProjectionDirty = true;
    m_ViewProjectionDirty = true;
}

void Camera::SetClearColor(float r, float g, float b, float a)
{
    m_ClearColor = {r, g, b, a};
    m_RenderSettings.clearColor = m_ClearColor;
}

void Camera::SetGraphicsAPIType(GraphicsAPI_Type apiType) { s_globalApiType = apiType; }

const XrMatrix4x4f& Camera::GetViewMatrix()
{
    Transform* transform = GetGameObject()->GetComponent<Transform>();
    if (transform)
    {
        return transform->GetViewMatrix();
    }

    static XrMatrix4x4f identity;
    XrMatrix4x4f_CreateIdentity(&identity);
    return identity;
}

const XrMatrix4x4f& Camera::GetProjectionMatrix()
{
    if (m_ProjectionDirty)
    {
        XrMatrix4x4f_CreateProjectionFov(&m_ProjectionMatrix, m_ApiType, m_FieldOfView, m_NearPlane, m_FarPlane);
        m_ProjectionDirty = false;
    }
    return m_ProjectionMatrix;
}

void Camera::PreTick(float deltaTime)
{
    int currentViewIndex = OpenXRDisplayMgr::GetCurrentViewIndex();
    if (currentViewIndex <= -1) return;

    if (m_CurrentViewIndex != currentViewIndex)
    {
        m_CurrentViewIndex = currentViewIndex;
        m_ApiType = s_globalApiType;

        m_RenderSettings.width = OpenXRDisplayMgr::activeViewConfigurationViews[currentViewIndex].recommendedImageRectWidth;
        m_RenderSettings.height = OpenXRDisplayMgr::activeViewConfigurationViews[currentViewIndex].recommendedImageRectHeight;
        m_RenderSettings.blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        m_RenderSettings.clearColor = m_ClearColor;
        m_RenderSettings.pipeline = nullptr;

        const XrFovf& fov = OpenXRRenderMgr::views[m_CurrentViewIndex].fov;
        SetFieldOfView(fov);
    }

    OpenXRDisplayMgr::AcquireAndWaitSwapChainImages(currentViewIndex, m_RenderSettings.colorImage, m_RenderSettings.depthImage);

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->BeginRendering();
    SetupRenderTarget();
}

void Camera::PostTick(float deltaTime)
{
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->EndRendering();

    if (m_CurrentViewIndex >= 0)
    {
        OpenXRDisplayMgr::ReleaseSwapChainImages(m_CurrentViewIndex);
    }
}

void Camera::SetupRenderTarget()
{
    if (m_RenderSettings.colorImage && m_RenderSettings.width > 0 && m_RenderSettings.height > 0)
    {
        if (m_RenderSettings.blendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE)
        {
            OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->ClearColor(m_RenderSettings.colorImage, m_RenderSettings.clearColor.x,
                                                                      m_RenderSettings.clearColor.y, m_RenderSettings.clearColor.z,
                                                                      m_RenderSettings.clearColor.w);
        }

        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->ClearDepth(m_RenderSettings.depthImage, 1.0f);
    }
    else
    {
        XR_TUT_LOG_ERROR("Camera::SetupRenderTarget() - Invalid render settings: colorImage="
                         << m_RenderSettings.colorImage << ", width=" << m_RenderSettings.width << ", height=" << m_RenderSettings.height);
    }
}