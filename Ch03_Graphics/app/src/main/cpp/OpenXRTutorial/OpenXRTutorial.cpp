#include "OpenXRTutorial.h"
#include <DebugOutput.h>
#include <OpenXRDebugUtils.h>

#include <GraphicsAPI_Vulkan.h>

size_t renderCuboidIndex = 0;


OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType) : m_apiType(apiType) {}

OpenXRTutorial::~OpenXRTutorial() = default;

void OpenXRTutorial::Run()
{
    CreateInstance();
    CreateDebugMessenger();
    GetSystemID();
    CreateSession();
    GetViewConfigurationViews();
    CreateSwapchains();
    GetEnvironmentBlendModes();
    CreateReferenceSpaces();
    CreateResources();

    while (m_applicationRunning)
    {
        PollSystemEvents();
        PollEvent();
        if (m_sessionRunning)
        {
            RenderFrame();
        }
    }

    DestroyResources();
    DestroyReferenceSpace();
    DestroySwapchains();
    DestroySession();
    DestroyDebugMessenger();
    DestroyInstance();
}

void OpenXRTutorial::ActiveAvailableApiLayers()
{
    std::vector<XrApiLayerProperties> availableApiLayersProperties;

    uint32_t propertyCount = 0;
    xrEnumerateApiLayerProperties(0, &propertyCount, nullptr);
    availableApiLayersProperties.resize(propertyCount, {XR_TYPE_API_LAYER_PROPERTIES});
    xrEnumerateApiLayerProperties(propertyCount, &propertyCount, availableApiLayersProperties.data());

    for (auto& requestLayer : m_RequestApiLayers)
    {
        for (const auto& availableLayer : availableApiLayersProperties)
        {
            if (strcmp(requestLayer.c_str(), availableLayer.layerName) != 0)
            {
                continue;
            }
            else
            {
                m_ActiveApiLayers.push_back(availableLayer.layerName);
                XR_TUT_LOG_ERROR("Requested api layer not found: " << requestLayer);
                break;
            }
        }
    }
}

void OpenXRTutorial::ActiveAvailableExtensions()
{
    // We always request the debug utils extension and the graphics API instance extension.
    m_ActiveExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
    m_ActiveExtensions.push_back(GetGraphicsAPIInstanceExtensionString(m_apiType));

    uint32_t extensionCount = 0;
    std::vector<XrExtensionProperties> availableExtensions;
    xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr);
    availableExtensions.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, availableExtensions.data());

    for (const auto& requestExtension : m_RequestExtensions)
    {
        bool found = false;
        for (const auto& availableExtension : availableExtensions)
        {
            if (strcmp(requestExtension.c_str(), availableExtension.extensionName) != 0)
            {
                continue;
            }
            else
            {
                m_ActiveExtensions.push_back(availableExtension.extensionName);
                found = true;
                break;
            }

            if (!found)
            {
                XR_TUT_LOG_ERROR("Requested extension not found: " << requestExtension);
            }
        }
    }
}

void OpenXRTutorial::GetInstanceProperties()
{
    XrInstanceProperties instanceProperties = {XR_TYPE_INSTANCE_PROPERTIES};
    OPENXR_CHECK(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get OpenXR instance properties");
    XR_TUT_LOG("OpenXR Runtime: " << instanceProperties.runtimeName << " - " << XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
        << XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
        << XR_VERSION_PATCH(instanceProperties.runtimeVersion));
}

void OpenXRTutorial::CreateInstance()
{
    XrApplicationInfo appInfo = {};
    strncpy(appInfo.applicationName, "OpenXRTutorial Ch03_Graphics", XR_MAX_APPLICATION_NAME_SIZE);
    appInfo.applicationVersion = 1;
    strncpy(appInfo.engineName, "OpenXRTutorial Engine", XR_MAX_ENGINE_NAME_SIZE);
    appInfo.engineVersion = 1;
    appInfo.apiVersion = XR_CURRENT_API_VERSION;

    ActiveAvailableApiLayers();
    ActiveAvailableExtensions();

    XrInstanceCreateInfo instanceCreateInfo = {XR_TYPE_INSTANCE_CREATE_INFO};
    instanceCreateInfo.createFlags = 0;  // There are currently no instance creation flag bits defined. This is reserved for future use.
    instanceCreateInfo.applicationInfo = appInfo;
    instanceCreateInfo.enabledApiLayerCount = static_cast<uint32_t>(m_ActiveApiLayers.size());
    instanceCreateInfo.enabledApiLayerNames = m_ActiveApiLayers.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_ActiveExtensions.size());
    instanceCreateInfo.enabledExtensionNames = m_ActiveExtensions.data();
    OPENXR_CHECK(xrCreateInstance(&instanceCreateInfo, &m_xrInstance), "Failed to create OpenXR instance");
    XR_TUT_LOG("OpenXR instance created successfully");

    GetInstanceProperties();
}

void OpenXRTutorial::DestroyInstance() { OPENXR_CHECK(xrDestroyInstance(m_xrInstance), "Failed to destroy OpenXR instance"); }

void OpenXRTutorial::CreateDebugMessenger() { m_DebugUtilsMessenger = CreateOpenXRDebugUtilsMessenger(m_xrInstance); }

void OpenXRTutorial::DestroyDebugMessenger()
{
    if (m_DebugUtilsMessenger != XR_NULL_HANDLE)
    {
        DestroyOpenXRDebugUtilsMessenger(m_xrInstance, m_DebugUtilsMessenger);
        m_DebugUtilsMessenger = XR_NULL_HANDLE;
    }
}

void OpenXRTutorial::GetSystemID()
{
    XrSystemGetInfo systemGetInfo{XR_TYPE_SYSTEM_GET_INFO};
    systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    OPENXR_CHECK(xrGetSystem(m_xrInstance, &systemGetInfo, &m_SystemID), "Failed to get OpenXR system ID");
    XR_TUT_LOG("OpenXR System ID: " << m_SystemID);

    XrSystemProperties m_systemProperties{XR_TYPE_SYSTEM_PROPERTIES};
    OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, m_SystemID, &m_systemProperties), "Failed to get OpenXR system properties");
}

void OpenXRTutorial::CreateSession()
{
    XrSessionCreateInfo sessionCreateInfo{XR_TYPE_SESSION_CREATE_INFO};
    m_GraphicsAPI = std::make_unique<GraphicsAPI_Vulkan>(m_xrInstance, m_SystemID);
    sessionCreateInfo.next = m_GraphicsAPI->GetGraphicsBinding();
    sessionCreateInfo.createFlags = 0;  // There are currently no session creation flag bits defined. This is reserved for future use.
    sessionCreateInfo.systemId = m_SystemID;

    OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCreateInfo, &m_xrSession), "Failed to create OpenXR session");
}

void OpenXRTutorial::DestroySession()
{
    if (m_xrSession != XR_NULL_HANDLE)
    {
        OPENXR_CHECK(xrDestroySession(m_xrSession), "Failed to destroy OpenXR session");
        m_xrSession = XR_NULL_HANDLE;
    }
}

void OpenXRTutorial::PollEvent()
{
    XrEventDataBuffer eventDataBuffer{XR_TYPE_EVENT_DATA_BUFFER};

    auto XrPollEvent = [&]() -> bool
    {
        eventDataBuffer = {XR_TYPE_EVENT_DATA_BUFFER};
        return xrPollEvent(m_xrInstance, &eventDataBuffer) == XR_SUCCESS;
    };

    while (XrPollEvent())
    {
        switch (eventDataBuffer.type)
        {
            case XR_TYPE_EVENT_DATA_EVENTS_LOST:
            {
                auto* eventsLost = reinterpret_cast<XrEventDataEventsLost*>(&eventDataBuffer);
                XR_TUT_LOG_ERROR("OpenXR events lost: " << eventsLost->lostEventCount);
                break;
            }
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            {
                auto* sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventDataBuffer);
                m_SessionState = sessionStateChanged->state;
                XR_TUT_LOG("OpenXR session state changed: " << m_SessionState);

                if (m_SessionState == XR_SESSION_STATE_READY)
                {
                    XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
                    sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                    OPENXR_CHECK(xrBeginSession(m_xrSession, &sessionBeginInfo), "Failed to begin OpenXR session");
                    XR_TUT_LOG("OpenXR session started successfully");
                    m_sessionRunning = true;
                }
                else if (m_SessionState == XR_SESSION_STATE_STOPPING)
                {
                    OPENXR_CHECK(xrEndSession(m_xrSession), "Failed to end OpenXR session");
                    m_sessionRunning = false;
                }
                else if (m_SessionState == XR_SESSION_STATE_EXITING)
                {
                    m_sessionRunning = false;
                    m_applicationRunning = false;
                }
                else if (m_SessionState == XR_SESSION_STATE_LOSS_PENDING)
                {
                    m_sessionRunning = false;
                    m_applicationRunning = false;
                }

                break;
            }
            default:
            {
                XR_TUT_LOG("OpenXR event data type: " << eventDataBuffer.type);
                break;
            }
        }
    }
}

void OpenXRTutorial::GetViewConfigurationViews()
{
    uint32_t viewConfigurationCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_SystemID, 0, &viewConfigurationCount, nullptr),
                 "Failed to enumerate OpenXR view configurations");

    m_AvailableViewConfigurations.resize(viewConfigurationCount);
    OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_SystemID, viewConfigurationCount, &viewConfigurationCount,
                     m_AvailableViewConfigurations.data()),
                 "Failed to enumerate OpenXR view configurations");

    for (const XrViewConfigurationType& expectViewConfiguration : m_ExpectedViewConfiguration)
    {
        if (std::find(m_AvailableViewConfigurations.begin(), m_AvailableViewConfigurations.end(), expectViewConfiguration) !=
            m_AvailableViewConfigurations.end())
        {
            m_ActiveViewConfiguration = expectViewConfiguration;
            break;
        }
    }

    uint32_t viewConfigurationViewsCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_SystemID, m_ActiveViewConfiguration, 0, &viewConfigurationViewsCount, nullptr),
                 "Failed to enumerate OpenXR view configuration views");

    m_ActiveViewConfigurationViews.resize(viewConfigurationViewsCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_SystemID, m_ActiveViewConfiguration, viewConfigurationViewsCount,
                     &viewConfigurationViewsCount, m_ActiveViewConfigurationViews.data()),
                 "Failed to enumerate OpenXR view configuration views");

    XR_TUT_LOG("OpenXR view configuration type: " << m_ActiveViewConfiguration);
    XR_TUT_LOG("OpenXR view configuration views count: " << m_ActiveViewConfigurationViews.size());
}

void OpenXRTutorial::CreateSwapchains()
{
    uint32_t formatCount = 0;
    OPENXR_CHECK(xrEnumerateSwapchainFormats(m_xrSession, 0, &formatCount, nullptr), "Failed to enumerate OpenXR swapchain formats");
    std::vector<int64_t> swapchainFormats(formatCount);
    OPENXR_CHECK(xrEnumerateSwapchainFormats(m_xrSession, formatCount, &formatCount, swapchainFormats.data()),
                 "Failed to enumerate OpenXR swapchain formats");

    int64_t depthFormat = m_GraphicsAPI->SelectDepthSwapchainFormat(swapchainFormats);
    if (depthFormat == 0)
    {
        XR_TUT_LOG_ERROR("No suitable depth swapchain format found");
        DEBUG_BREAK;
    }
    else
    {
        XR_TUT_LOG("Selected depth swapchain format: " << depthFormat);
    }

    m_ColorSwapchainInfos.resize(m_ActiveViewConfigurationViews.size());
    m_DepthSwapchainInfos.resize(m_ActiveViewConfigurationViews.size());

    for (size_t i = 0; i < m_ActiveViewConfigurationViews.size(); ++i)
    {
        SwapchainInfo& colorSwapchainInfo = m_ColorSwapchainInfos[i];
        SwapchainInfo& depthSwapchainInfo = m_DepthSwapchainInfos[i];
        const XrViewConfigurationView& viewConfigView = m_ActiveViewConfigurationViews[i];

        XrSwapchainCreateInfo swapchainCreateInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchainCreateInfo.createFlags = 0;
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
        swapchainCreateInfo.format = m_GraphicsAPI->SelectColorSwapchainFormat(swapchainFormats);
        swapchainCreateInfo.sampleCount = viewConfigView.recommendedSwapchainSampleCount;
        swapchainCreateInfo.width = viewConfigView.recommendedImageRectWidth;
        swapchainCreateInfo.height = viewConfigView.recommendedImageRectHeight;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.arraySize = 1;
        swapchainCreateInfo.mipCount = 1;
        OPENXR_CHECK(xrCreateSwapchain(m_xrSession, &swapchainCreateInfo, &colorSwapchainInfo.swapchain), "Failed to create OpenXR color swapchain");
        colorSwapchainInfo.swapchainFormat = swapchainCreateInfo.format;

        // Depth swapchain creation
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
        swapchainCreateInfo.format = depthFormat;
        OPENXR_CHECK(xrCreateSwapchain(m_xrSession, &swapchainCreateInfo, &depthSwapchainInfo.swapchain), "Failed to create OpenXR depth swapchain");

        uint32_t colorSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr),
                     "Failed to get color swapchain images count");
        XrSwapchainImageBaseHeader* colorSwapchainImages =
            m_GraphicsAPI->AllocateSwapchainImageData(colorSwapchainInfo.swapchain, GraphicsAPI::SwapchainType::COLOR, colorSwapchainImageCount);
        OPENXR_CHECK(
            xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, colorSwapchainImageCount, &colorSwapchainImageCount, colorSwapchainImages),
            "Failed to get color swapchain images");

        uint32_t depthSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &depthSwapchainImageCount, nullptr),
                     "Failed to get depth swapchain images count");
        XrSwapchainImageBaseHeader* depthSwapchainImages =
            m_GraphicsAPI->AllocateSwapchainImageData(depthSwapchainInfo.swapchain, GraphicsAPI::SwapchainType::DEPTH, depthSwapchainImageCount);
        OPENXR_CHECK(
            xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, depthSwapchainImageCount, &depthSwapchainImageCount, depthSwapchainImages),
            "Failed to get depth swapchain images");

        for (uint32_t j = 0; j < colorSwapchainImageCount; ++j)
        {
            GraphicsAPI::ImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.image = m_GraphicsAPI->GetSwapchainImage(colorSwapchainInfo.swapchain, j);
            imageViewCreateInfo.type = GraphicsAPI::ImageViewCreateInfo::Type::RTV; // Render Target View
            imageViewCreateInfo.view = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCreateInfo.format = colorSwapchainInfo.swapchainFormat;
            imageViewCreateInfo.aspect = GraphicsAPI::ImageViewCreateInfo::Aspect::COLOR_BIT;
            imageViewCreateInfo.baseMipLevel = 0;
            imageViewCreateInfo.levelCount = 1;
            imageViewCreateInfo.baseArrayLayer = 0;
            imageViewCreateInfo.layerCount = 1;
            colorSwapchainInfo.imageViews.push_back(m_GraphicsAPI->CreateImageView(imageViewCreateInfo));
        }

        for (uint32_t j = 0; j < depthSwapchainImageCount; ++j)
        {
            GraphicsAPI::ImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.image = m_GraphicsAPI->GetSwapchainImage(depthSwapchainInfo.swapchain, j);
            imageViewCreateInfo.type = GraphicsAPI::ImageViewCreateInfo::Type::DSV; // Depth Stencil View
            imageViewCreateInfo.view = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCreateInfo.format = depthSwapchainInfo.swapchainFormat;
            imageViewCreateInfo.aspect = GraphicsAPI::ImageViewCreateInfo::Aspect::DEPTH_BIT;
            imageViewCreateInfo.baseMipLevel = 0;
            imageViewCreateInfo.levelCount = 1;
            imageViewCreateInfo.baseArrayLayer = 0;
            imageViewCreateInfo.layerCount = 1;
            depthSwapchainInfo.imageViews.push_back(m_GraphicsAPI->CreateImageView(imageViewCreateInfo));
        }
    }
}

void OpenXRTutorial::DestroySwapchains()
{
    for (size_t i = 0; i != m_ActiveViewConfigurationViews.size(); ++i)
    {
        SwapchainInfo& colorSwapchainInfo = m_ColorSwapchainInfos[i];
        SwapchainInfo& depthSwapchainInfo = m_DepthSwapchainInfos[i];

        for (auto& imageView : colorSwapchainInfo.imageViews)
        {
            m_GraphicsAPI->DestroyImageView(imageView);
        }
        m_GraphicsAPI->FreeSwapchainImageData(colorSwapchainInfo.swapchain);

        for (auto& imageView : depthSwapchainInfo.imageViews)
        {
            m_GraphicsAPI->DestroyImageView(imageView);
        }
        m_GraphicsAPI->FreeSwapchainImageData(depthSwapchainInfo.swapchain);

        OPENXR_CHECK(xrDestroySwapchain(colorSwapchainInfo.swapchain), "Failed to destroy OpenXR color swapchain");
        OPENXR_CHECK(xrDestroySwapchain(depthSwapchainInfo.swapchain), "Failed to destroy OpenXR depth swapchain");
    }
}

void OpenXRTutorial::GetEnvironmentBlendModes()
{
    uint32_t environmentBlendModeCount = 0;
    OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_SystemID, m_ActiveViewConfiguration, 0, &environmentBlendModeCount, nullptr),
                 "Failed to enumerate OpenXR environment blend modes");

    m_AvailableEnvironmentBlendModes.resize(environmentBlendModeCount);
    OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_SystemID, m_ActiveViewConfiguration, environmentBlendModeCount,
                     &environmentBlendModeCount, m_AvailableEnvironmentBlendModes.data()),
                 "Failed to enumerate OpenXR environment blend modes");

    for (const auto& expectedEnvironmentBlendMode : m_ExpectedEnvironmentBlendModes)
    {
        if (std::find(m_AvailableEnvironmentBlendModes.begin(), m_AvailableEnvironmentBlendModes.end(), expectedEnvironmentBlendMode) !=
            m_AvailableEnvironmentBlendModes.end())
        {
            m_ActiveEnvironmentBlendMode = expectedEnvironmentBlendMode;
            XR_TUT_LOG("OpenXR active environment blend mode: " << m_ActiveEnvironmentBlendMode);
            return;
        }
    }

    if (m_ActiveEnvironmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM)
    {
        XR_TUT_LOG_ERROR("No suitable environment blend mode found");
        m_ActiveEnvironmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE; // Fallback to opaque mode
    }
}

void OpenXRTutorial::CreateReferenceSpaces()
{
    XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO, nullptr};
    referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    referenceSpaceCreateInfo.poseInReferenceSpace = XrPosef{{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
    OPENXR_CHECK(xrCreateReferenceSpace(m_xrSession, &referenceSpaceCreateInfo, &m_ActiveSpaces), "Failed to create OpenXR local reference space");
}

void OpenXRTutorial::RenderFrame()
{
    XrFrameState frameState{XR_TYPE_FRAME_STATE, nullptr};
    XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO, nullptr};
    OPENXR_CHECK(xrWaitFrame(m_xrSession, &frameWaitInfo, &frameState), "Failed to wait for OpenXR frame");

    XrFrameBeginInfo frameBeginInfo{XR_TYPE_FRAME_BEGIN_INFO};
    OPENXR_CHECK(xrBeginFrame(m_xrSession, &frameBeginInfo), "Failed to begin the XR Frame.");

    // Variables for rendering and layer composition.
    RenderLayerInfo renderLayerInfo{}; // Create new RenderLayerInfo every frame
    renderLayerInfo.predictedDisplayTime = frameState.predictedDisplayTime;

    bool sessionActive = (m_SessionState == XR_SESSION_STATE_SYNCHRONIZED || m_SessionState == XR_SESSION_STATE_VISIBLE || m_SessionState ==
                          XR_SESSION_STATE_FOCUSED);
    if (sessionActive && frameState.shouldRender)
    {
        // Render the stereo image and associate one of swapchain images with the XrCompositionLayerProjection structure.
        bool rendered = RenderLayer(renderLayerInfo);
        if (rendered)
        {
            renderLayerInfo.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&renderLayerInfo.projectionLayer));
        }
    }

    XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
    frameEndInfo.displayTime = frameState.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = m_ActiveEnvironmentBlendMode;
    frameEndInfo.layerCount = static_cast<uint32_t>(renderLayerInfo.layers.size());
    frameEndInfo.layers = renderLayerInfo.layers.data();
    OPENXR_CHECK(xrEndFrame(m_xrSession, &frameEndInfo), "Failed to end the XR Frame.");
}

bool OpenXRTutorial::RenderLayer(RenderLayerInfo& renderLayerInfo)
{
    std::vector<XrView> views(m_ActiveViewConfigurationViews.size(), {XR_TYPE_VIEW});

    XrViewState viewState{XR_TYPE_VIEW_STATE, nullptr};
    XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO, nullptr};
    viewLocateInfo.viewConfigurationType = m_ActiveViewConfiguration;
    viewLocateInfo.displayTime = renderLayerInfo.predictedDisplayTime;
    viewLocateInfo.space = m_ActiveSpaces;
    uint32_t viewCount = 0;
    XrResult result = xrLocateViews(m_xrSession, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount, views.data());
    if (result != XR_SUCCESS)
    {
        XR_TUT_LOG_ERROR("Failed to locate views: " << result);
        return false;
    }

    renderLayerInfo.layerProjectionViews.resize(viewCount, {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW});
    for (uint32_t i = 0; i < viewCount; ++i)
    {
        uint32_t colorImageIndex = 0;
        SwapchainInfo& colorSwapchainInfo = m_ColorSwapchainInfos[i];
        OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, nullptr, &colorImageIndex), "Failed to acquire color swapchain image");

        uint32_t depthImageIndex = 0;
        SwapchainInfo& depthSwapchainInfo = m_DepthSwapchainInfos[i];
        OPENXR_CHECK(xrAcquireSwapchainImage(depthSwapchainInfo.swapchain, nullptr, &depthImageIndex), "Failed to acquire depth swapchain image");

        XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO, nullptr};
        waitInfo.timeout = XR_INFINITE_DURATION;
        OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for color swapchain image");
        OPENXR_CHECK(xrWaitSwapchainImage(depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for depth swapchain image");

        const uint32_t& width = m_ActiveViewConfigurationViews[i].recommendedImageRectWidth;
        const uint32_t& height = m_ActiveViewConfigurationViews[i].recommendedImageRectHeight;
        GraphicsAPI::Viewport viewport = {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
        GraphicsAPI::Rect2D scissor = {{0, 0}, {width, height}};
        float nearZ = 0.05f;
        float farZ = 100.0f;

        // Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
        // This also associates the swapchain image with this layer projection view.
        renderLayerInfo.layerProjectionViews[i] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW, nullptr};
        renderLayerInfo.layerProjectionViews[i].pose = views[i].pose;
        renderLayerInfo.layerProjectionViews[i].fov = views[i].fov;
        renderLayerInfo.layerProjectionViews[i].subImage.swapchain = colorSwapchainInfo.swapchain;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.offset.x = 0;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.offset.y = 0;
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.extent.width = static_cast<int32_t>(width);
        renderLayerInfo.layerProjectionViews[i].subImage.imageRect.extent.height = static_cast<int32_t>(height);
        renderLayerInfo.layerProjectionViews[i].subImage.imageArrayIndex = 0;  // Useful for multiview rendering

        m_GraphicsAPI->BeginRendering();

        // Clear
        if (m_ActiveEnvironmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE)
        {
            m_GraphicsAPI->ClearColor(colorSwapchainInfo.imageViews[colorImageIndex], 0.17f, 0.17f, 0.17f, 1.00f);
        }
        m_GraphicsAPI->ClearDepth(depthSwapchainInfo.imageViews[depthImageIndex], 1.0f);

        m_GraphicsAPI->SetRenderAttachments(&colorSwapchainInfo.imageViews[colorImageIndex], 1, depthSwapchainInfo.imageViews[depthImageIndex], width,
                                            height, m_pipeline);
        m_GraphicsAPI->SetViewports(&viewport, 1);
        m_GraphicsAPI->SetScissors(&scissor, 1);

        XrMatrix4x4f proj;
        XrMatrix4x4f_CreateProjectionFov(&proj, m_apiType, views[i].fov, nearZ, farZ);
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

        m_GraphicsAPI->EndRendering();

        // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
        XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
        OPENXR_CHECK(xrReleaseSwapchainImage(depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");

    }
    renderLayerInfo.projectionLayer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT |
                                                 XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
    renderLayerInfo.projectionLayer.space = m_ActiveSpaces;
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

    m_vertexBuffer = m_GraphicsAPI->CreateBuffer(
        {GraphicsAPI::BufferCreateInfo::Type::VERTEX, sizeof(float) * 4, sizeof(cubeVertices), &cubeVertices});

    m_indexBuffer = m_GraphicsAPI->CreateBuffer({GraphicsAPI::BufferCreateInfo::Type::INDEX, sizeof(uint32_t), sizeof(cubeIndices), &cubeIndices});

    size_t numberOfCuboids = 2;
    m_uniformBuffer_Camera = m_GraphicsAPI->CreateBuffer({GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(CuboidConstants) * numberOfCuboids,
                                                          nullptr});
    m_uniformBuffer_Normals = m_GraphicsAPI->CreateBuffer({GraphicsAPI::BufferCreateInfo::Type::UNIFORM, 0, sizeof(normals), &normals});

#if defined(__ANDROID__)
        if (m_apiType == VULKAN) {
            std::vector<char> vertexSource = ReadBinaryFile("shaders/VertexShader.spv", androidApp->activity->assetManager);
            m_vertexShader = m_GraphicsAPI->CreateShader({GraphicsAPI::ShaderCreateInfo::Type::VERTEX, vertexSource.data(), vertexSource.size()});
            std::vector<char> fragmentSource = ReadBinaryFile("shaders/PixelShader.spv", androidApp->activity->assetManager);
            m_fragmentShader = m_GraphicsAPI->CreateShader({GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(), fragmentSource.size()});
        }
#else
    if (m_apiType == VULKAN)
    {
        std::vector<char> vertexSource = ReadBinaryFile("VertexShader.spv");
        m_vertexShader = m_GraphicsAPI->CreateShader({GraphicsAPI::ShaderCreateInfo::Type::VERTEX, vertexSource.data(), vertexSource.size()});

        std::vector<char> fragmentSource = ReadBinaryFile("PixelShader.spv");
        m_fragmentShader = m_GraphicsAPI->CreateShader({GraphicsAPI::ShaderCreateInfo::Type::FRAGMENT, fragmentSource.data(), fragmentSource.size()});
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
    pipelineCreateInfo.colorFormats = {m_ColorSwapchainInfos[0].swapchainFormat};
    pipelineCreateInfo.depthFormat = m_DepthSwapchainInfos[0].swapchainFormat;
    pipelineCreateInfo.layout = {{0, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
                                 {1, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX},
                                 {2, nullptr, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::FRAGMENT}};
    m_pipeline = m_GraphicsAPI->CreatePipeline(pipelineCreateInfo);
}


void OpenXRTutorial::RenderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color)
{
    XrMatrix4x4f_CreateTranslationRotationScale(&cuboidsConstants.model, &pose.position, &pose.orientation, &scale);

    XrMatrix4x4f_Multiply(&cuboidsConstants.modelViewProj, &cuboidsConstants.viewProj, &cuboidsConstants.model);
    cuboidsConstants.color = {color.x, color.y, color.z, 1.0};
    size_t offsetCameraUB = sizeof(CuboidConstants) * renderCuboidIndex;

    m_GraphicsAPI->SetPipeline(m_pipeline);

    m_GraphicsAPI->SetBufferData(m_uniformBuffer_Camera, offsetCameraUB, sizeof(CuboidConstants), &cuboidsConstants);
    m_GraphicsAPI->SetDescriptor({0, m_uniformBuffer_Camera, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX,
                                  false, offsetCameraUB, sizeof(CuboidConstants)});
    m_GraphicsAPI->SetDescriptor({1, m_uniformBuffer_Normals, GraphicsAPI::DescriptorInfo::Type::BUFFER, GraphicsAPI::DescriptorInfo::Stage::VERTEX,
                                  false, 0, sizeof(normals)});

    m_GraphicsAPI->UpdateDescriptors();

    m_GraphicsAPI->SetVertexBuffers(&m_vertexBuffer, 1);
    m_GraphicsAPI->SetIndexBuffer(m_indexBuffer);
    m_GraphicsAPI->DrawIndexed(36);

    renderCuboidIndex++;
}

void OpenXRTutorial::DestroyResources()
{
    m_GraphicsAPI->DestroyPipeline(m_pipeline);
    m_GraphicsAPI->DestroyShader(m_fragmentShader);
    m_GraphicsAPI->DestroyShader(m_vertexShader);
    m_GraphicsAPI->DestroyBuffer(m_uniformBuffer_Camera);
    m_GraphicsAPI->DestroyBuffer(m_uniformBuffer_Normals);
    m_GraphicsAPI->DestroyBuffer(m_indexBuffer);
    m_GraphicsAPI->DestroyBuffer(m_vertexBuffer);
}


void OpenXRTutorial::DestroyReferenceSpace()
{
    OPENXR_CHECK(xrDestroySpace(m_ActiveSpaces), "Failed to destroy Space.")
}