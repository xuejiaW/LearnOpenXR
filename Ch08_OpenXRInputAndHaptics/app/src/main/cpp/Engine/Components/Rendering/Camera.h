#pragma once

#include "../../Core/IComponent.h"
#include "RenderSettings.h"
#include <openxr/openxr.h>
#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>

class Camera : public IComponent {
public:
    RenderSettings m_RenderSettings;

    Camera();
    
    void SetFieldOfView(const XrFovf& fov);
    void SetProjectionParameters(float nearPlane, float farPlane);
    void SetViewMatrix(const XrMatrix4x4f& viewMatrix);
    void SetProjectionMatrix(const XrMatrix4x4f& projMatrix);
    void SetRenderSettings(const RenderSettings& settings);
    
    void SetupForOpenXR(int viewIndex, void* colorImage, void* depthImage, GraphicsAPI_Type apiType);
    void SetupForOpenXRFromDisplayMgr(void* colorImage, void* depthImage, GraphicsAPI_Type apiType);
    
    static void SetGraphicsAPIType(GraphicsAPI_Type apiType);
    
    const XrMatrix4x4f& GetViewMatrix();
    const XrMatrix4x4f& GetProjectionMatrix();
    const XrMatrix4x4f& GetViewProjectionMatrix();
    const RenderSettings& GetRenderSettings() const { return m_RenderSettings; }
    
    void PreTick(float deltaTime) override;
    void PostTick(float deltaTime) override;

private:
    XrFovf m_FieldOfView = {-1.0f, 1.0f, 1.0f, -1.0f};
    float m_NearPlane = 0.05f;
    float m_FarPlane = 1000.0f;
    
    XrMatrix4x4f m_ProjectionMatrix;
    XrMatrix4x4f m_ViewProjectionMatrix;
    bool m_ProjectionDirty = true;
    bool m_ViewProjectionDirty = true;
    
    int m_CurrentViewIndex = -1;
    GraphicsAPI_Type m_ApiType = UNKNOWN;
    bool m_NeedsMatrixUpdate = false;
    
    static GraphicsAPI_Type s_globalApiType;
    
    void SetupRenderTarget();
    void UpdateProjectionMatrix();
    void UpdateViewProjectionMatrix();
    void UpdateMatricesFromOpenXR();
};
