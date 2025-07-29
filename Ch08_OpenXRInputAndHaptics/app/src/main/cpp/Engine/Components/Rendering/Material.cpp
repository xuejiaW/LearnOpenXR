#include "Material.h"
#include <DebugOutput.h>
#include <fstream>
#include <vector>
#include "../../../OpenXR/OpenXRCoreMgr.h"
#include "../../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "../../../OpenXR/OpenXRDisplayMgr.h"  // Add this for swapchain format access
#include "../../../Application/OpenXRTutorial.h"
#include "Camera.h"
#include "../../Core/Scene.h"
#include "../../Core/GameObject.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#include "../../../Application/OpenXRTutorial.h"
#endif

Material::Material(const std::string& vertShaderFile, const std::string& fragShaderFile, GraphicsAPI_Type apiType)
    : m_vertShaderFile(vertShaderFile), m_fragShaderFile(fragShaderFile), m_apiType(apiType)
{
    // Move shader creation to Initialize() method
}

Material::~Material()
{
    if (m_vertexShader) OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyShader(m_vertexShader);
    if (m_fragmentShader) OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyShader(m_fragmentShader);
}

void Material::Initialize() {
    XR_TUT_LOG("Creating material with vertex shader: " << m_vertShaderFile << ", fragment shader: " << m_fragShaderFile);

    if (m_apiType == VULKAN) {
        m_vertexShader = CreateShaderFromFile(m_vertShaderFile, GraphicsAPI::ShaderCreateInfo::Type::VERTEX);
        m_fragmentShader = CreateShaderFromFile(m_fragShaderFile, GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT);
    }
}

void Material::Destroy() {
    if (m_vertexShader) {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyShader(m_vertexShader);
        m_vertexShader = nullptr;
    }
    if (m_fragmentShader) {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyShader(m_fragmentShader);
        m_fragmentShader = nullptr;
    }
    if (m_pipeline) {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyPipeline(m_pipeline);
        m_pipeline = nullptr;
    }
}

void* Material::GetOrCreatePipeline() {
    if (m_pipeline) {
        return m_pipeline;
    }
    
    if (!m_vertexShader || !m_fragmentShader) {
        return nullptr;
    }
    
    m_pipeline = CreatePipeline();
    return m_pipeline;
}

Camera* Material::GetActiveCamera() {
    // Use Scene's static method to get active camera
    Camera* activeCamera = Scene::GetActiveCamera();
    if (!activeCamera) {
        XR_TUT_LOG_ERROR("Material::GetActiveCamera() - No active camera found in scene");
    }
    return activeCamera;
}

void* Material::CreatePipeline() {
    XR_TUT_LOG("Material::CreatePipeline() - Creating pipeline with camera render settings");
    
    // Get active camera to retrieve render settings
    Camera* activeCamera = GetActiveCamera();
    if (!activeCamera) {
        XR_TUT_LOG_ERROR("Material::CreatePipeline() - No active camera found, using default settings");
        // Fall back to original pipeline creation without camera settings
    }
    
    GraphicsAPI::PipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.shaders = {m_vertexShader, m_fragmentShader};

    // Get render settings from camera if available
    uint32_t renderWidth = 1024;
    uint32_t renderHeight = 1024;
    if (activeCamera) {
        const Camera::RenderSettings& cameraSettings = activeCamera->GetRenderSettings();
        if (cameraSettings.width > 0 && cameraSettings.height > 0) {
            renderWidth = cameraSettings.width;
            renderHeight = cameraSettings.height;
            XR_TUT_LOG("Material::CreatePipeline() - Using camera render size: " << renderWidth << "x" << renderHeight);
        }
    }

    // Vertex input state - CRITICAL FIX: Match SceneRenderer exactly (only 1 vertex attribute)
    // VertexShader only has 1 input: layout(location = 0) in vec4 a_Positions
    pipelineCreateInfo.vertexInputState.attributes.resize(1);  // Only 1 attribute like SceneRenderer
    pipelineCreateInfo.vertexInputState.attributes[0] = {0, 0, GraphicsAPI::VertexType::VEC4, 0, "TEXCOORD"};  // Position only
    
    pipelineCreateInfo.vertexInputState.bindings.resize(1);  // Only 1 binding like SceneRenderer  
    pipelineCreateInfo.vertexInputState.bindings[0] = {0, 0, 4 * sizeof(float)};  // Position buffer only

    // Input assembly state
    pipelineCreateInfo.inputAssemblyState.topology = GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST;
    pipelineCreateInfo.inputAssemblyState.primitiveRestartEnable = false;

    // Rasterization state
    pipelineCreateInfo.rasterisationState.depthClampEnable = false;
    pipelineCreateInfo.rasterisationState.rasteriserDiscardEnable = false;
    pipelineCreateInfo.rasterisationState.polygonMode = GraphicsAPI::PolygonMode::FILL;
    pipelineCreateInfo.rasterisationState.cullMode = GraphicsAPI::CullMode::NONE;  // Disable culling for debugging
    pipelineCreateInfo.rasterisationState.frontFace = GraphicsAPI::FrontFace::COUNTER_CLOCKWISE;
    pipelineCreateInfo.rasterisationState.depthBiasEnable = false;
    pipelineCreateInfo.rasterisationState.lineWidth = 1.0f;

    // Multisample state
    pipelineCreateInfo.multisampleState.rasterisationSamples = 1;
    pipelineCreateInfo.multisampleState.sampleShadingEnable = false;
    pipelineCreateInfo.multisampleState.minSampleShading = 1.0f;
    pipelineCreateInfo.multisampleState.sampleMask = 0xFFFFFFFF;

    // Depth stencil state
    pipelineCreateInfo.depthStencilState.depthTestEnable = true;
    pipelineCreateInfo.depthStencilState.depthWriteEnable = true;
    pipelineCreateInfo.depthStencilState.depthCompareOp = GraphicsAPI::CompareOp::LESS_OR_EQUAL;
    pipelineCreateInfo.depthStencilState.depthBoundsTestEnable = false;
    pipelineCreateInfo.depthStencilState.stencilTestEnable = false;

    // Color blend state
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

    // CRITICAL MISSING: Add DescriptorLayout configuration exactly like SceneRenderer
    // SceneRenderer defines 3 descriptor bindings (0,1,2) - we must match this exactly!
    pipelineCreateInfo.layout.resize(3);
    
    pipelineCreateInfo.layout[0].bindingIndex = 0;
    pipelineCreateInfo.layout[0].resource = nullptr;
    pipelineCreateInfo.layout[0].type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    pipelineCreateInfo.layout[0].stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    
    pipelineCreateInfo.layout[1].bindingIndex = 1;
    pipelineCreateInfo.layout[1].resource = nullptr;
    pipelineCreateInfo.layout[1].type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    pipelineCreateInfo.layout[1].stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    
    pipelineCreateInfo.layout[2].bindingIndex = 2;
    pipelineCreateInfo.layout[2].resource = nullptr;
    pipelineCreateInfo.layout[2].type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    pipelineCreateInfo.layout[2].stage = GraphicsAPI::DescriptorInfo::Stage::FRAGMENT;

    // Add color formats and depth format like SceneRenderer
    // Note: These should match the actual swapchain formats used
    pipelineCreateInfo.colorFormats = {OpenXRDisplayMgr::colorSwapchainInfos[0].swapchainFormat};
    pipelineCreateInfo.depthFormat = OpenXRDisplayMgr::depthSwapchainInfos[0].swapchainFormat;

    void* pipeline = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreatePipeline(pipelineCreateInfo);
    XR_TUT_LOG("Material::CreatePipeline() - Pipeline created: " << pipeline);
    return pipeline;
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

#if defined(__ANDROID__)
void* Material::LoadShaderFromAndroidAssets(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type)
{
    std::string shaderPath = "shaders/" + filename;

    // Check if Android asset manager is available
    if (OpenXRTutorial::androidApp == nullptr || OpenXRTutorial::androidApp->activity == nullptr || OpenXRTutorial::androidApp->activity->assetManager == nullptr)
    {
        XR_TUT_LOG_ERROR("Android asset manager not available");
        return nullptr;
    }

    // Open asset from Android asset manager
    AAsset* asset = AAssetManager_open(OpenXRTutorial::androidApp->activity->assetManager, shaderPath.c_str(), AASSET_MODE_BUFFER);
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

    // Create shader from buffer
    return CreateShaderFromBuffer(buffer, type, shaderPath);
}
#endif

void* Material::LoadShaderFromFileSystem(const std::string& filename, GraphicsAPI::ShaderCreateInfo::Type type)
{
    std::string shaderPath = filename;

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

    void* shader = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateShader(shaderCreateInfo);
    if (!shader)
    {
        XR_TUT_LOG_ERROR("Failed to create shader from buffer: " << shaderPath);
        return nullptr;
    }

    XR_TUT_LOG("Successfully created shader: " << shaderPath);
    return shader;
}