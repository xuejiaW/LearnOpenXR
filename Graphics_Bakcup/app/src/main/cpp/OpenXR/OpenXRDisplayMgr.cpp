#include "OpenXRDisplayMgr.h"

#include "DebugOutput.h"
#include "GraphicsAPI.h"
#include "OpenXRCoreMgr.h"
#include "OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "OpenXRHelper.h"
#include "OpenXRSessionMgr.h"


std::vector<SwapchainInfo> OpenXRDisplayMgr::m_ColorSwapchainInfos = {};
std::vector<SwapchainInfo> OpenXRDisplayMgr::m_DepthSwapchainInfos = {};
std::vector<XrViewConfigurationType> OpenXRDisplayMgr::m_ExpectedViewConfiguration = {XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
                                                                                      XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO};
std::vector<XrViewConfigurationType> OpenXRDisplayMgr::m_AvailableViewConfigurations;
XrViewConfigurationType OpenXRDisplayMgr::m_ActiveViewConfiguration = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
std::vector<XrViewConfigurationView> OpenXRDisplayMgr::m_ActiveViewConfigurationViews;

XrEnvironmentBlendMode OpenXRDisplayMgr::m_ActiveEnvironmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;
std::vector<XrEnvironmentBlendMode> OpenXRDisplayMgr::m_ExpectedEnvironmentBlendModes = {XR_ENVIRONMENT_BLEND_MODE_OPAQUE};
std::vector<XrEnvironmentBlendMode> OpenXRDisplayMgr::m_AvailableEnvironmentBlendModes = {};

std::vector<XrView> OpenXRDisplayMgr::views = {};
RenderLayerInfo OpenXRDisplayMgr::renderLayerInfo{};
void OpenXRDisplayMgr::GetViewConfigurationViews()
{
    uint32_t viewConfigurationCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurations(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, 0, &viewConfigurationCount, nullptr),
                 "Failed to enumerate OpenXR view configurations");

    m_AvailableViewConfigurations.resize(viewConfigurationCount);
    OPENXR_CHECK(xrEnumerateViewConfigurations(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, viewConfigurationCount, &viewConfigurationCount,
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
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, m_ActiveViewConfiguration, 0,
                                                   &viewConfigurationViewsCount, nullptr),
                 "Failed to enumerate OpenXR view configuration iews");

    XrViewConfigurationView templateConfigurationView{};
    templateConfigurationView.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
    m_ActiveViewConfigurationViews.resize(viewConfigurationViewsCount, templateConfigurationView);
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, m_ActiveViewConfiguration,
                                                   viewConfigurationViewsCount, &viewConfigurationViewsCount, m_ActiveViewConfigurationViews.data()),
                 "Failed to enumerate OpenXR view configuration views");

    XR_TUT_LOG("OpenXR view configuration type: " << m_ActiveViewConfiguration);
    XR_TUT_LOG("OpenXR view configuration views count: " << m_ActiveViewConfigurationViews.size());
}

void OpenXRDisplayMgr::GetEnvironmentBlendModes()
{
    uint32_t environmentBlendModeCount = 0;
    OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, m_ActiveViewConfiguration, 0,
                                                  &environmentBlendModeCount, nullptr),
                 "Failed to enumerate OpenXR environment blend modes");

    m_AvailableEnvironmentBlendModes.resize(environmentBlendModeCount);
    OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, m_ActiveViewConfiguration,
                                                  environmentBlendModeCount, &environmentBlendModeCount, m_AvailableEnvironmentBlendModes.data()),
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
        m_ActiveEnvironmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;  // Fallback to opaque mode
    }
}

void OpenXRDisplayMgr::CreateSwapchains()
{
    uint32_t formatCount = 0;
    OPENXR_CHECK(xrEnumerateSwapchainFormats(OpenXRCoreMgr::xrSession, 0, &formatCount, nullptr), "Failed to enumerate OpenXR swapchain formats");
    std::vector<int64_t> swapchainFormats(formatCount);
    OPENXR_CHECK(xrEnumerateSwapchainFormats(OpenXRCoreMgr::xrSession, formatCount, &formatCount, swapchainFormats.data()),
                 "Failed to enumerate OpenXR swapchain formats");

    int64_t depthFormat = OpenXRCoreMgr::GetGraphicsAPI()->SelectDepthSwapchainFormat(swapchainFormats);
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

        XrSwapchainCreateInfo swapchainCreateInfo{};
        swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
        swapchainCreateInfo.createFlags = 0;
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
        swapchainCreateInfo.format = OpenXRCoreMgr::GetGraphicsAPI()->SelectColorSwapchainFormat(swapchainFormats);
        swapchainCreateInfo.sampleCount = viewConfigView.recommendedSwapchainSampleCount;
        swapchainCreateInfo.width = viewConfigView.recommendedImageRectWidth;
        swapchainCreateInfo.height = viewConfigView.recommendedImageRectHeight;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.arraySize = 1;
        swapchainCreateInfo.mipCount = 1;
        OPENXR_CHECK(xrCreateSwapchain(OpenXRCoreMgr::xrSession, &swapchainCreateInfo, &colorSwapchainInfo.swapchain),
                     "Failed to create OpenXR color swapchain");
        colorSwapchainInfo.swapchainFormat = swapchainCreateInfo.format;

        // Depth swapchain creation
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
        swapchainCreateInfo.format = depthFormat;
        OPENXR_CHECK(xrCreateSwapchain(OpenXRCoreMgr::xrSession, &swapchainCreateInfo, &depthSwapchainInfo.swapchain),
                     "Failed to create OpenXR depth swapchain");

        uint32_t colorSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr),
                     "Failed to get color swapchain images count");
        XrSwapchainImageBaseHeader* colorSwapchainImages =
            OpenXRCoreMgr::GetOpenXRGraphicsAPI()->AllocateSwapchainImageData(colorSwapchainInfo.swapchain, colorSwapchainImageCount);
        OPENXR_CHECK(
            xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, colorSwapchainImageCount, &colorSwapchainImageCount, colorSwapchainImages),
            "Failed to get color swapchain images");

        uint32_t depthSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &depthSwapchainImageCount, nullptr),
                     "Failed to get depth swapchain images count");
        XrSwapchainImageBaseHeader* depthSwapchainImages =
            OpenXRCoreMgr::GetOpenXRGraphicsAPI()->AllocateSwapchainImageData(depthSwapchainInfo.swapchain,depthSwapchainImageCount);
        OPENXR_CHECK(
            xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, depthSwapchainImageCount, &depthSwapchainImageCount, depthSwapchainImages),
            "Failed to get depth swapchain images");

        for (uint32_t j = 0; j < colorSwapchainImageCount; ++j)
        {
            GraphicsAPI::ImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.image = OpenXRCoreMgr::GetOpenXRGraphicsAPI()->GetSwapchainImage(colorSwapchainInfo.swapchain, j);
            imageViewCreateInfo.type = GraphicsAPI::ImageViewCreateInfo::Type::RTV;  // Render Target View
            imageViewCreateInfo.view = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCreateInfo.format = colorSwapchainInfo.swapchainFormat;
            imageViewCreateInfo.aspect = GraphicsAPI::ImageViewCreateInfo::Aspect::COLOR_BIT;
            imageViewCreateInfo.baseMipLevel = 0;
            imageViewCreateInfo.levelCount = 1;
            imageViewCreateInfo.baseArrayLayer = 0;
            imageViewCreateInfo.layerCount = 1;
            colorSwapchainInfo.imageViews.push_back(OpenXRCoreMgr::GetGraphicsAPI()->CreateImageView(imageViewCreateInfo));
        }

        for (uint32_t j = 0; j < depthSwapchainImageCount; ++j)
        {
            GraphicsAPI::ImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.image = OpenXRCoreMgr::GetOpenXRGraphicsAPI()->GetSwapchainImage(depthSwapchainInfo.swapchain, j);
            imageViewCreateInfo.type = GraphicsAPI::ImageViewCreateInfo::Type::DSV;  // Depth Stencil View
            imageViewCreateInfo.view = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCreateInfo.format = depthSwapchainInfo.swapchainFormat;
            imageViewCreateInfo.aspect = GraphicsAPI::ImageViewCreateInfo::Aspect::DEPTH_BIT;
            imageViewCreateInfo.baseMipLevel = 0;
            imageViewCreateInfo.levelCount = 1;
            imageViewCreateInfo.baseArrayLayer = 0;
            imageViewCreateInfo.layerCount = 1;
            depthSwapchainInfo.imageViews.push_back(OpenXRCoreMgr::GetGraphicsAPI()->CreateImageView(imageViewCreateInfo));
        }
    }
}

void OpenXRDisplayMgr::DestroySwapchains()
{
    for (size_t i = 0; i != m_ActiveViewConfigurationViews.size(); ++i)
    {
        SwapchainInfo& colorSwapchainInfo = m_ColorSwapchainInfos[i];
        SwapchainInfo& depthSwapchainInfo = m_DepthSwapchainInfos[i];

        for (auto& imageView : colorSwapchainInfo.imageViews)
        {
            OpenXRCoreMgr::GetGraphicsAPI()->DestroyImageView(imageView);
        }
        OpenXRCoreMgr::GetOpenXRGraphicsAPI()->FreeSwapchainImageData(colorSwapchainInfo.swapchain);

        for (auto& imageView : depthSwapchainInfo.imageViews)
        {
            OpenXRCoreMgr::GetGraphicsAPI()->DestroyImageView(imageView);
        }
        OpenXRCoreMgr::GetOpenXRGraphicsAPI()->FreeSwapchainImageData(depthSwapchainInfo.swapchain);

        OPENXR_CHECK(xrDestroySwapchain(colorSwapchainInfo.swapchain), "Failed to destroy OpenXR color swapchain");
        OPENXR_CHECK(xrDestroySwapchain(depthSwapchainInfo.swapchain), "Failed to destroy OpenXR depth swapchain");
    }
}

int OpenXRDisplayMgr::RefreshViewsData()
{
    XrView viewTemplate{};
    viewTemplate.type = XR_TYPE_VIEW;

    views.resize(m_ActiveViewConfigurationViews.size(), viewTemplate);

    XrViewState viewState{};
    viewState.type = XR_TYPE_VIEW_STATE;
    XrViewLocateInfo viewLocateInfo{};
    viewLocateInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
    viewLocateInfo.viewConfigurationType = m_ActiveViewConfiguration;
    viewLocateInfo.displayTime = OpenXRSessionMgr::frameState.predictedDisplayTime;
    viewLocateInfo.space = OpenXRCoreMgr::m_ActiveSpaces;
    uint32_t viewCount = 0;

    OPENXR_CHECK(xrLocateViews(OpenXRCoreMgr::xrSession, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount, views.data()),
                 "Failed to locate views");

    return static_cast<uint32_t>(viewCount);
}

void OpenXRDisplayMgr::AcquireSwapChainImages(const int viewIndex, void*& colorImage, void*& depthImage)
{

    uint32_t colorImageIndex = 0, depthImageIndex = 0;
    SwapchainInfo& colorSwapchainInfo = m_ColorSwapchainInfos[viewIndex];
    OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, nullptr, &colorImageIndex), "Failed to acquire color swapchain image");

    SwapchainInfo& depthSwapchainInfo = m_DepthSwapchainInfos[viewIndex];
    OPENXR_CHECK(xrAcquireSwapchainImage(depthSwapchainInfo.swapchain, nullptr, &depthImageIndex), "Failed to acquire depth swapchain image");

    XrSwapchainImageWaitInfo waitInfo{};
    waitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
    waitInfo.timeout = XR_INFINITE_DURATION;
    OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for color swapchain image");
    OPENXR_CHECK(xrWaitSwapchainImage(depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for depth swapchain image");

    colorImage = m_ColorSwapchainInfos[viewIndex].imageViews[colorImageIndex];
    depthImage = m_DepthSwapchainInfos[viewIndex].imageViews[depthImageIndex];
}

void OpenXRDisplayMgr::ReleaseSwapChainImages(int viewIndex)
{
    XrSwapchainImageReleaseInfo releaseInfo{};
    releaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
    OPENXR_CHECK(xrReleaseSwapchainImage(m_ColorSwapchainInfos[viewIndex].swapchain, &releaseInfo),
                 "Failed to release Image back to the Color Swapchain");
    OPENXR_CHECK(xrReleaseSwapchainImage(m_DepthSwapchainInfos[viewIndex].swapchain, &releaseInfo),
                 "Failed to release Image back to the Depth Swapchain");
}

void OpenXRDisplayMgr::RefreshProjectionLayerViews(int viewIndex)
{
    const uint32_t& width = m_ActiveViewConfigurationViews[viewIndex].recommendedImageRectWidth;
    const uint32_t& height = m_ActiveViewConfigurationViews[viewIndex].recommendedImageRectHeight;

    renderLayerInfo.layerProjectionViews[viewIndex].pose = views[viewIndex].pose;
    renderLayerInfo.layerProjectionViews[viewIndex].fov = views[viewIndex].fov;
    renderLayerInfo.layerProjectionViews[viewIndex].subImage.swapchain = m_ColorSwapchainInfos[viewIndex].swapchain;
    renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageRect.offset.x = 0;
    renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageRect.offset.y = 0;
    renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageRect.extent.width = static_cast<int32_t>(width);
    renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageRect.extent.height = static_cast<int32_t>(height);
    renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageArrayIndex = 0;
}

void OpenXRDisplayMgr::GenerateRenderLayerInfo()
{
    XrCompositionLayerProjectionView layerProjectionViewTemplate = {};
    layerProjectionViewTemplate.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
    renderLayerInfo.layerProjectionViews.resize(views.size(), layerProjectionViewTemplate);

    renderLayerInfo.projectionLayer.layerFlags =
        XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
    renderLayerInfo.projectionLayer.space = OpenXRCoreMgr::m_ActiveSpaces;
    renderLayerInfo.projectionLayer.viewCount = static_cast<uint32_t>(renderLayerInfo.layerProjectionViews.size());
    renderLayerInfo.projectionLayer.views = renderLayerInfo.layerProjectionViews.data();
}
