#include "Camera.h"
#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>
#include <DebugOutput.h>
#include "../../../OpenXR/OpenXRCoreMgr.h"
#include "../../../OpenXR/OpenXRDisplayMgr.h"
#include "../../../OpenXR/OpenXRRenderMgr.h"
#include "../../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "../../Core/Scene.h"

GraphicsAPI_Type Camera::s_globalApiType = UNKNOWN;

Camera::Camera()
{
    XrMatrix4x4f_CreateIdentity(&m_viewMatrix);
    XrMatrix4x4f_CreateIdentity(&m_projectionMatrix);
    XrMatrix4x4f_CreateIdentity(&m_viewProjectionMatrix);
    
    Scene::SetActiveCamera(this);
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

void Camera::SetupForOpenXR(int viewIndex, void* colorImage, void* depthImage, GraphicsAPI_Type apiType)
{
    m_currentViewIndex = viewIndex;
    m_apiType = apiType;
    
    m_renderSettings.colorImage = colorImage;
    m_renderSettings.depthImage = depthImage;
    m_renderSettings.width = OpenXRDisplayMgr::activeViewConfigurationViews[viewIndex].recommendedImageRectWidth;
    m_renderSettings.height = OpenXRDisplayMgr::activeViewConfigurationViews[viewIndex].recommendedImageRectHeight;
    m_renderSettings.blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    m_renderSettings.clearColor = {0.17f, 0.17f, 0.17f, 1.0f};
    m_renderSettings.pipeline = nullptr;
    
    m_needsMatrixUpdate = true;
}

void Camera::SetupForOpenXRFromDisplayMgr(void* colorImage, void* depthImage, GraphicsAPI_Type apiType)
{
    int currentViewIndex = OpenXRDisplayMgr::GetCurrentViewIndex();
    if (currentViewIndex >= 0) {
        SetupForOpenXR(currentViewIndex, colorImage, depthImage, apiType);
    }
}

void Camera::SetGraphicsAPIType(GraphicsAPI_Type apiType)
{
    s_globalApiType = apiType;
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
    int currentViewIndex = OpenXRDisplayMgr::GetCurrentViewIndex();
    if (currentViewIndex >= 0) {
        if (m_currentViewIndex != currentViewIndex) {
            m_currentViewIndex = currentViewIndex;
            m_apiType = s_globalApiType;
            
            m_renderSettings.width = OpenXRDisplayMgr::activeViewConfigurationViews[currentViewIndex].recommendedImageRectWidth;
            m_renderSettings.height = OpenXRDisplayMgr::activeViewConfigurationViews[currentViewIndex].recommendedImageRectHeight;
            m_renderSettings.blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
            m_renderSettings.clearColor = {0.17f, 0.17f, 0.17f, 1.0f};
            m_renderSettings.pipeline = nullptr;
            
            m_needsMatrixUpdate = true;
        }
        
        OpenXRDisplayMgr::AcquireAndWaitSwapChainImages(currentViewIndex, m_renderSettings.colorImage, m_renderSettings.depthImage);
        
        if (m_needsMatrixUpdate) {
            UpdateMatricesFromOpenXR();
            m_needsMatrixUpdate = false;
        }
    }
    
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->BeginRendering();
    SetupRenderTarget();
}

void Camera::PostTick(float deltaTime) {
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->EndRendering();
    
    if (m_currentViewIndex >= 0) {
        OpenXRDisplayMgr::ReleaseSwapChainImages(m_currentViewIndex);
    }
}

void Camera::SetupRenderTarget()
{
    if (m_renderSettings.colorImage && m_renderSettings.width > 0 && m_renderSettings.height > 0)
    {
        if (m_renderSettings.blendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE)
        {
            OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->ClearColor(
                m_renderSettings.colorImage, 
                m_renderSettings.clearColor.x, 
                m_renderSettings.clearColor.y, 
                m_renderSettings.clearColor.z, 
                m_renderSettings.clearColor.w
            );
        }
        
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->ClearDepth(m_renderSettings.depthImage, 1.0f);
    }
    else
    {
        XR_TUT_LOG_ERROR("Camera::SetupRenderTarget() - Invalid render settings: colorImage=" << m_renderSettings.colorImage << ", width=" << m_renderSettings.width << ", height=" << m_renderSettings.height);
    }
}

void Camera::UpdateViewProjectionMatrix()
{
    XrMatrix4x4f_Multiply(&m_viewProjectionMatrix, &m_projectionMatrix, &m_viewMatrix);
}

void Camera::UpdateMatricesFromOpenXR()
{
    if (m_currentViewIndex < 0) return;
    
    XrMatrix4x4f viewMatrix, projMatrix;
    XrMatrix4x4f rotationMatrix, translationMatrix;
    
    XrMatrix4x4f_CreateFromQuaternion(&rotationMatrix, &OpenXRRenderMgr::views[m_currentViewIndex].pose.orientation);
    
    XrMatrix4x4f_CreateTranslation(&translationMatrix, 
        -OpenXRRenderMgr::views[m_currentViewIndex].pose.position.x,
        -OpenXRRenderMgr::views[m_currentViewIndex].pose.position.y, 
        -OpenXRRenderMgr::views[m_currentViewIndex].pose.position.z);
    
    // View matrix requires inverse transform: rotation is orthogonal so inverse = transpose
    XrMatrix4x4f rotationInverse;
    XrMatrix4x4f_Transpose(&rotationInverse, &rotationMatrix);
    
    XrMatrix4x4f_Multiply(&viewMatrix, &rotationInverse, &translationMatrix);
    
    XrMatrix4x4f_CreateProjectionFov(&projMatrix, m_apiType, OpenXRRenderMgr::views[m_currentViewIndex].fov, 0.05f, 1000.0f);
    
    SetViewMatrix(viewMatrix);
    SetProjectionMatrix(projMatrix);
}