#include "Camera.h"
#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>
#include <DebugOutput.h>
#include "../../../OpenXR/OpenXRCoreMgr.h"
#include "../../../OpenXR/OpenXRDisplayMgr.h"
#include "../../../OpenXR/OpenXRRenderMgr.h"
#include "../../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "../../Core/Scene.h"

// 静态变量定义
GraphicsAPI_Type Camera::s_globalApiType = UNKNOWN;

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

void Camera::SetupForOpenXR(int viewIndex, void* colorImage, void* depthImage, GraphicsAPI_Type apiType)
{
    m_currentViewIndex = viewIndex;
    m_apiType = apiType;
    
    // 设置渲染设置
    m_renderSettings.colorImage = colorImage;
    m_renderSettings.depthImage = depthImage;
    m_renderSettings.width = OpenXRDisplayMgr::activeViewConfigurationViews[viewIndex].recommendedImageRectWidth;
    m_renderSettings.height = OpenXRDisplayMgr::activeViewConfigurationViews[viewIndex].recommendedImageRectHeight;
    m_renderSettings.blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    m_renderSettings.clearColor = {0.17f, 0.17f, 0.17f, 1.0f};
    m_renderSettings.pipeline = nullptr; // 避免崩溃
    
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
    XR_TUT_LOG("Camera::PreTick() - Starting");
    
    // 自动从 OpenXRDisplayMgr 获取当前视图索引并更新设置
    int currentViewIndex = OpenXRDisplayMgr::GetCurrentViewIndex();
    if (currentViewIndex >= 0) {
        // 更新视图索引和 API 类型
        if (m_currentViewIndex != currentViewIndex) {
            m_currentViewIndex = currentViewIndex;
            m_apiType = s_globalApiType;
            
            // 自动设置渲染设置
            m_renderSettings.width = OpenXRDisplayMgr::activeViewConfigurationViews[currentViewIndex].recommendedImageRectWidth;
            m_renderSettings.height = OpenXRDisplayMgr::activeViewConfigurationViews[currentViewIndex].recommendedImageRectHeight;
            m_renderSettings.blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
            m_renderSettings.clearColor = {0.17f, 0.17f, 0.17f, 1.0f};
            m_renderSettings.pipeline = nullptr; // 避免崩溃
            
            m_needsMatrixUpdate = true;
        }
        
        // Camera 负责获取当前视图的图像数据
        OpenXRDisplayMgr::AcquireAndWaitSwapChainImages(currentViewIndex, m_renderSettings.colorImage, m_renderSettings.depthImage);
        
        // 更新矩阵
        if (m_needsMatrixUpdate) {
            UpdateMatricesFromOpenXR();
            m_needsMatrixUpdate = false;
        }
    }
    
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->BeginRendering();
    XR_TUT_LOG("Camera::PreTick() - BeginRendering called");
    SetupRenderTarget();
    XR_TUT_LOG("Camera::PreTick() - SetupRenderTarget completed");
}

void Camera::PostTick(float deltaTime) {
    XR_TUT_LOG("Camera::PostTick() - Starting");
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->EndRendering();
    XR_TUT_LOG("Camera::PostTick() - EndRendering completed");
    
    // Camera 负责释放当前视图的图像数据
    if (m_currentViewIndex >= 0) {
        OpenXRDisplayMgr::ReleaseSwapChainImages(m_currentViewIndex);
    }
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

void Camera::UpdateMatricesFromOpenXR()
{
    if (m_currentViewIndex < 0) return;
    
    // 从 Tutorial 中移过来的矩阵计算逻辑
    XrMatrix4x4f viewMatrix, projMatrix;
    XrMatrix4x4f rotationMatrix, translationMatrix;
    
    // Create rotation matrix from head orientation
    XrMatrix4x4f_CreateFromQuaternion(&rotationMatrix, &OpenXRRenderMgr::views[m_currentViewIndex].pose.orientation);
    
    // Create translation matrix from head position
    XrMatrix4x4f_CreateTranslation(&translationMatrix, 
        -OpenXRRenderMgr::views[m_currentViewIndex].pose.position.x,
        -OpenXRRenderMgr::views[m_currentViewIndex].pose.position.y, 
        -OpenXRRenderMgr::views[m_currentViewIndex].pose.position.z);
    
    // CRITICAL FIX: For view matrix, we need inverse transform
    // View matrix = inverse(rotation) * inverse(translation)
    // Since rotation is orthogonal, inverse = transpose
    XrMatrix4x4f rotationInverse;
    XrMatrix4x4f_Transpose(&rotationInverse, &rotationMatrix);
    
    // Combine: first apply rotation inverse, then translation inverse
    XrMatrix4x4f_Multiply(&viewMatrix, &rotationInverse, &translationMatrix);
    
    XrMatrix4x4f_CreateProjectionFov(&projMatrix, m_apiType, OpenXRRenderMgr::views[m_currentViewIndex].fov, 0.05f, 1000.0f);
    
    SetViewMatrix(viewMatrix);
    SetProjectionMatrix(projMatrix);
}