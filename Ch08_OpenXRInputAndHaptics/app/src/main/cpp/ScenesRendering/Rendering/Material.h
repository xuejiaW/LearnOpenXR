#pragma once

#include <string>
#include <vector>
#include <GraphicsAPI.h>

class Material {
public:
    Material(const std::string& vertShaderFile, const std::string& fragShaderFile, GraphicsAPI_Type apiType);
    ~Material();
    
    void* GetVertexShader() const { return m_vertexShader; }
    void* GetFragmentShader() const { return m_fragmentShader; }
    std::string GetShaderKey() const;
    
private:
    void* m_vertexShader = nullptr;
    void* m_fragmentShader = nullptr;
    std::string m_vertShaderFile;
    std::string m_fragShaderFile;
    
    void* CreateShaderFromFile(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* LoadShaderFromAndroidAssets(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* LoadShaderFromFileSystem(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type);
    void* CreateShaderFromBuffer(const std::vector<char>& buffer, GraphicsAPI::ShaderCreateInfo::Type type, const std::string& shaderPath);
};
