#pragma once

#include "../Rendering/SceneRenderer.h"

class TableFloorScene : public SceneRenderer
{
public:
    TableFloorScene(GraphicsAPI_Type apiType);
    ~TableFloorScene() override;

    void CreateResources() override;
    void Render(const XrMatrix4x4f& viewProj) override;
    void DestroyResources() override;
    void SetViewHeight(float heightInMeters) override { m_viewHeightM = heightInMeters; }
    void* GetPipeline() const override { return m_pipeline; }

private:
    void RenderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color);

    struct CuboidConstants
    {
        XrMatrix4x4f viewProj;
        XrMatrix4x4f modelViewProj;
        XrMatrix4x4f model;
        XrVector4f color;
        XrVector4f pad1;
        XrVector4f pad2;
        XrVector4f pad3;
    };

    float m_viewHeightM = 1.5f;
    CuboidConstants m_cuboidConstants;
    void* m_uniformBuffer_Camera = nullptr;
    void* m_uniformBuffer_Normals = nullptr;
    void* m_pipeline = nullptr;

    XrVector4f m_normals[6] = {
        {1.00f, 0.00f, 0.00f, 0},
        {-1.00f, 0.00f, 0.00f, 0},
        {0.00f, 1.00f, 0.00f, 0},
        {0.00f, -1.00f, 0.00f, 0},
        {0.00f, 0.00f, 1.00f, 0},
        {0.00f, 0.0f, -1.00f, 0}
    };
};
