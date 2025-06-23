#include "SceneRenderer.h"
#include "TableFloorScene.h"
#include <DebugOutput.h>
#include <OpenXRDebugUtils.h>
#include <fstream>
#include "OpenXRTutorial.h"

#if defined(__ANDROID__)
// Android implementation
std::vector<char> ReadBinaryFile(const char* filename, AAssetManager* assetManager)
{
    // 从 Android asset 管理器中读取文件
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
// Windows/Desktop implementation
std::vector<char> ReadBinaryFile(const char* filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        XR_TUT_LOG_ERROR("Failed to open binary file: " << filename);
        return {};
    }

    const size_t length = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(length);
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), length);
    file.close();
    return buffer;
}
#endif

SceneRenderer::SceneRenderer(GraphicsAPI_Type apiType)
    : m_apiType(apiType)
{
}

SceneRenderer::~SceneRenderer()
{
    // 确保子类在析构函数中调用 DestroyResources()
}

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
    // 桌面平台读取着色器
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
    if (shaderSource.empty()) {
        return nullptr;
    }
    return OpenXRCoreMgr::graphicsAPI->CreateShader({shaderType, shaderSource.data(), shaderSource.size()});
}
