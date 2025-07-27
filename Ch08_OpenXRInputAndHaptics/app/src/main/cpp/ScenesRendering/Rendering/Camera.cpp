#include "Camera.h"
#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>
#include <DebugOutput.h>
#include "../../OpenXR/OpenXRCoreMgr.h"
#include "../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "../Core/Scene.h"

Camera::Camera()
{
    XrMatrix4x4f_CreateIdentity(&m_viewMatrix);
    XrMatrix4x4f_CreateIdentity(&m_projectionMatrix);
    XrMatrix4x4f_CreateIdentity(&m_viewProjectionMatrix);
    
    Scene::SetActiveCamera(this);
    XR_TUT_LOG("Camera::Camera() - Set as active camera");
}

void Camera::SetViewMatrix(const XrMatrix4x4f& viewMatrix)
{
    m_viewMatrix = viewMatrix;
    m_isDirty = true;
}

void Camera::SetProjectionMatrix(const XrMatrix4x4f& projMatrix)
{
    m_projectionMatrix = projMatrix;
    m_isDirty = true;
}

void Camera::SetRenderSettings(const RenderSettings& settings)
{
    m_renderSettings = settings;
}

const XrMatrix4x4f& Camera::GetViewProjectionMatrix()
{
    if (m_isDirty)
    {
        UpdateViewProjectionMatrix();
        m_isDirty = false;
    }
    return m_viewProjectionMatrix;
}

void Camera::PreTick(float deltaTime) {
    XR_TUT_LOG("Camera::PreTick() - Starting");
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->BeginRendering();
    XR_TUT_LOG("Camera::PreTick() - BeginRendering called");
    SetupRenderTarget();
    XR_TUT_LOG("Camera::PreTick() - SetupRenderTarget completed");
}

void Camera::PostTick(float deltaTime) {
    XR_TUT_LOG("Camera::PostTick() - Starting");
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->EndRendering();
    XR_TUT_LOG("Camera::PostTick() - EndRendering completed");
}

void Camera::SetupRenderTarget()
{
    XR_TUT_LOG("Camera::SetupRenderTarget() - Starting");
    XR_TUT_LOG("Camera::SetupRenderTarget() - colorImage: " << m_renderSettings.colorImage << ", width: " << m_renderSettings.width << ", height: " << m_renderSettings.height);
    
    if (m_renderSettings.colorImage && m_renderSettings.width > 0 && m_renderSettings.height > 0)
    {
        // Clear color buffer for opaque blend mode
        if (m_renderSettings.blendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE)
        {
            XR_TUT_LOG("Camera::SetupRenderTarget() - Clearing color buffer");
            OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->ClearColor(
                m_renderSettings.colorImage, 
                m_renderSettings.clearColor.x, 
                m_renderSettings.clearColor.y, 
                m_renderSettings.clearColor.z, 
                m_renderSettings.clearColor.w
            );
        }
        
        // Clear depth buffer
        XR_TUT_LOG("Camera::SetupRenderTarget() - Clearing depth buffer");
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->ClearDepth(m_renderSettings.depthImage, 1.0f);
        
        XR_TUT_LOG("Camera::SetupRenderTarget() - Camera only handles clearing, Materials will handle render attachments");
    }
    else
    {
        XR_TUT_LOG_ERROR("Camera::SetupRenderTarget() - Invalid render settings: colorImage=" << m_renderSettings.colorImage << ", width=" << m_renderSettings.width << ", height=" << m_renderSettings.height);
    }
    
    XR_TUT_LOG("Camera::SetupRenderTarget() - Completed");
}

void Camera::UpdateViewProjectionMatrix()
{
    XrMatrix4x4f_Multiply(&m_viewProjectionMatrix, &m_projectionMatrix, &m_viewMatrix);
}