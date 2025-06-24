#pragma once

#include "../Rendering/SceneRenderer.h"

// TableFloorScene - 一个具体的场景实现，渲染桌子和地板
class TableFloorScene : public SceneRenderer
{
public:
    TableFloorScene(GraphicsAPI_Type apiType);
    ~TableFloorScene() override;

    // 实现基类的抽象方法
    void CreateResources() override;
    void Render(const XrMatrix4x4f& viewProj) override;
    void DestroyResources() override;

    // 设置视图高度
    void SetViewHeight(float heightInMeters) override { m_viewHeightM = heightInMeters; }

    // 获取图形管线
    void* GetPipeline() const override { return m_pipeline; }

private:
    // 渲染一个立方体
    void RenderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color);

    // 数据结构
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

    // 渲染数据
    float m_viewHeightM = 1.5f;
    CuboidConstants m_cuboidConstants;
    void* m_uniformBuffer_Camera = nullptr;
    void* m_uniformBuffer_Normals = nullptr;
    void* m_pipeline = nullptr;

    // 法线数据
    XrVector4f m_normals[6] = {
        {1.00f, 0.00f, 0.00f, 0},
        {-1.00f, 0.00f, 0.00f, 0},
        {0.00f, 1.00f, 0.00f, 0},
        {0.00f, -1.00f, 0.00f, 0},
        {0.00f, 0.00f, 1.00f, 0},
        {0.00f, 0.0f, -1.00f, 0}
    };
};
