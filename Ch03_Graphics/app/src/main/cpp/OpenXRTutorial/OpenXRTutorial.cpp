#include "OpenXRTutorial.h"
#include <DebugOutput.h>
#include <OpenXRDebugUtils.h>

#include <GraphicsAPI_Vulkan.h>
#include <openxr/openxr.h>
#include "../OpenXR//OpenXRCoreMgr.h"
#include "../OpenXR/OpenXRDisplayMgr.h"
#include "../OpenXR/OpenXRSessionMgr.h"

size_t renderCuboidIndex = 0;

GraphicsAPI_Type OpenXRTutorial::m_apiType = UNKNOWN;

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType)
{
    m_apiType = apiType;
}

OpenXRTutorial::~OpenXRTutorial() = default;

void OpenXRTutorial::Run()
{
    OpenXRCoreMgr::CreateInstance();
    OpenXRCoreMgr::CreateDebugMessenger();
    OpenXRCoreMgr::GetSystemID();
    OpenXRCoreMgr::CreateSession();
    OpenXRDisplayMgr::GetViewConfigurationViews();
    OpenXRDisplayMgr::CreateSwapchains();
    OpenXRDisplayMgr::GetEnvironmentBlendModes();
    OpenXRCoreMgr::CreateReferenceSpaces();
    CreateResources();

    while (OpenXRSessionMgr::applicationRunning)
    {
        PollSystemEvents();
        OpenXRSessionMgr::PollEvent();
        if (OpenXRSessionMgr::IsSessionRunning())
        {
            RenderFrame();
        }
    }

    DestroyResources();
    OpenXRCoreMgr::DestroyReferenceSpace();
    OpenXRDisplayMgr::DestroySwapchains();
    OpenXRCoreMgr::DestroySession();
    OpenXRCoreMgr::DestroyDebugMessenger();
    OpenXRCoreMgr::DestroyInstance();
}

void OpenXRTutorial::RenderFrame()
{
    OpenXRSessionMgr::WaitFrame();
    OpenXRSessionMgr::BeginFrame();

    // Variables for rendering and layer composition.
    RenderLayerInfo renderLayerInfo{}; // Create new RenderLayerInfo every frame
    renderLayerInfo.predictedDisplayTime = OpenXRSessionMgr::frameState.predictedDisplayTime;

    // bool sessionActive = OpenXRSessionMgr::IsSessionActive();
    if (OpenXRSessionMgr::ShouldRender())
    {
        // Render the stereo image and associate one of swapchain images with the XrCompositionLayerProjection structure.
        bool rendered = RenderLayer(renderLayerInfo);
        if (rendered)
        {
            renderLayerInfo.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&renderLayerInfo.projectionLayer));
        }
    }

    OpenXRSessionMgr::EndFrame(renderLayerInfo);
}

bool OpenXRTutorial::RenderLayer(RenderLayerInfo& renderLayerInfo)
{
    std::vector<XrView> views(OpenXRDisplayMgr::m_ActiveViewConfigurationViews.size(), {XR_TYPE_VIEW});

    XrViewState viewState{XR_TYPE_VIEW_STATE, nullptr};
    XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO, nullptr};
    viewLocateInfo.viewConfigurationType = OpenXRDisplayMgr::m_ActiveViewConfiguration;
    viewLocateInfo.displayTime = renderLayerInfo.predictedDisplayTime;
    viewLocateInfo.space = OpenXRCoreMgr::m_ActiveSpaces;
    uint32_t viewCount = 0;
    XrResult result = xrLocateViews(OpenXRCoreMgr::xrSession, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount,
                                    views.data());
    if (result != XR_SUCCESS)
    {
        XR_TUT_LOG_ERROR("Failed to locate views: " << result);
        return false;
    }

    renderLayerInfo.layerProjectionViews.resize(viewCount, {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW});
    for (uint32_t i = 0; i < viewCount; ++i)
    {
        uint32_t colorImageIndex = 0;
        SwapchainInfo& colorSwapchainInfo = OpenXRDisplayMgr::m_ColorSwapchainInfos[i];
        OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, nullptr, &colorImageIndex), "Failed to acquire color swapchain image");

        uint32_t depthImageIndex = 0;
        SwapchainInfo& depthSwapchainInfo = OpenXRDisplayMgr::m_DepthSwapchainInfos[i];
        OPENXR_CHECK(xrAcquireSwapchainImage(depthSwapchainInfo.swapchain, nullptr, &depthImageIndex), "Failed to acquire depth swapchain image");

        XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO, nullptr};
        waitInfo.timeout = XR_INFINITE_DURATION;
        OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for color swapchain image");
        OPENXR_CHECK(xrWaitSwapchainImage(depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for depth swapchain image");

        const uint32_t& width = OpenXRDisplayMgr::m_ActiveViewConfigurationViews[i].recommendedImageRectWidth;
        const uint32_t& height = OpenXRDisplayMgr::m_ActiveViewConfigurationViews[i].recommendedImageRectHeight;
        GraphicsAPI::Viewport viewport = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
        GraphicsAPI::Rect2D scissor = {{0, 0}, {width, height}};

        // Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
        // This also associates the swapchain image with this layer projection view.
        XrCompositionLayerProjectionView layerProjectionView = {};
        layerProjectionView.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
        renderLayerInfo.layerProjectionViews[i] = layerProjectionView;
        renderLayerInfo.layerProjectionViews[i].pose = views[i].pose;
        renderLayerInfo.layerProjectionViews[i].fov = views[i].fov;
        renderLayerInfo.layerProjectionViews[i].subImage.swapchain = colorSwapchainInfo.swapchain;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.offset.x = 0;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.offset.y = 0;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.extent.width = static_cast<int32_t>(width);
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.extent.height = static_cast<int32_t>(height);
        renderLayerInfo.layerProjectionViews[i].subImage.imageArrayIndex = 0;  // Useful for multiview rendering

        OpenXRCoreMgr::graphicsAPI->BeginRendering();

        // Clear
        if (OpenXRDisplayMgr::m_ActiveEnvironmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE)
        {
            OpenXRCoreMgr::graphicsAPI->ClearColor(colorSwapchainInfo.imageViews[colorImageIndex], 0.17f, 0.17f, 0.17f, 1.00f);
        }
        OpenXRCoreMgr::graphicsAPI->ClearDepth(depthSwapchainInfo.imageViews[depthImageIndex], 1.0f);

        OpenXRCoreMgr::graphicsAPI->SetRenderAttachments(&colorSwapchainInfo.imageViews[colorImageIndex], 1,
                                                         depthSwapchainInfo.imageViews[depthImageIndex], width,
                                                         height, m_pipeline);
        OpenXRCoreMgr::graphicsAPI->SetViewports(&viewport, 1);
        OpenXRCoreMgr::graphicsAPI->SetScissors(&scissor, 1);

        XrMatrix4x4f proj;
        XrMatrix4x4f_CreateProjectionFov(&proj, m_apiType, views[i].fov, 0.05f, 1000.0f);
        XrMatrix4x4f toView;
        XrVector3f scale1m{1.0f, 1.0f, 1.0f};
        XrMatrix4x4f_CreateTranslationRotationScale(&toView, &views[i].pose.position, &views[i].pose.orientation, &scale1m);
        XrMatrix4x4f view;
        XrMatrix4x4f_InvertRigidBody(&view, &toView);
        XrMatrix4x4f_Multiply(&cuboidsConstants.viewProj, &proj, &view);

        renderCuboidIndex = 0;
        // Draw a floor. Scale it by 2 in the X and Z, and 0.1 in the Y,
        RenderCuboid({{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM, 0.0f}}, {2.0f, 0.1f, 2.0f}, {0.4f, 0.5f, 0.5f});
        // Draw a "table".
        RenderCuboid({{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM + 0.9f, -0.7f}}, {1.0f, 0.2f, 1.0f}, {0.6f, 0.6f, 0.4f});

        OpenXRCoreMgr::graphicsAPI->EndRendering();

        // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
        XrSwapchainImageReleaseInfo releaseInfo{};
        releaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
        OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
        OPENXR_CHECK(xrReleaseSwapchainImage(depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");

    }
    renderLayerInfo.projectionLayer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT |
                                                 XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
    renderLayerInfo.projectionLayer.space = OpenXRCoreMgr::m_ActiveSpaces;
    renderLayerInfo.projectionLayer.viewCount = static_cast<uint32_t>(renderLayerInfo.layerProjectionViews.size());
    renderLayerInfo.projectionLayer.views = renderLayerInfo.layerProjectionViews.data();

    return true;
}

void OpenXRTutorial::CreateResources()
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
            24, 25, 26, 27, 28, 29,  // -Z
            30, 31, 32, 33, 34, 35,  // +Z
        };

    m_vertexBuffer = OpenXRCoreMgr::graphicsAPI->CreateBuffer(
        {GraphicsAPI::BufferCreateInfo::Type::VERTEX, sizeof(float) * 4, sizeof(cubeVertices), &cubeVertices});

    m_indexBuffer = OpenXRCoreMgr::graphicsAPI->CreateBuffer({GraphicsAPI::BufferCreateInfo::Type::INDEX, sizeof(uint32_t), sizeof(cubeIndices),
                                                              &cubeIndices});

    size_t numberOfCuboids = 2;
    m_uniformBuffer_Camera = OpenXRCoreMgr::graphicsAPI->CreateBuffer({GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0,
                                                                       sizeof(CuboidConstants) * numberOfCuboids,
                                                                       nullptr});
    m_uniformBuffer_Normals = OpenXRCoreMgr::graphicsAPI->
        CreateBuffer({GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(normals), &normals});

#if defined(__ANDROID__)
        if (m_apiType == VULKAN) {
            std::vector<char> vertexSource = ReadBinaryFile("shaders/VertexShader.spv", androidApp->activity->assetManager);
            m_vertexShader = OpenXRCoreMgr::graphicsAPI->CreateShader({GraphicsAPI::ShaderCreateInfo::Type::VERTEX, vertexSource.data(), vertexSource.size()});
            std::vector<char> fragmentSource = ReadBinaryFile("shaders/PixelShader.spv", androidApp->activity->assetManager);
            m_fragmentShader = OpenXRCoreMgr::graphicsAPI->CreateShader({GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(), fragmentSource.size()});
        }
#else
    if (m_apiType == VULKAN)
    {
        std::vector<char> vertexSource = ReadBinaryFile("VertexShader.spv");
        m_vertexShader = OpenXRCoreMgr::graphicsAPI->CreateShader({GraphicsAPI::ShaderCreateInfo::Type::VERTEX, vertexSource.data(),
                                                                   vertexSource.size()});

        std::vector<char> fragmentSource = ReadBinaryFile("PixelShader.spv");
        m_fragmentShader = OpenXRCoreMgr::graphicsAPI->CreateShader({GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(),
                                                                     fragmentSource.size()});
    }
#endif

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
                                          {{true, GraphicsAPI::BlendFactor::SRC_ALPHA, GraphicsAPI::BlendFactor::ONE_MINUS_SRC_ALPHA, GraphicsAPI::BlendOp::ADD, GraphicsAPI::BlendFactor::ONE, GraphicsAPI::BlendFactor::ZERO, GraphicsAPI::BlendOp::ADD, (GraphicsAPI::ColorComponentBit)15}},
                                          {0.0f, 0.0f, 0.0f, 0.0f}};
    pipelineCreateInfo.colorFormats = {OpenXRDisplayMgr::m_ColorSwapchainInfos[0].swapchainFormat};
    pipelineCreateInfo.depthFormat = OpenXRDisplayMgr::m_DepthSwapchainInfos[0].swapchainFormat;
    pipelineCreateInfo.layout = {{0, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
                                 {1, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
                                 {2, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT}};
    m_pipeline = OpenXRCoreMgr::graphicsAPI->CreatePipeline(pipelineCreateInfo);
}


void OpenXRTutorial::RenderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color)
{
    XrMatrix4x4f_CreateTranslationRotationScale(&cuboidsConstants.model, &pose.position, &pose.orientation, &scale);

    XrMatrix4x4f_Multiply(&cuboidsConstants.modelViewProj, &cuboidsConstants.viewProj, &cuboidsConstants.model);
    cuboidsConstants.color = {color.x, color.y, color.z, 1.0};
    size_t offsetCameraUB = sizeof(CuboidConstants) * renderCuboidIndex;

    OpenXRCoreMgr::graphicsAPI->SetPipeline(m_pipeline);

    OpenXRCoreMgr::graphicsAPI->SetBufferData(m_uniformBuffer_Camera, offsetCameraUB, sizeof(CuboidConstants), &cuboidsConstants);
    OpenXRCoreMgr::graphicsAPI->SetDescriptor({0, m_uniformBuffer_Camera, GraphicsAPI::DescriptorInfo::Type::BUFFER,
                                               GraphicsAPI::DescriptorInfo::Stage::VERTEX,
                                               false, offsetCameraUB, sizeof(CuboidConstants)});
    OpenXRCoreMgr::graphicsAPI->SetDescriptor({1, m_uniformBuffer_Normals, GraphicsAPI::DescriptorInfo::Type::BUFFER,
                                               GraphicsAPI::DescriptorInfo::Stage::VERTEX,
                                               false, 0, sizeof(normals)});

    OpenXRCoreMgr::graphicsAPI->UpdateDescriptors();

    OpenXRCoreMgr::graphicsAPI->SetVertexBuffers(&m_vertexBuffer, 1);
    OpenXRCoreMgr::graphicsAPI->SetIndexBuffer(m_indexBuffer);
    OpenXRCoreMgr::graphicsAPI->DrawIndexed(36);

    renderCuboidIndex++;
}

void OpenXRTutorial::DestroyResources()
{
    OpenXRCoreMgr::graphicsAPI->DestroyPipeline(m_pipeline);
    OpenXRCoreMgr::graphicsAPI->DestroyShader(m_fragmentShader);
    OpenXRCoreMgr::graphicsAPI->DestroyShader(m_vertexShader);
    OpenXRCoreMgr::graphicsAPI->DestroyBuffer(m_uniformBuffer_Camera);
    OpenXRCoreMgr::graphicsAPI->DestroyBuffer(m_uniformBuffer_Normals);
    OpenXRCoreMgr::graphicsAPI->DestroyBuffer(m_indexBuffer);
    OpenXRCoreMgr::graphicsAPI->DestroyBuffer(m_vertexBuffer);
}