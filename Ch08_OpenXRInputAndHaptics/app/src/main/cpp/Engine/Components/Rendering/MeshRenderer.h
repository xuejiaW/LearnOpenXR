#pragma once

#include "../../Core/IComponent.h"
#include "../../Rendering/Mesh/IMesh.h"
#include <memory>
#include <GraphicsAPI.h>

class MeshRenderer : public IComponent {
private:
    std::shared_ptr<IMesh> m_mesh;
    void* m_vertexBuffer = nullptr;
    void* m_indexBuffer = nullptr;
    void* m_uniformBuffer = nullptr;
    void* m_faceNormalsBuffer = nullptr;
    bool m_buffersCreated = false;

public:
    MeshRenderer() = default;
    ~MeshRenderer();
    
    void SetMesh(std::shared_ptr<IMesh> mesh);
    std::shared_ptr<IMesh> GetMesh() const { return m_mesh; }
    
    void Initialize() override;
    void Tick(float deltaTime) override;
    void Destroy() override;

private:
    void CreateBuffers();
    void RenderMesh();
    void DestroyBuffers();
};
