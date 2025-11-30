#pragma once

#include "../../Core/IComponent.h"
#include <string>
#include <vector>
#include <GraphicsAPI.h>
#include <openxr/openxr.h>

class Camera;

class Material : public IComponent {
public:
    Material(const std::string& vertShaderFile, const std::string& fragShaderFile, GraphicsAPI_Type apiType);
    ~Material() override;
    
    void* GetVertexShader() const { return m_VertexShader; }
    void* GetFragmentShader() const { return m_FragmentShader; }
    
    void SetColor(const XrVector4f& color) { m_Color = color; }
    const XrVector4f& GetColor() const { return m_Color; }
    void SetColor(float r, float g, float b, float a = 1.0f) { 
        m_Color = {r, g, b, a}; 
    }
    
    void* GetOrCreatePipeline();
    
    void Initialize() override;
    void Destroy() override;
    
private:
    void* m_VertexShader = nullptr;
    void* m_FragmentShader = nullptr;
    void* m_Pipeline = nullptr;
    std::string m_VertShaderFile;
    std::string m_FragShaderFile;
    GraphicsAPI_Type m_ApiType;
    XrVector4f m_Color = {1.0f, 1.0f, 1.0f, 1.0f};
    
    void* CreateShaderFromFile(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
#if defined(__ANDROID__)
    void* LoadShaderFromAndroidAssets(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
#endif
    
    void* LoadShaderFromFileSystem(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* CreateShaderFromBuffer(const std::vector<char>& buffer, GraphicsAPI::ShaderCreateInfo::Type type, const std::string& shaderPath);
    void* CreatePipeline();
    
    Camera* GetActiveCamera();
};
