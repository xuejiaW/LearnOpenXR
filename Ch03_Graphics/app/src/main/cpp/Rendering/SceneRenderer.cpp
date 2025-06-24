#include "SceneRenderer.h"
#include <DebugOutput.h>
#include <OpenXRDebugUtils.h>
#include <fstream>
#include "../Application/OpenXRTutorial.h"
#include "../OpenXR/OpenXRCoreMgr.h"

#if defined(__ANDROID__)
// Android implementation
std::vector<char> ReadBinaryFile(const char* filename, AAssetManager* assetManager)
{
    AAsset* asset = AAssetManager_open(assetManager, filename, AASSET_MODE_BUFFER);
    if (asset == nullptr) {
        XR_TUT_LOG_ERROR("Failed to open binary file: " << filename);
        return {};
    }

    const size_t length = AAsset_getLength(asset);
    std::vector<char> buffer(length);
    AAsset_read(asset, buffer.data(), length);
    AAsset_close(asset);
    return buffer;
}
#else
std::vector<char> ReadBinaryFile(const char* filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        XR_TUT_LOG_ERROR("Failed to open binary file: " << filename);
        return {};
    }

    const size_t length = file.tellg();
    std::vector<char> buffer(length);
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), length);
    file.close();
    return buffer;
}
#endif

SceneRenderer::SceneRenderer(GraphicsAPI_Type apiType)
    : m_apiType(apiType)
{}

SceneRenderer::~SceneRenderer()
{}

std::vector<char> SceneRenderer::ReadShaderFile(const std::string& filename)
{
#if defined(__ANDROID__)
    // Android 平台读取着色器
    if (OpenXRTutorial::androidApp != nullptr && 
        OpenXRTutorial::androidApp->activity != nullptr && 
        OpenXRTutorial::androidApp->activity->assetManager != nullptr) {
        return ReadBinaryFile(filename.c_str(), OpenXRTutorial::androidApp->activity->assetManager);
    }
    return {};
#else
    return ReadBinaryFile(filename.c_str());
#endif
}

void* SceneRenderer::CreateShaderFromFile(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type shaderType)
{
    std::string shaderPath =
#if defined(__ANDROID__)
        "shaders/" + filename;
#else
        filename;
#endif
    std::vector<char> shaderSource = ReadShaderFile(shaderPath);
    if (shaderSource.empty())
    {
        return nullptr;
    }
    return OpenXRCoreMgr::graphicsAPI->CreateShader({shaderType, shaderSource.data(), shaderSource.size()});
}