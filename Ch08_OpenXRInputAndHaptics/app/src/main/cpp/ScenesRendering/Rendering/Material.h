#pragma once

#include "../Core/IComponent.h"
#include <string>
#include <vector>
#include <GraphicsAPI.h>

// Forward declaration
class Camera;

class Material : public IComponent {
public:
    Material(const std::string& vertShaderFile, const std::string& fragShaderFile, GraphicsAPI_Type apiType);
    ~Material();
    
    void* GetVertexShader() const { return m_vertexShader; }
    void* GetFragmentShader() const { return m_fragmentShader; }
    std::string GetShaderKey() const;
    
    // Pipeline management
    void* GetOrCreatePipeline();
    
    void Initialize() override;
    void Destroy() override;
    
private:
    void* m_vertexShader = nullptr;
    void* m_fragmentShader = nullptr;
    void* m_pipeline = nullptr;  // Cached pipeline
    std::string m_vertShaderFile;
    std::string m_fragShaderFile;
    GraphicsAPI_Type m_apiType;
    
    void* CreateShaderFromFile(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* LoadShaderFromAndroidAssets(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* LoadShaderFromFileSystem(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* CreateShaderFromBuffer(const std::vector<char>& buffer, GraphicsAPI::ShaderCreateInfo::Type type, const std::string& shaderPath);
    void* CreatePipeline();
    
    // Helper method to get camera render settings for pipeline creation
    Camera* GetActiveCamera();
};
