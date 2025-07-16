#pragma once

#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>
#include <vector>
#include <memory>
#include <unordered_map>

class IScene;
class Material;

class SceneRenderer
{
public:
    SceneRenderer(GraphicsAPI_Type apiType);
    ~SceneRenderer();

    void SetScene(std::shared_ptr<IScene> scene);
    void CreateResources();
    void Render(const XrMatrix4x4f& viewProj);
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
};
