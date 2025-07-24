#pragma once

#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <openxr/openxr.h>

class IScene;
class Material;

// Render settings for scene rendering
struct RenderSettings
{
    void* colorImage;
    void* depthImage;
    uint32_t width;
    uint32_t height;
    XrEnvironmentBlendMode blendMode;
    void* pipeline;
    XrVector4f clearColor = {0.17f, 0.17f, 0.17f, 1.0f};
};

class SceneRenderer
{
public:
    SceneRenderer(GraphicsAPI_Type apiType);
    ~SceneRenderer();

    void SetScene(std::shared_ptr<IScene> scene);
    void CreateResources();
    
    void Render(const XrMatrix4x4f& viewProj, const RenderSettings& settings);
    void Render(const XrMatrix4x4f& viewProj); // Backward compatibility
    
    void DestroyResources();
    void* GetDefaultPipeline();

private:
    struct ObjectRenderData
    {
        XrMatrix4x4f viewProj;
        XrMatrix4x4f modelViewProj;
        XrMatrix4x4f model;
        XrVector4f color;
        XrVector4f pad1;
        XrVector4f pad2;
        XrVector4f pad3;
    };
    
    GraphicsAPI_Type m_apiType;
    std::shared_ptr<IScene> m_scene;
    
    void* m_uniformBuffer_ObjectData = nullptr;
    std::unordered_map<std::string, void*> m_renderPipelines;
    
    void* GetOrCreatePipeline(std::shared_ptr<Material> material);
    void RenderObject(const struct SceneObject& object, const XrMatrix4x4f& viewProj, size_t objectIndex);
    void SetupRenderTarget(const RenderSettings& settings);
};
