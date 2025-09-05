#pragma once

#include <GraphicsAPI.h>
#include <openxr/openxr.h>
#include <xr_linear_algebra.h>
#include "../../Core/IComponent.h"
#include "RenderSettings.h"


class Camera : public IComponent
{
  public:
    Camera();

    void SetFieldOfView(const XrFovf& fov);
    void SetProjectionParameters(float nearPlane, float farPlane);
    void SetClearColor(float r, float g, float b, float a = 1.0f);

    static void SetGraphicsAPIType(GraphicsAPI_Type apiType);

    const XrMatrix4x4f& GetViewMatrix();
    const XrMatrix4x4f& GetProjectionMatrix();
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
    XrVector4f m_ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};

    static GraphicsAPI_Type s_globalApiType;
    RenderSettings m_RenderSettings;

    void SetupRenderTarget();
};
