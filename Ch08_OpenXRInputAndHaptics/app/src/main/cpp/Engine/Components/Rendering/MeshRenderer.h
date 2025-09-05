#pragma once

#include "../../Core/IComponent.h"
#include "../../Rendering/Mesh/IMesh.h"
#include "../../Rendering/Material.h"
#include <memory>

class MeshRenderer : public IComponent {

public:
    MeshRenderer() = default;
    ~MeshRenderer() override;
    
    void SetMesh(std::shared_ptr<IMesh> mesh);
    std::shared_ptr<IMesh> GetMesh() const { return m_Mesh; }
    
    void SetMaterial(std::shared_ptr<Material> material);
    std::shared_ptr<Material> GetMaterial() const { return m_Material; }
    
    void Initialize() override;
    void Tick(float deltaTime) override;
    void Destroy() override;

private:
    void CreateBuffers();
    void RenderMesh();
    void DestroyBuffers();
    
    std::shared_ptr<IMesh> m_Mesh;
    std::shared_ptr<Material> m_Material;
    void* m_VertexBuffer = nullptr;
    void* m_IndexBuffer = nullptr;
    void* m_UniformBuffer = nullptr;
    bool m_BuffersCreated = false;
};
