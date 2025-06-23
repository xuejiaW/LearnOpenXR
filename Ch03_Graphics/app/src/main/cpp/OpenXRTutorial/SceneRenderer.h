#pragma once

#include <GraphicsAPI.h>
#include <openxr/openxr.h>
#include <xr_linear_algebra.h>
#include <vector>
#include <string>
#include "../OpenXR/OpenXRCoreMgr.h"

// Abstract base class for scene rendering
class SceneRenderer
{
public:
    SceneRenderer(GraphicsAPI_Type apiType);
    virtual ~SceneRenderer();

    // Initialize the scene resources (shaders, buffers, etc.)
    virtual void CreateResources() = 0;

    // Render the scene with the given view projection matrix
    virtual void Render(const XrMatrix4x4f& viewProj) = 0;

    // Clean up scene resources
    virtual void DestroyResources() = 0;

protected:
    // Common rendering data
    GraphicsAPI_Type m_apiType;

    // Graphics resources
    void* m_vertexBuffer = nullptr;
    void* m_indexBuffer = nullptr;
    void* m_vertexShader = nullptr;
    void* m_fragmentShader = nullptr;
    void* m_pipeline = nullptr;

    // Helper methods for derived classes
    std::vector<char> ReadShaderFile(const std::string& filename);    // 辅助方法：加载着色器并创建着色器对象
    void* CreateShaderFromFile(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type shaderType);
};