#include "Material.h"
#include <DebugOutput.h>
#include <fstream>
#include <vector>
#include "../../OpenXR/OpenXRCoreMgr.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#include "../../Application/OpenXRTutorial.h"
#endif

Material::Material(const std::string& vertShaderFile, const std::string& fragShaderFile, GraphicsAPI_Type apiType)
    : m_vertShaderFile(vertShaderFile), m_fragShaderFile(fragShaderFile)
{

    XR_TUT_LOG("Creating material with vertex shader: " << vertShaderFile << ", fragment shader: " << fragShaderFile);

    if (apiType == VULKAN)
    {
        m_vertexShader = CreateShaderFromFile(vertShaderFile, GraphicsAPI::ShaderCreateInfo::Type::VERTEX);
        m_fragmentShader = CreateShaderFromFile(fragShaderFile, GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT);
    }
}

Material::~Material()
{
    if (m_vertexShader) OpenXRCoreMgr::GetGraphicsAPI()->DestroyShader(m_vertexShader);
    if (m_fragmentShader) OpenXRCoreMgr::GetGraphicsAPI()->DestroyShader(m_fragmentShader);
}

std::string Material::GetShaderKey() const { return m_vertShaderFile + "_" + m_fragShaderFile; }

void* Material::CreateShaderFromFile(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type)
{
#if defined(__ANDROID__)
    return LoadShaderFromAndroidAssets(filename, type);
#else
    return LoadShaderFromFileSystem(filename, type);
#endif
}

void* Material::LoadShaderFromAndroidAssets(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type)
{
    std::string shaderPath = "shaders/" + filename;
    XR_TUT_LOG("Loading shader from Android assets: " << shaderPath);

    // Check if Android asset manager is available
    if (OpenXRTutorial::androidApp == nullptr || 
        OpenXRTutorial::androidApp->activity == nullptr ||
        OpenXRTutorial::androidApp->activity->assetManager == nullptr)
    {
        XR_TUT_LOG_ERROR("Android asset manager not available");
        return nullptr;
    }

    // Open asset from Android asset manager
    AAsset* asset = AAssetManager_open(OpenXRTutorial::androidApp->activity->assetManager, 
                                       shaderPath.c_str(), 
                                       AASSET_MODE_BUFFER);
    if (!asset)
    {
        XR_TUT_LOG_ERROR("Failed to open Android asset: " << shaderPath);
        return nullptr;
    }

    // Read asset data
    const size_t length = AAsset_getLength(asset);
    if (length == 0)
    {
        XR_TUT_LOG_ERROR("Android asset is empty: " << shaderPath);
        AAsset_close(asset);
        return nullptr;
    }

    std::vector<char> buffer(length);
    int bytesRead = AAsset_read(asset, buffer.data(), length);
    AAsset_close(asset);

    XR_TUT_LOG("Successfully loaded shader from Android assets: " << shaderPath << 
               ", size: " << length << " bytes");

    // Create shader from buffer
    return CreateShaderFromBuffer(buffer, type, shaderPath);
}

void* Material::LoadShaderFromFileSystem(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type)
{
    std::string shaderPath = filename;
    XR_TUT_LOG("Loading shader from file system: " << shaderPath);

    // Open file
    std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        XR_TUT_LOG_ERROR("Failed to open shader file: " << shaderPath);
        return nullptr;
    }

    // Get file size and read data
    size_t fileSize = static_cast<size_t>(file.tellg());

    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    if (!file.good() && !file.eof())
    {
        XR_TUT_LOG_ERROR("Failed to read complete shader file: " << shaderPath);
        return nullptr;
    }

    XR_TUT_LOG("Successfully loaded shader from file system: " << shaderPath << 
               ", size: " << fileSize << " bytes");

    // Create shader from buffer
    return CreateShaderFromBuffer(buffer, type, shaderPath);
}

void* Material::CreateShaderFromBuffer(const std::vector<char>& buffer, 
                                       GraphicsAPI::ShaderCreateInfo::Type type, 
                                       const std::string& shaderPath)
{
    GraphicsAPI::ShaderCreateInfo shaderCreateInfo;
    shaderCreateInfo.type = type;
    shaderCreateInfo.sourceSize = buffer.size();
    shaderCreateInfo.sourceData = buffer.data();

    void* shader = OpenXRCoreMgr::GetGraphicsAPI()->CreateShader(shaderCreateInfo);
    if (!shader)
    {
        XR_TUT_LOG_ERROR("Failed to create shader from buffer: " << shaderPath);
        return nullptr;
    }

    XR_TUT_LOG("Successfully created shader: " << shaderPath);
    return shader;
}
