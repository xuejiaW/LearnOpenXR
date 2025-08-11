#pragma once

#include "../../Core/IComponent.h"
#include "RenderSettings.h"
#include <openxr/openxr.h>
#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>

class Camera : public IComponent {
private:
    XrMatrix4x4f m_viewMatrix;
    XrMatrix4x4f m_projectionMatrix;
    XrMatrix4x4f m_viewProjectionMatrix;
    bool m_isDirty = true;

public:
    RenderSettings m_renderSettings;

    Camera();
    
    void SetViewMatrix(const XrMatrix4x4f& viewMatrix);
    void SetProjectionMatrix(const XrMatrix4x4f& projMatrix);
    void SetRenderSettings(const RenderSettings& settings);
    
    void SetupForOpenXR(int viewIndex, void* colorImage, void* depthImage, GraphicsAPI_Type apiType);
    void SetupForOpenXRFromDisplayMgr(void* colorImage, void* depthImage, GraphicsAPI_Type apiType);
    
    static void SetGraphicsAPIType(GraphicsAPI_Type apiType);
    
    const XrMatrix4x4f& GetViewMatrix() const { return m_viewMatrix; }
    const XrMatrix4x4f& GetProjectionMatrix() const { return m_projectionMatrix; }
    const XrMatrix4x4f& GetViewProjectionMatrix();
    const RenderSettings& GetRenderSettings() const { return m_renderSettings; }
    
    void PreTick(float deltaTime) override;
    void PostTick(float deltaTime) override;

private:
    void SetupRenderTarget();
    void UpdateViewProjectionMatrix();
    void UpdateMatricesFromOpenXR();
    
    int m_currentViewIndex = -1;
    GraphicsAPI_Type m_apiType = UNKNOWN;
    bool m_needsMatrixUpdate = false;
    
    static GraphicsAPI_Type s_globalApiType;
};
