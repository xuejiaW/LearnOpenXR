#include "TableFloorScene.h"
#include "../OpenXR/OpenXRDisplayMgr.h"
#include "../OpenXR/OpenXRCoreMgr.h"
#include "../Application/OpenXRTutorial.h"

static size_t renderCuboidIndex = 0;

TableFloorScene::TableFloorScene(GraphicsAPI_Type apiType)
    : SceneRenderer(apiType), m_cuboidConstants()
{}

TableFloorScene::~TableFloorScene() {}

void TableFloorScene::CreateResources()
{
    constexpr XrVector4f vertexPositions[] = {
        {+0.5f, +0.5f, +0.5f, 1.0f},
        {+0.5f, +0.5f, -0.5f, 1.0f},
        {+0.5f, -0.5f, +0.5f, 1.0f},
        {+0.5f, -0.5f, -0.5f, 1.0f},
        {-0.5f, +0.5f, +0.5f, 1.0f},
        {-0.5f, +0.5f, -0.5f, 1.0f},
        {-0.5f, -0.5f, +0.5f, 1.0f},
        {-0.5f, -0.5f, -0.5f, 1.0f}};

#define CUBE_FACE(V1, V2, V3, V4, V5, V6) vertexPositions[V1], vertexPositions[V2], vertexPositions[V3], vertexPositions[V4], vertexPositions[V5], vertexPositions[V6],

    XrVector4f cubeVertices[] = {
            CUBE_FACE(2, 1, 0, 2, 3, 1)  // -X
            CUBE_FACE(6, 4, 5, 6, 5, 7)  // +X
            CUBE_FACE(0, 1, 5, 0, 5, 4)  // -Y
            CUBE_FACE(2, 6, 7, 2, 7, 3)  // +Y
            CUBE_FACE(0, 4, 6, 0, 6, 2)  // -Z
            CUBE_FACE(1, 3, 7, 1, 7, 5)  // +Z
        };

    uint32_t cubeIndices[36] = {
            0, 1, 2, 3, 4, 5,        // -X
            6, 7, 8, 9, 10, 11,      // +X
            12, 13, 14, 15, 16, 17,  // -Y
            18, 19, 20, 21, 22, 23,  // +Y
            24, 25, 26, 27, 28, 29,  // -Z            30, 31, 32, 33, 34, 35,  // +Z
        };
    GraphicsAPI::BufferCreateInfo vertexBufferInfo;
    vertexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::VERTEX;
    vertexBufferInfo.stride = sizeof(float) * 4;
    vertexBufferInfo.size = sizeof(cubeVertices);
    vertexBufferInfo.data = &cubeVertices;
    m_vertexBuffer = OpenXRCoreMgr::GetGraphicsAPI()->CreateBuffer(vertexBufferInfo);

    GraphicsAPI::BufferCreateInfo indexBufferInfo;
    indexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::INDEX;
    indexBufferInfo.stride = sizeof(uint32_t);
    indexBufferInfo.size = sizeof(cubeIndices);
    indexBufferInfo.data = &cubeIndices;
    m_indexBuffer = OpenXRCoreMgr::GetGraphicsAPI()->CreateBuffer(indexBufferInfo);

    size_t numberOfCuboids = 2;
    GraphicsAPI::BufferCreateInfo cameraBufferInfo;
    cameraBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::UNIFORM;
    cameraBufferInfo.stride = 0;
    cameraBufferInfo.size = sizeof(CuboidConstants) * numberOfCuboids;
    cameraBufferInfo.data = nullptr;
    m_uniformBuffer_Camera = OpenXRCoreMgr::GetGraphicsAPI()->CreateBuffer(cameraBufferInfo);

    GraphicsAPI::BufferCreateInfo normalsBufferInfo;
    normalsBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::UNIFORM;
    normalsBufferInfo.stride = 0;
    normalsBufferInfo.size = sizeof(m_normals);
    normalsBufferInfo.data = &m_normals;
    m_uniformBuffer_Normals = OpenXRCoreMgr::GetGraphicsAPI()->CreateBuffer(normalsBufferInfo);

    if (m_apiType == VULKAN)
    {
        m_vertexShader = CreateShaderFromFile("VertexShader.spv", GraphicsAPI::ShaderCreateInfo::Type::VERTEX);
        m_fragmentShader = CreateShaderFromFile("PixelShader.spv", GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT);
    }
    GraphicsAPI::PipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.shaders = {m_vertexShader, m_fragmentShader};

    // Set vertex input state
    pipelineCreateInfo.vertexInputState.attributes.resize(1);
    pipelineCreateInfo.vertexInputState.attributes[0] = {0, 0, GraphicsAPI::VertexType::VEC4, 0, "TEXCOORD"};
    pipelineCreateInfo.vertexInputState.bindings.resize(1);
    pipelineCreateInfo.vertexInputState.bindings[0] = {0, 0, 4 * sizeof(float)};

    // Set input assembly state
    pipelineCreateInfo.inputAssemblyState.topology = GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST;
    pipelineCreateInfo.inputAssemblyState.primitiveRestartEnable = false;

    // Set rasterisation state
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

    // Set multisample state
    pipelineCreateInfo.multisampleState.rasterisationSamples = 1;
    pipelineCreateInfo.multisampleState.sampleShadingEnable = false;
    pipelineCreateInfo.multisampleState.minSampleShading = 1.0f;
    pipelineCreateInfo.multisampleState.sampleMask = 0xFFFFFFFF;
    pipelineCreateInfo.multisampleState.alphaToCoverageEnable = false;
    pipelineCreateInfo.multisampleState.alphaToOneEnable = false;

    // Set depth stencil state
    pipelineCreateInfo.depthStencilState.depthTestEnable = true;
    pipelineCreateInfo.depthStencilState.depthWriteEnable = true;
    pipelineCreateInfo.depthStencilState.depthCompareOp = GraphicsAPI::CompareOp::LESS_OR_EQUAL;
    pipelineCreateInfo.depthStencilState.depthBoundsTestEnable = false;
    pipelineCreateInfo.depthStencilState.stencilTestEnable = false;
    pipelineCreateInfo.depthStencilState.front = {};
    pipelineCreateInfo.depthStencilState.back = {};
    pipelineCreateInfo.depthStencilState.minDepthBounds = 0.0f;
    pipelineCreateInfo.depthStencilState.maxDepthBounds = 1.0f;

    // Set color blend state
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

    // Set layout descriptors
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
    m_pipeline = OpenXRCoreMgr::GetGraphicsAPI()->CreatePipeline(pipelineCreateInfo);
}

void TableFloorScene::Render(const XrMatrix4x4f& viewProj)
{
    m_cuboidConstants.viewProj = viewProj;
    renderCuboidIndex = 0;

    RenderCuboid({{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM, 0.0f}}, {2.0f, 0.1f, 2.0f}, {0.4f, 0.5f, 0.5f});
    RenderCuboid({{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM + 0.9f, -0.7f}}, {1.0f, 0.2f, 1.0f}, {0.6f, 0.6f, 0.4f});
}

void TableFloorScene::RenderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color)
{
    XrMatrix4x4f_CreateTranslationRotationScale(&m_cuboidConstants.model, &pose.position, &pose.orientation, &scale);
    XrMatrix4x4f_Multiply(&m_cuboidConstants.modelViewProj, &m_cuboidConstants.viewProj, &m_cuboidConstants.model);
    m_cuboidConstants.color = {color.x, color.y, color.z, 1.0};

    size_t offsetCameraUB = sizeof(CuboidConstants) * renderCuboidIndex;

    OpenXRCoreMgr::GetGraphicsAPI()->SetPipeline(m_pipeline);
    OpenXRCoreMgr::GetGraphicsAPI()->SetBufferData(m_uniformBuffer_Camera, offsetCameraUB, sizeof(CuboidConstants), &m_cuboidConstants);

    GraphicsAPI::DescriptorInfo descriptor1;
    descriptor1.bindingIndex = 0;
    descriptor1.resource = m_uniformBuffer_Camera;
    descriptor1.type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    descriptor1.stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    descriptor1.readWrite = false;
    descriptor1.bufferOffset = offsetCameraUB;
    descriptor1.bufferSize = sizeof(CuboidConstants);
    OpenXRCoreMgr::GetGraphicsAPI()->SetDescriptor(descriptor1);

    GraphicsAPI::DescriptorInfo descriptor2;
    descriptor2.bindingIndex = 1;
    descriptor2.resource = m_uniformBuffer_Normals;
    descriptor2.type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    descriptor2.stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    descriptor2.readWrite = false;
    descriptor2.bufferOffset = 0;
    descriptor2.bufferSize = sizeof(m_normals);
    OpenXRCoreMgr::GetGraphicsAPI()->SetDescriptor(descriptor2);

    OpenXRCoreMgr::GetGraphicsAPI()->UpdateDescriptors();

    OpenXRCoreMgr::GetGraphicsAPI()->SetVertexBuffers(&m_vertexBuffer, 1);
    OpenXRCoreMgr::GetGraphicsAPI()->SetIndexBuffer(m_indexBuffer);

    OpenXRCoreMgr::GetGraphicsAPI()->DrawIndexed(36);
    renderCuboidIndex++;
}

void TableFloorScene::DestroyResources()
{
    OpenXRCoreMgr::GetGraphicsAPI()->DestroyPipeline(m_pipeline);
    OpenXRCoreMgr::GetGraphicsAPI()->DestroyShader(m_fragmentShader);
    OpenXRCoreMgr::GetGraphicsAPI()->DestroyShader(m_vertexShader);
    OpenXRCoreMgr::GetGraphicsAPI()->DestroyBuffer(m_uniformBuffer_Camera);
    OpenXRCoreMgr::GetGraphicsAPI()->DestroyBuffer(m_uniformBuffer_Normals);
    OpenXRCoreMgr::GetGraphicsAPI()->DestroyBuffer(m_indexBuffer);
    OpenXRCoreMgr::GetGraphicsAPI()->DestroyBuffer(m_vertexBuffer);

    m_pipeline = nullptr;
    m_fragmentShader = nullptr;
    m_vertexShader = nullptr;
    m_uniformBuffer_Camera = nullptr;
    m_uniformBuffer_Normals = nullptr;
    m_indexBuffer = nullptr;
    m_vertexBuffer = nullptr;
}
