#pragma once

#include <memory>
#include <openxr/openxr.h>
#include <GraphicsAPI.h>
#include "Shader.h"

class Camera;

class Material {
public:
    Material();
    ~Material();
    
    // Shader management
    void SetShaders(std::shared_ptr<Shader> vertexShader, std::shared_ptr<Shader> fragmentShader);
    std::shared_ptr<Shader> GetVertexShader() const { return m_VertexShader; }
    std::shared_ptr<Shader> GetFragmentShader() const { return m_FragmentShader; }
    
    // Color properties
    void SetColor(const XrVector4f& color) { m_Color = color; }
    const XrVector4f& GetColor() const { return m_Color; }
    void SetColor(float r, float g, float b, float a = 1.0f) { 
        m_Color = {r, g, b, a}; 
    }
    
    // Pipeline management
    void* GetOrCreatePipeline();
    void DestroyPipeline();
    
    // Validation
    bool IsValid() const;
    
private:
    std::shared_ptr<Shader> m_VertexShader;
    std::shared_ptr<Shader> m_FragmentShader;
    void* m_Pipeline = nullptr;
    XrVector4f m_Color = {1.0f, 1.0f, 1.0f, 1.0f};
    
    void* CreatePipeline();
    Camera* GetActiveCamera();
};
