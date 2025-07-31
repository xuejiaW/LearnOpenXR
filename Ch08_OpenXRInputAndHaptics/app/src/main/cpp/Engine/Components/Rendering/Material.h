#pragma once

#include "../../Core/IComponent.h"
#include <string>
#include <vector>
#include <GraphicsAPI.h>
#include <openxr/openxr.h>

// Forward declaration
class Camera;

class Material : public IComponent {
public:
    Material(const std::string& vertShaderFile, const std::string& fragShaderFile, GraphicsAPI_Type apiType);
    ~Material() override;
    
    void* GetVertexShader() const { return m_vertexShader; }
    void* GetFragmentShader() const { return m_fragmentShader; }
    std::string GetShaderKey() const;
    
    // Color management
    void SetColor(const XrVector4f& color) { m_color = color; }
    const XrVector4f& GetColor() const { return m_color; }
    void SetColor(float r, float g, float b, float a = 1.0f) { 
        m_color = {r, g, b, a}; 
    }
    
    void* GetOrCreatePipeline();
    
    void Initialize() override;
    void Destroy() override;
    
private:
    void* m_vertexShader = nullptr;
    void* m_fragmentShader = nullptr;
    void* m_pipeline = nullptr;
    std::string m_vertShaderFile;
    std::string m_fragShaderFile;
    GraphicsAPI_Type m_apiType;
    XrVector4f m_color = {1.0f, 1.0f, 1.0f, 1.0f};
    
    void* CreateShaderFromFile(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* LoadShaderFromAndroidAssets(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* LoadShaderFromFileSystem(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* CreateShaderFromBuffer(const std::vector<char>& buffer, GraphicsAPI::ShaderCreateInfo::Type type, const std::string& shaderPath);
    void* CreatePipeline();
    
    Camera* GetActiveCamera();
};
