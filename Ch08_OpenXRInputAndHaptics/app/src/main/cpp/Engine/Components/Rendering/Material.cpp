#include "Material.h"
#include <DebugOutput.h>
#include <fstream>
#include <vector>
#include "../../../OpenXR/OpenXRCoreMgr.h"
#include "../../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "../../../OpenXR/OpenXRDisplayMgr.h"
#include "Camera.h"
#include "../../Core/Scene.h"
#include "../../Core/GameObject.h"
#include "../../Rendering/Vertex.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#include "../../../Application/OpenXRTutorial.h"
#endif

Material::Material(const std::string& vertShaderFile, const std::string& fragShaderFile, GraphicsAPI_Type apiType)
    : m_VertShaderFile(vertShaderFile), m_FragShaderFile(fragShaderFile), m_ApiType(apiType)
{
}

Material::~Material()
{
    if (m_VertexShader) OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyShader(m_VertexShader);
    if (m_FragmentShader) OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyShader(m_FragmentShader);
}

void Material::Initialize() {
    if (m_ApiType == VULKAN) {
        m_VertexShader = CreateShaderFromFile(m_VertShaderFile, GraphicsAPI::ShaderCreateInfo::Type::VERTEX);
        m_FragmentShader = CreateShaderFromFile(m_FragShaderFile, GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT);
    }
}

void Material::Destroy() {
    if (m_VertexShader) {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyShader(m_VertexShader);
        m_VertexShader = nullptr;
    }
    if (m_FragmentShader) {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyShader(m_FragmentShader);
        m_FragmentShader = nullptr;
    }
    if (m_Pipeline) {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyPipeline(m_Pipeline);
        m_Pipeline = nullptr;
    }
}

void* Material::GetOrCreatePipeline() {
    if (m_Pipeline) {
        return m_Pipeline;
    }
    
    if (!m_VertexShader || !m_FragmentShader) {
        return nullptr;
    }
    
    m_Pipeline = CreatePipeline();
    return m_Pipeline;
}

Camera* Material::GetActiveCamera() {
    Camera* activeCamera = Scene::GetActiveCamera();
    if (!activeCamera) {
        XR_TUT_LOG_ERROR("Material::GetActiveCamera() - No active camera found in scene");
    }
    return activeCamera;
}

void* Material::CreatePipeline() {
    Camera* activeCamera = GetActiveCamera();
    if (!activeCamera) {
        XR_TUT_LOG_ERROR("Material::CreatePipeline() - No active camera found, using default settings");
    }
    
    GraphicsAPI::PipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.shaders = {m_VertexShader, m_FragmentShader};

    pipelineCreateInfo.vertexInputState.attributes.resize(2);
    pipelineCreateInfo.vertexInputState.attributes[0] = {0, 0, GraphicsAPI::VertexType::VEC4, 0, "POSITION"};
    pipelineCreateInfo.vertexInputState.attributes[1] = {1, 0, GraphicsAPI::VertexType::VEC3, sizeof(XrVector4f), "NORMAL"};
    
    pipelineCreateInfo.vertexInputState.bindings.resize(1);
    pipelineCreateInfo.vertexInputState.bindings[0] = {0, 0, sizeof(Vertex)};

    pipelineCreateInfo.inputAssemblyState.topology = GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST;
    pipelineCreateInfo.inputAssemblyState.primitiveRestartEnable = false;

    pipelineCreateInfo.rasterisationState.depthClampEnable = false;
    pipelineCreateInfo.rasterisationState.rasteriserDiscardEnable = false;
    pipelineCreateInfo.rasterisationState.polygonMode = GraphicsAPI::PolygonMode::FILL;
    pipelineCreateInfo.rasterisationState.cullMode = GraphicsAPI::CullMode::NONE;
    pipelineCreateInfo.rasterisationState.frontFace = GraphicsAPI::FrontFace::COUNTER_CLOCKWISE;
    pipelineCreateInfo.rasterisationState.depthBiasEnable = false;
    pipelineCreateInfo.rasterisationState.lineWidth = 1.0f;

    pipelineCreateInfo.multisampleState.rasterisationSamples = 1;
    pipelineCreateInfo.multisampleState.sampleShadingEnable = false;
    pipelineCreateInfo.multisampleState.minSampleShading = 1.0f;
    pipelineCreateInfo.multisampleState.sampleMask = 0xFFFFFFFF;

    pipelineCreateInfo.depthStencilState.depthTestEnable = true;
    pipelineCreateInfo.depthStencilState.depthWriteEnable = true;
    pipelineCreateInfo.depthStencilState.depthCompareOp = GraphicsAPI::CompareOp::LESS_OR_EQUAL;
    pipelineCreateInfo.depthStencilState.depthBoundsTestEnable = false;
    pipelineCreateInfo.depthStencilState.stencilTestEnable = false;

    pipelineCreateInfo.colorBlendState.logicOpEnable = false;
    pipelineCreateInfo.colorBlendState.logicOp = GraphicsAPI::LogicOp::NO_OP;
    pipelineCreateInfo.colorBlendState.attachments.resize(1);
    pipelineCreateInfo.colorBlendState.attachments[0].colorWriteMask = static_cast<GraphicsAPI::ColorComponentBit>(
        static_cast<uint8_t>(GraphicsAPI::ColorComponentBit::R_BIT) |
        static_cast<uint8_t>(GraphicsAPI::ColorComponentBit::G_BIT) |
        static_cast<uint8_t>(GraphicsAPI::ColorComponentBit::B_BIT) |
        static_cast<uint8_t>(GraphicsAPI::ColorComponentBit::A_BIT)
    );
    pipelineCreateInfo.colorBlendState.attachments[0].blendEnable = false;

    pipelineCreateInfo.layout.resize(2);
    
    pipelineCreateInfo.layout[0].bindingIndex = 0;
    pipelineCreateInfo.layout[0].resource = nullptr;
    pipelineCreateInfo.layout[0].type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    pipelineCreateInfo.layout[0].stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    
    pipelineCreateInfo.layout[1].bindingIndex = 1;
    pipelineCreateInfo.layout[1].resource = nullptr;
    pipelineCreateInfo.layout[1].type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    pipelineCreateInfo.layout[1].stage = GraphicsAPI::DescriptorInfo::Stage::FRAGMENT;

    pipelineCreateInfo.colorFormats = {OpenXRDisplayMgr::colorSwapchainInfos[0].swapchainFormat};
    pipelineCreateInfo.depthFormat = OpenXRDisplayMgr::depthSwapchainInfos[0].swapchainFormat;

    void* pipeline = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreatePipeline(pipelineCreateInfo);
    return pipeline;
}


void* Material::CreateShaderFromFile(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type)
{
#if defined(__ANDROID__)
    return LoadShaderFromAndroidAssets(filename, type);
#else
    return LoadShaderFromFileSystem(filename, type);
#endif
}

#if defined(__ANDROID__)
void* Material::LoadShaderFromAndroidAssets(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type)
{
    std::string shaderPath = "shaders/" + filename;

    if (OpenXRTutorial::androidApp == nullptr || OpenXRTutorial::androidApp->activity == nullptr || OpenXRTutorial::androidApp->activity->assetManager == nullptr)
    {
        XR_TUT_LOG_ERROR("Android asset manager not available");
        return nullptr;
    }

    AAsset* asset = AAssetManager_open(OpenXRTutorial::androidApp->activity->assetManager, shaderPath.c_str(), AASSET_MODE_BUFFER);
    if (!asset)
    {
        XR_TUT_LOG_ERROR("Failed to open Android asset: " << shaderPath);
        return nullptr;
    }

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

    return CreateShaderFromBuffer(buffer, type, shaderPath);
}
#endif

void* Material::LoadShaderFromFileSystem(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type)
{
    std::string shaderPath = filename;

    std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        XR_TUT_LOG_ERROR("Failed to open shader file: " << shaderPath);
        return nullptr;
    }

    size_t fileSize = file.tellg();

    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    if (!file.good() && !file.eof())
    {
        XR_TUT_LOG_ERROR("Failed to read complete shader file: " << shaderPath);
        return nullptr;
    }

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

    void* shader = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateShader(shaderCreateInfo);
    if (!shader)
    {
        XR_TUT_LOG_ERROR("Failed to create shader from buffer: " << shaderPath);
        return nullptr;
    }

    return shader;
}