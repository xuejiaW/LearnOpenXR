#include "SceneRenderer.h"
#include "../../OpenXR/OpenXRCoreMgr.h"
#include "../../OpenXR/OpenXRDisplayMgr.h"
#include "../Rendering/Geometry/IRenderableGeometry.h"
#include "../Rendering/Material.h"
#include "IScene.h"


SceneRenderer::SceneRenderer(GraphicsAPI_Type apiType) : m_apiType(apiType) {}

SceneRenderer::~SceneRenderer() { DestroyResources(); }

void SceneRenderer::SetScene(std::shared_ptr<IScene> scene) { m_scene = scene; }

void SceneRenderer::CreateResources()
{
    if (!m_scene) return;

    const auto& objects = m_scene->GetObjects();
    size_t numberOfObjects = objects.size();

    GraphicsAPI::BufferCreateInfo objectDataBufferInfo;
    objectDataBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::UNIFORM;
    objectDataBufferInfo.stride = 0;
    objectDataBufferInfo.size = sizeof(ObjectRenderData) * numberOfObjects;
    objectDataBufferInfo.data = nullptr;
    m_uniformBuffer_ObjectData = OpenXRCoreMgr::GetGraphicsAPI()->CreateBuffer(objectDataBufferInfo);
}

void SceneRenderer::Render(const XrMatrix4x4f& viewProj)
{
    if (!m_scene) return;

    const auto& objects = m_scene->GetObjects();
    for (size_t i = 0; i < objects.size(); ++i)
    {
        RenderObject(objects[i], viewProj, i);
    }
}

void SceneRenderer::RenderObject(const struct SceneObject& object, const XrMatrix4x4f& viewProj, size_t objectIndex)
{
    ObjectRenderData renderData;
    renderData.viewProj = viewProj;
    XrMatrix4x4f_CreateTranslationRotationScale(&renderData.model, &object.pose.position, &object.pose.orientation, &object.scale);
    XrMatrix4x4f_Multiply(&renderData.modelViewProj, &renderData.viewProj, &renderData.model);
    renderData.color = {object.color.x, object.color.y, object.color.z, 1.0f};

    size_t objectDataOffset = sizeof(ObjectRenderData) * objectIndex;
    void* pipeline = GetOrCreatePipeline(object.material);

    OpenXRCoreMgr::GetGraphicsAPI()->SetPipeline(pipeline);
    OpenXRCoreMgr::GetGraphicsAPI()->SetBufferData(m_uniformBuffer_ObjectData, objectDataOffset, sizeof(ObjectRenderData), &renderData);

    GraphicsAPI::DescriptorInfo objectDataDescriptor;
    objectDataDescriptor.bindingIndex = 0;
    objectDataDescriptor.resource = m_uniformBuffer_ObjectData;
    objectDataDescriptor.type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    objectDataDescriptor.stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    objectDataDescriptor.readWrite = false;
    objectDataDescriptor.bufferOffset = objectDataOffset;
    objectDataDescriptor.bufferSize = sizeof(ObjectRenderData);
    OpenXRCoreMgr::GetGraphicsAPI()->SetDescriptor(objectDataDescriptor);

    // Use geometry-specific normals buffer
    GraphicsAPI::DescriptorInfo normalsDescriptor;
    normalsDescriptor.bindingIndex = 1;
    normalsDescriptor.resource = object.geometry->GetNormalsBuffer();
    normalsDescriptor.type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    normalsDescriptor.stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    normalsDescriptor.readWrite = false;
    normalsDescriptor.bufferOffset = 0;
    normalsDescriptor.bufferSize = sizeof(XrVector4f) * object.geometry->GetNormals().size();
    OpenXRCoreMgr::GetGraphicsAPI()->SetDescriptor(normalsDescriptor);

    OpenXRCoreMgr::GetGraphicsAPI()->UpdateDescriptors();

    void* vertexBuffer = object.geometry->GetVertexBuffer();
    void* indexBuffer = object.geometry->GetIndexBuffer();
    uint32_t indexCount = object.geometry->GetIndexCount();

    OpenXRCoreMgr::GetGraphicsAPI()->SetVertexBuffers(&vertexBuffer, 1);
    OpenXRCoreMgr::GetGraphicsAPI()->SetIndexBuffer(indexBuffer);
    OpenXRCoreMgr::GetGraphicsAPI()->DrawIndexed(indexCount);
}

void* SceneRenderer::GetOrCreatePipeline(std::shared_ptr<Material> material)
{
    std::string materialKey = material->GetShaderKey();
    auto it = m_renderPipelines.find(materialKey);
    if (it != m_renderPipelines.end())
    {
        return it->second;
    }

    void* vertexShader = material->GetVertexShader();
    void* fragmentShader = material->GetFragmentShader();

    if (!vertexShader || !fragmentShader)
    {
        return nullptr;
    }

    GraphicsAPI::PipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.shaders = {vertexShader, fragmentShader};

    pipelineCreateInfo.vertexInputState.attributes.resize(1);
    pipelineCreateInfo.vertexInputState.attributes[0] = {0, 0, GraphicsAPI::VertexType::VEC4, 0, "TEXCOORD"};
    pipelineCreateInfo.vertexInputState.bindings.resize(1);
    pipelineCreateInfo.vertexInputState.bindings[0] = {0, 0, 4 * sizeof(float)};

    pipelineCreateInfo.inputAssemblyState.topology = GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST;
    pipelineCreateInfo.inputAssemblyState.primitiveRestartEnable = false;

    pipelineCreateInfo.rasterisationState.depthClampEnable = false;
    pipelineCreateInfo.rasterisationState.rasteriserDiscardEnable = false;
    pipelineCreateInfo.rasterisationState.polygonMode = GraphicsAPI::PolygonMode::FILL;
    pipelineCreateInfo.rasterisationState.cullMode = GraphicsAPI::CullMode::BACK;
    pipelineCreateInfo.rasterisationState.frontFace = GraphicsAPI::FrontFace::COUNTER_CLOCKWISE;
    pipelineCreateInfo.rasterisationState.depthBiasEnable = false;
    pipelineCreateInfo.rasterisationState.depthBiasConstantFactor = 0.0f;
    pipelineCreateInfo.rasterisationState.depthBiasClamp = 0.0f;
    pipelineCreateInfo.rasterisationState.depthBiasSlopeFactor = 0.0f;
    pipelineCreateInfo.rasterisationState.lineWidth = 1.0f;

    pipelineCreateInfo.multisampleState.rasterisationSamples = 1;
    pipelineCreateInfo.multisampleState.sampleShadingEnable = false;
    pipelineCreateInfo.multisampleState.minSampleShading = 1.0f;
    pipelineCreateInfo.multisampleState.sampleMask = 0xFFFFFFFF;
    pipelineCreateInfo.multisampleState.alphaToCoverageEnable = false;
    pipelineCreateInfo.multisampleState.alphaToOneEnable = false;

    pipelineCreateInfo.depthStencilState.depthTestEnable = true;
    pipelineCreateInfo.depthStencilState.depthWriteEnable = true;
    pipelineCreateInfo.depthStencilState.depthCompareOp = GraphicsAPI::CompareOp::LESS_OR_EQUAL;
    pipelineCreateInfo.depthStencilState.depthBoundsTestEnable = false;
    pipelineCreateInfo.depthStencilState.stencilTestEnable = false;
    pipelineCreateInfo.depthStencilState.front = {};
    pipelineCreateInfo.depthStencilState.back = {};
    pipelineCreateInfo.depthStencilState.minDepthBounds = 0.0f;
    pipelineCreateInfo.depthStencilState.maxDepthBounds = 1.0f;

    pipelineCreateInfo.colorBlendState.logicOpEnable = false;
    pipelineCreateInfo.colorBlendState.logicOp = GraphicsAPI::LogicOp::NO_OP;
    pipelineCreateInfo.colorBlendState.attachments.resize(1);
    pipelineCreateInfo.colorBlendState.attachments[0].colorWriteMask = (GraphicsAPI::ColorComponentBit)15;
    pipelineCreateInfo.colorBlendState.attachments[0].blendEnable = true;
    pipelineCreateInfo.colorBlendState.attachments[0].srcColorBlendFactor = GraphicsAPI::BlendFactor::SRC_ALPHA;
    pipelineCreateInfo.colorBlendState.attachments[0].dstColorBlendFactor = GraphicsAPI::BlendFactor::ONE_MINUS_SRC_ALPHA;
    pipelineCreateInfo.colorBlendState.attachments[0].colorBlendOp = GraphicsAPI::BlendOp::ADD;
    pipelineCreateInfo.colorBlendState.attachments[0].srcAlphaBlendFactor = GraphicsAPI::BlendFactor::ONE;
    pipelineCreateInfo.colorBlendState.attachments[0].dstAlphaBlendFactor = GraphicsAPI::BlendFactor::ZERO;
    pipelineCreateInfo.colorBlendState.attachments[0].alphaBlendOp = GraphicsAPI::BlendOp::ADD;
    pipelineCreateInfo.colorBlendState.blendConstants[0] = 0.0f;
    pipelineCreateInfo.colorBlendState.blendConstants[1] = 0.0f;
    pipelineCreateInfo.colorBlendState.blendConstants[2] = 0.0f;
    pipelineCreateInfo.colorBlendState.blendConstants[3] = 0.0f;

    pipelineCreateInfo.colorFormats = {OpenXRDisplayMgr::m_ColorSwapchainInfos[0].swapchainFormat};
    pipelineCreateInfo.depthFormat = OpenXRDisplayMgr::m_DepthSwapchainInfos[0].swapchainFormat;

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

    void* pipeline = OpenXRCoreMgr::GetGraphicsAPI()->CreatePipeline(pipelineCreateInfo);
    m_renderPipelines[materialKey] = pipeline;
    return pipeline;
}

void* SceneRenderer::GetDefaultPipeline()
{
    if (!m_scene) return nullptr;

    const auto& objects = m_scene->GetObjects();
    if (objects.empty()) return nullptr;

    void* pipeline = GetOrCreatePipeline(objects[0].material);
    return pipeline;
}

void SceneRenderer::DestroyResources()
{
    for (auto& pair : m_renderPipelines)
    {
        if (pair.second)
        {
            OpenXRCoreMgr::GetGraphicsAPI()->DestroyPipeline(pair.second);
        }
    }
    m_renderPipelines.clear();

    if (m_uniformBuffer_ObjectData)
    {
        OpenXRCoreMgr::GetGraphicsAPI()->DestroyBuffer(m_uniformBuffer_ObjectData);
        m_uniformBuffer_ObjectData = nullptr;
    }
}
