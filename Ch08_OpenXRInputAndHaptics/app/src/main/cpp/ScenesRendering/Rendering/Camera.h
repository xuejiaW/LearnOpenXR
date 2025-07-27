#pragma once

#include "../Core/IComponent.h"
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
    struct RenderSettings {
        uint32_t width = 0;
        uint32_t height = 0;
        void* colorImage = nullptr;
        void* depthImage = nullptr;
        XrEnvironmentBlendMode blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        XrVector4f clearColor = {0.0f, 0.0f, 0.2f, 1.0f};
        void* pipeline = nullptr;
    } m_renderSettings;

    Camera();
    
    void SetViewMatrix(const XrMatrix4x4f& viewMatrix);
    void SetProjectionMatrix(const XrMatrix4x4f& projMatrix);
    void SetRenderSettings(const RenderSettings& settings);
    
    const XrMatrix4x4f& GetViewMatrix() const { return m_viewMatrix; }
    const XrMatrix4x4f& GetProjectionMatrix() const { return m_projectionMatrix; }
    const XrMatrix4x4f& GetViewProjectionMatrix();
    const RenderSettings& GetRenderSettings() const { return m_renderSettings; }
    
    void PreTick(float deltaTime) override;
    void PostTick(float deltaTime) override;

private:
    void SetupRenderTarget();
    void UpdateViewProjectionMatrix();
};
