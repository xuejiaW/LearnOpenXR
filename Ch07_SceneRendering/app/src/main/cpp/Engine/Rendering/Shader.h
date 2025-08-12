#pragma once

#include <string>
#include <vector>
#include <GraphicsAPI.h>

class Shader {
public:
    enum Type {
        VERTEX,
        FRAGMENT,
        GEOMETRY,
        COMPUTE
    };

    Shader(const std::string& filePath, Type type, GraphicsAPI_Type apiType);
    ~Shader();

    void* GetShaderHandle() const { return m_ShaderHandle; }
    Type GetType() const { return m_Type; }
    const std::string& GetFilePath() const { return m_FilePath; }
    bool IsValid() const { return m_ShaderHandle != nullptr; }

    // Static utility methods
    static std::vector<char> LoadShaderFromFile(const std::string& filePath);
    static void* CreateShaderFromBuffer(const std::vector<char>& buffer, 
                                      GraphicsAPI::ShaderCreateInfo::Type type, 
                                      const std::string& shaderPath);

private:
    void LoadShader();
    GraphicsAPI::ShaderCreateInfo::Type ConvertToAPIType(Type type) const;

#if defined(__ANDROID__)
    void* LoadShaderFromAndroidAssets(const std::string& filename);
#endif
    void* LoadShaderFromFileSystem(const std::string& filename);

    std::string m_FilePath;
    Type m_Type;
    GraphicsAPI_Type m_ApiType;
    void* m_ShaderHandle = nullptr;
};
