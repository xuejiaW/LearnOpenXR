#pragma once

#include "../Core/IComponent.h"
#include "Mesh/IMesh.h"
#include <memory>
#include <GraphicsAPI.h>

class MeshRenderer : public IComponent {
private:
    std::shared_ptr<IMesh> m_mesh;
    void* m_vertexBuffer = nullptr;
    void* m_indexBuffer = nullptr;
    void* m_uniformBuffer = nullptr;  // Add uniform buffer for MVP matrix
    void* m_colorBuffer = nullptr;    // Add color buffer for fragment shader
    void* m_faceNormalsBuffer = nullptr;  // Add face normals buffer (6 faces, matching VertexShader)
    bool m_buffersCreated = false;

public:
    MeshRenderer() = default;
    ~MeshRenderer();
    
    void SetMesh(std::shared_ptr<IMesh> mesh);
    std::shared_ptr<IMesh> GetMesh() const { return m_mesh; }
    
    void Initialize() override;
    void Tick(float deltaTime) override;
    void Destroy() override;
    
    void* GetVertexBuffer() const { return m_vertexBuffer; }
    void* GetIndexBuffer() const { return m_indexBuffer; }

private:
    void CreateBuffers();
    void UpdateBuffers();
    void RenderMesh();
    void DestroyBuffers();
};
