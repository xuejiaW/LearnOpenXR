#pragma once

#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>
#include <vector>
#include <string>

// Abstract base class for scene rendering
class SceneRenderer
{
public:
    SceneRenderer(GraphicsAPI_Type apiType);
    virtual ~SceneRenderer();

    virtual void CreateResources() = 0;

    virtual void Render(const XrMatrix4x4f& viewProj) = 0;
    virtual void DestroyResources() = 0;
    virtual void* GetPipeline() const = 0;
    virtual void SetViewHeight(float heightM) { m_viewHeightM = heightM; }

protected:
    GraphicsAPI_Type m_apiType;
    float m_viewHeightM = 1.5f;

    void* m_vertexBuffer = nullptr;
    void* m_indexBuffer = nullptr;
    void* m_vertexShader = nullptr;
    void* m_fragmentShader = nullptr;
    void* m_pipeline = nullptr;

    std::vector<char> ReadShaderFile(const std::string& filename);
    void* CreateShaderFromFile(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type shaderType);
};
