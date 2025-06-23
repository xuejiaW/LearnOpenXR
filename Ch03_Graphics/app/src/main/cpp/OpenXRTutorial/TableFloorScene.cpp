#include "TableFloorScene.h"
#include <OpenXRDebugUtils.h>
#include "../OpenXR//OpenXRDisplayMgr.h"
#include "OpenXRTutorial.h"

// 跟踪当前要渲染的立方体索引
static size_t renderCuboidIndex = 0;

TableFloorScene::TableFloorScene(GraphicsAPI_Type apiType)
    : SceneRenderer(apiType)
{}

TableFloorScene::~TableFloorScene()
{
    // 资源应该已经在 DestroyResources 中清理了
}

void TableFloorScene::CreateResources()
{
    // 创建立方体顶点数据
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
            24, 25, 26, 27, 28, 29,  // -Z
            30, 31, 32, 33, 34, 35,  // +Z
        };

    // 创建顶点和索引缓冲区
    m_vertexBuffer = OpenXRCoreMgr::graphicsAPI->CreateBuffer(
        {GraphicsAPI::BufferCreateInfo::Type::VERTEX, sizeof(float) * 4, sizeof(cubeVertices), &cubeVertices});

    m_indexBuffer = OpenXRCoreMgr::graphicsAPI->CreateBuffer({GraphicsAPI::BufferCreateInfo::Type::INDEX, sizeof(uint32_t), sizeof(cubeIndices),
                                                              &cubeIndices});

    // 创建 Uniform 缓冲区
    size_t numberOfCuboids = 2; // 地板和桌子
    m_uniformBuffer_Camera = OpenXRCoreMgr::graphicsAPI->CreateBuffer({GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0,
                                                                       sizeof(CuboidConstants) * numberOfCuboids,
                                                                       nullptr});    m_uniformBuffer_Normals = OpenXRCoreMgr::graphicsAPI->
        CreateBuffer({GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(m_normals), &m_normals});

    // 使用基类的辅助方法加载着色器
    if (m_apiType == VULKAN)
    {
        m_vertexShader = CreateShaderFromFile("VertexShader.spv", GraphicsAPI::ShaderCreateInfo::Type::VERTEX);
        m_fragmentShader = CreateShaderFromFile("PixelShader.spv", GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT);
    }

    // 创建管线
    GraphicsAPI::PipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.shaders = {m_vertexShader, m_fragmentShader};
    pipelineCreateInfo.vertexInputState.attributes = {{0, 0, GraphicsAPI::VertexType::VEC4, 0, "TEXCOORD"}};
    pipelineCreateInfo.vertexInputState.bindings = {{0, 0, 4 * sizeof(float)}};
    pipelineCreateInfo.inputAssemblyState = {GraphicsAPI::PrimitiveTopology::TRIANGLE_LIST, false};
    pipelineCreateInfo.rasterisationState = {false, false, GraphicsAPI::PolygonMode::FILL, GraphicsAPI::CullMode::BACK,
                                             GraphicsAPI::FrontFace::COUNTER_CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f};
    pipelineCreateInfo.multisampleState = {1, false, 1.0f, 0xFFFFFFFF, false, false};
    pipelineCreateInfo.depthStencilState = {true, true, GraphicsAPI::CompareOp::LESS_OR_EQUAL, false, false, {}, {}, 0.0f, 1.0f};
    pipelineCreateInfo.colorBlendState = {false, GraphicsAPI::LogicOp::NO_OP,
                                          {{true, GraphicsAPI::BlendFactor::SRC_ALPHA, GraphicsAPI::BlendFactor::ONE_MINUS_SRC_ALPHA,
                                            GraphicsAPI::BlendOp::ADD, GraphicsAPI::BlendFactor::ONE, GraphicsAPI::BlendFactor::ZERO,
                                            GraphicsAPI::BlendOp::ADD, (GraphicsAPI::ColorComponentBit)15}},
                                          {0.0f, 0.0f, 0.0f, 0.0f}};
    pipelineCreateInfo.colorFormats = {OpenXRDisplayMgr::m_ColorSwapchainInfos[0].swapchainFormat};
    pipelineCreateInfo.depthFormat = OpenXRDisplayMgr::m_DepthSwapchainInfos[0].swapchainFormat;
    pipelineCreateInfo.layout = {{0, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
                                 {1, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
                                 {2, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT}};
    m_pipeline = OpenXRCoreMgr::graphicsAPI->CreatePipeline(pipelineCreateInfo);
}

void TableFloorScene::Render(const XrMatrix4x4f& viewProj)
{
    // 保存视图投影矩阵
    m_cuboidConstants.viewProj = viewProj;

    // 重置立方体索引
    renderCuboidIndex = 0;

    // 渲染地板（将其在 X 和 Z 方向缩放 2 倍，在 Y 方向缩放 0.1 倍）
    RenderCuboid({{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM, 0.0f}}, {2.0f, 0.1f, 2.0f}, {0.4f, 0.5f, 0.5f});

    // 渲染桌子
    RenderCuboid({{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM + 0.9f, -0.7f}}, {1.0f, 0.2f, 1.0f}, {0.6f, 0.6f, 0.4f});
}

void TableFloorScene::RenderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color)
{
    // 计算模型矩阵
    XrMatrix4x4f_CreateTranslationRotationScale(&m_cuboidConstants.model, &pose.position, &pose.orientation, &scale);

    // 计算模型视图投影矩阵
    XrMatrix4x4f_Multiply(&m_cuboidConstants.modelViewProj, &m_cuboidConstants.viewProj, &m_cuboidConstants.model);

    // 设置颜色
    m_cuboidConstants.color = {color.x, color.y, color.z, 1.0};

    // 计算缓冲区偏移
    size_t offsetCameraUB = sizeof(CuboidConstants) * renderCuboidIndex;

    // 设置渲染管线和缓冲区
    OpenXRCoreMgr::graphicsAPI->SetPipeline(m_pipeline);
    OpenXRCoreMgr::graphicsAPI->SetBufferData(m_uniformBuffer_Camera, offsetCameraUB, sizeof(CuboidConstants), &m_cuboidConstants);

    // 设置描述符
    OpenXRCoreMgr::graphicsAPI->SetDescriptor({0, m_uniformBuffer_Camera, GraphicsAPI::DescriptorInfo::Type::BUFFER,
                                               GraphicsAPI::DescriptorInfo::Stage::VERTEX,
                                               false, offsetCameraUB, sizeof(CuboidConstants)});
    OpenXRCoreMgr::graphicsAPI->SetDescriptor({1, m_uniformBuffer_Normals, GraphicsAPI::DescriptorInfo::Type::BUFFER,
                                               GraphicsAPI::DescriptorInfo::Stage::VERTEX,
                                               false, 0, sizeof(m_normals)});

    // 更新描述符
    OpenXRCoreMgr::graphicsAPI->UpdateDescriptors();

    // 设置顶点和索引缓冲区
    OpenXRCoreMgr::graphicsAPI->SetVertexBuffers(&m_vertexBuffer, 1);
    OpenXRCoreMgr::graphicsAPI->SetIndexBuffer(m_indexBuffer);

    // 绘制立方体
    OpenXRCoreMgr::graphicsAPI->DrawIndexed(36);

    // 增加立方体索引，为下一个立方体做准备
    renderCuboidIndex++;
}

void TableFloorScene::DestroyResources()
{
    // 清理资源
    OpenXRCoreMgr::graphicsAPI->DestroyPipeline(m_pipeline);
    OpenXRCoreMgr::graphicsAPI->DestroyShader(m_fragmentShader);
    OpenXRCoreMgr::graphicsAPI->DestroyShader(m_vertexShader);
    OpenXRCoreMgr::graphicsAPI->DestroyBuffer(m_uniformBuffer_Camera);
    OpenXRCoreMgr::graphicsAPI->DestroyBuffer(m_uniformBuffer_Normals);
    OpenXRCoreMgr::graphicsAPI->DestroyBuffer(m_indexBuffer);
    OpenXRCoreMgr::graphicsAPI->DestroyBuffer(m_vertexBuffer);

    // 重置指针
    m_pipeline = nullptr;
    m_fragmentShader = nullptr;
    m_vertexShader = nullptr;
    m_uniformBuffer_Camera = nullptr;
    m_uniformBuffer_Normals = nullptr;
    m_indexBuffer = nullptr;
    m_vertexBuffer = nullptr;
}