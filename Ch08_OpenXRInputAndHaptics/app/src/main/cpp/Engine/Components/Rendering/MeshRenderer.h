#pragma once

#include "../../Core/IComponent.h"
#include "../../Rendering/Mesh/IMesh.h"
#include <memory>

class MeshRenderer : public IComponent {

public:
    MeshRenderer() = default;
    ~MeshRenderer() override;
    
    void SetMesh(std::shared_ptr<IMesh> mesh);
    std::shared_ptr<IMesh> GetMesh() const { return m_Mesh; }
    
    void Initialize() override;
    void Tick(float deltaTime) override;
    void Destroy() override;

private:
    void CreateBuffers();
    void RenderMesh();
    void DestroyBuffers();
    std::shared_ptr<IMesh> m_Mesh;
    void* m_VertexBuffer = nullptr;
    void* m_IndexBuffer = nullptr;
    void* m_UniformBuffer = nullptr;
    bool m_BuffersCreated = false;
};
