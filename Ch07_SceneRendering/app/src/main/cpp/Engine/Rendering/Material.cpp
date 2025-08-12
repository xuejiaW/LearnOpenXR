#include "Material.h"
#include <DebugOutput.h>
#include "../Core/Scene.h"
#include "../Components/Rendering/Camera.h"
#include "../../OpenXR/OpenXRCoreMgr.h"
#include "../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "../../OpenXR/OpenXRDisplayMgr.h"
#include "Vertex.h"

Material::Material()
{
}

Material::~Material()
{
    DestroyPipeline();
}

void Material::SetShaders(std::shared_ptr<Shader> vertexShader, std::shared_ptr<Shader> fragmentShader)
{
    if (vertexShader && vertexShader->GetType() != Shader::VERTEX)
    {
        XR_TUT_LOG_ERROR("Material::SetShaders() - First shader must be VERTEX type");
        return;
    }
    
    if (fragmentShader && fragmentShader->GetType() != Shader::FRAGMENT)
    {
        XR_TUT_LOG_ERROR("Material::SetShaders() - Second shader must be FRAGMENT type");
        return;
    }
    
    m_VertexShader = vertexShader;
    m_FragmentShader = fragmentShader;
    
    // Invalidate existing pipeline since shaders changed
    DestroyPipeline();
}

bool Material::IsValid() const
{
    return m_VertexShader && m_VertexShader->IsValid() && 
           m_FragmentShader && m_FragmentShader->IsValid();
}

void* Material::GetOrCreatePipeline()
{
    if (m_Pipeline)
    {
        return m_Pipeline;
    }

    if (!IsValid())
    {
        XR_TUT_LOG_ERROR("Cannot create pipeline: Material shaders are not valid");
        return nullptr;
    }

    m_Pipeline = CreatePipeline();
    return m_Pipeline;
}

void Material::DestroyPipeline()
{
    if (m_Pipeline)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyPipeline(m_Pipeline);
        m_Pipeline = nullptr;
    }
}

Camera* Material::GetActiveCamera()
{
    Camera* activeCamera = Scene::GetActiveCamera();
    if (!activeCamera)
    {
        XR_TUT_LOG_ERROR("Material::GetActiveCamera() - No active camera found in scene");
    }
    return activeCamera;
}

void* Material::CreatePipeline()
{
    Camera* activeCamera = GetActiveCamera();
    if (!activeCamera)
    {
        XR_TUT_LOG_ERROR("Material::CreatePipeline() - No active camera found, using default settings");
    }

    GraphicsAPI::PipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.shaders = {m_VertexShader->GetShaderHandle(), m_FragmentShader->GetShaderHandle()};

    // Vertex input configuration
    pipelineCreateInfo.vertexInputState.attributes.resize(2);
    pipelineCreateInfo.vertexInputState.attributes[0] = {0, 0, GraphicsAPI::VertexType::VEC4, 0, "POSITION"};
    pipelineCreateInfo.vertexInputState.attributes[1] = {1, 0, GraphicsAPI::VertexType::VEC3, sizeof(XrVector4f), "NORMAL"};

    pipelineCreateInfo.vertexInputState.bindings.resize(1);
    pipelineCreateInfo.vertexInputState.bindings[0] = {0, 0, sizeof(Vertex)};

    // Input assembly
    pipelineCreateInfo.inputAssemblyState.topology = GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST;
    pipelineCreateInfo.inputAssemblyState.primitiveRestartEnable = false;

    // Rasterization
    pipelineCreateInfo.rasterisationState.depthClampEnable = false;
    pipelineCreateInfo.rasterisationState.rasteriserDiscardEnable = false;
    pipelineCreateInfo.rasterisationState.polygonMode = GraphicsAPI::PolygonMode::FILL;
    pipelineCreateInfo.rasterisationState.cullMode = GraphicsAPI::CullMode::NONE;
    pipelineCreateInfo.rasterisationState.frontFace = GraphicsAPI::FrontFace::COUNTER_CLOCKWISE;
    pipelineCreateInfo.rasterisationState.depthBiasEnable = false;
    pipelineCreateInfo.rasterisationState.lineWidth = 1.0f;

    // Multisampling
    pipelineCreateInfo.multisampleState.rasterisationSamples = 1;
    pipelineCreateInfo.multisampleState.sampleShadingEnable = false;
    pipelineCreateInfo.multisampleState.minSampleShading = 1.0f;
    pipelineCreateInfo.multisampleState.sampleMask = 0xFFFFFFFF;

    // Depth/Stencil
    pipelineCreateInfo.depthStencilState.depthTestEnable = true;
    pipelineCreateInfo.depthStencilState.depthWriteEnable = true;
    pipelineCreateInfo.depthStencilState.depthCompareOp = GraphicsAPI::CompareOp::LESS_OR_EQUAL;
    pipelineCreateInfo.depthStencilState.depthBoundsTestEnable = false;
    pipelineCreateInfo.depthStencilState.stencilTestEnable = false;

    // Color blending
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

    // Descriptor layout
    pipelineCreateInfo.layout.resize(1);
    pipelineCreateInfo.layout[0].bindingIndex = 0;
    pipelineCreateInfo.layout[0].resource = nullptr;
    pipelineCreateInfo.layout[0].type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    pipelineCreateInfo.layout[0].stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;

    // Render target formats
    pipelineCreateInfo.colorFormats = {OpenXRDisplayMgr::colorSwapchainInfos[0].swapchainFormat};
    pipelineCreateInfo.depthFormat = OpenXRDisplayMgr::depthSwapchainInfos[0].swapchainFormat;

    void* pipeline = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreatePipeline(pipelineCreateInfo);
    if (!pipeline)
    {
        XR_TUT_LOG_ERROR("Failed to create graphics pipeline");
    }
    return pipeline;
}
