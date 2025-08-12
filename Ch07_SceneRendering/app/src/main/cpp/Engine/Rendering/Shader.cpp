#include "Shader.h"
#include <DebugOutput.h>
#include <fstream>
#include "../../OpenXR/OpenXRCoreMgr.h"
#include "../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#include "../../Application/OpenXRTutorial.h"
#endif

Shader::Shader(const std::string& filePath, Type type, GraphicsAPI_Type apiType)
    : m_FilePath(filePath), m_Type(type), m_ApiType(apiType)
{
    LoadShader();
}

Shader::~Shader()
{
    if (m_ShaderHandle)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyShader(m_ShaderHandle);
        m_ShaderHandle = nullptr;
    }
}

void Shader::LoadShader()
{
    if (m_ApiType == VULKAN)
    {
#if defined(__ANDROID__)
        m_ShaderHandle = LoadShaderFromAndroidAssets(m_FilePath);
#else
        m_ShaderHandle = LoadShaderFromFileSystem(m_FilePath);
#endif
    }
}

GraphicsAPI::ShaderCreateInfo::Type Shader::ConvertToAPIType(Type type) const
{
    switch (type)
    {
        case VERTEX: return GraphicsAPI::ShaderCreateInfo::Type::VERTEX;
        case FRAGMENT: return GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT;
        case GEOMETRY: return GraphicsAPI::ShaderCreateInfo::Type::GEOMETRY;
        case COMPUTE: return GraphicsAPI::ShaderCreateInfo::Type::COMPUTE;
        default: return GraphicsAPI::ShaderCreateInfo::Type::VERTEX;
    }
}

#if defined(__ANDROID__)
void* Shader::LoadShaderFromAndroidAssets(const std::string& filename)
{
    AAssetManager* assetManager = OpenXRTutorial::androidApp->activity->assetManager;
    AAsset* asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_BUFFER);
    if (!asset)
    {
        XR_TUT_LOG_ERROR("Failed to open asset: " << filename);
        return nullptr;
    }

    size_t assetSize = AAsset_getLength(asset);
    std::vector<char> buffer(assetSize);
    AAsset_read(asset, buffer.data(), assetSize);
    AAsset_close(asset);

    return CreateShaderFromBuffer(buffer, ConvertToAPIType(m_Type), filename);
}
#endif

void* Shader::LoadShaderFromFileSystem(const std::string& filename)
{
    std::vector<char> buffer = LoadShaderFromFile(filename);
    if (buffer.empty())
    {
        return nullptr;
    }
    return CreateShaderFromBuffer(buffer, ConvertToAPIType(m_Type), filename);
}

std::vector<char> Shader::LoadShaderFromFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        XR_TUT_LOG_ERROR("Failed to open shader file: " << filePath);
        return {};
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void* Shader::CreateShaderFromBuffer(const std::vector<char>& buffer, 
                                   GraphicsAPI::ShaderCreateInfo::Type type, 
                                   const std::string& shaderPath)
{
    GraphicsAPI::ShaderCreateInfo shaderCreateInfo;
    shaderCreateInfo.type = type;
    shaderCreateInfo.sourceSize = buffer.size();
    shaderCreateInfo.sourceData = buffer.data();

    void* shader = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateShader(shaderCreateInfo);
    if (!shader)
    {
        XR_TUT_LOG_ERROR("Failed to create shader from: " << shaderPath);
    }
    return shader;
}
