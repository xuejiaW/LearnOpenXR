#include "OpenXRDisplayMgr.h"

#include "DebugOutput.h"
#include "OpenXRCoreMgr.h"
#include "OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "OpenXRHelper.h"


XrViewConfigurationType OpenXRDisplayMgr::activeViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
std::vector<XrViewConfigurationView> OpenXRDisplayMgr::activeViewConfigurationViews{};

std::vector<SwapchainInfo> OpenXRDisplayMgr::colorSwapchainInfos{};
std::vector<SwapchainInfo> OpenXRDisplayMgr::depthSwapchainInfos{};

void OpenXRDisplayMgr::GetActiveViewConfigurationType()
{
    uint32_t viewConfigurationCount = 0;

    std::vector<XrViewConfigurationType> availableViewConfigurationTypes{};

    xrEnumerateViewConfigurations(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, 0, &viewConfigurationCount, nullptr);
    availableViewConfigurationTypes.resize(viewConfigurationCount);
    xrEnumerateViewConfigurations(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, viewConfigurationCount, &viewConfigurationCount,
                                  availableViewConfigurationTypes.data());

    XR_TUT_LOG("Supported OpenXR view configurations count is " << viewConfigurationCount);

    std::vector<XrViewConfigurationType> expectedViewConfigurationTypes{XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
                                                                        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO};
    for (const auto& expectedViewConfigurationType : expectedViewConfigurationTypes)
    {
        if (std::find(availableViewConfigurationTypes.begin(), availableViewConfigurationTypes.end(), expectedViewConfigurationType) !=
            availableViewConfigurationTypes.end())
        {
            activeViewConfigurationType = expectedViewConfigurationType;
            break;
        }
    }
}

void OpenXRDisplayMgr::GetViewConfigurationViewsInfo()
{
    uint32_t viewConfigurationViewsCount = 0;
    xrEnumerateViewConfigurationViews(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, activeViewConfigurationType, 0,
                                      &viewConfigurationViewsCount, nullptr);

    XrViewConfigurationView templateConfigurationView{};
    templateConfigurationView.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
    activeViewConfigurationViews.resize(viewConfigurationViewsCount, templateConfigurationView);

    xrEnumerateViewConfigurationViews(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, activeViewConfigurationType, viewConfigurationViewsCount,
                                      &viewConfigurationViewsCount, activeViewConfigurationViews.data());

    XR_TUT_LOG("OpenXR view configuration type: " << XR_ENUM_STR(activeViewConfigurationType));
    XR_TUT_LOG("OpenXR view configuration views count: " << activeViewConfigurationViews.size());
}

void OpenXRDisplayMgr::CreateSwapchains()
{
    std::vector<int64_t> swapchainFormats = GetAvailableSwapchainFormats();

    int viewsCount = static_cast<int>(GetViewsCount());
    colorSwapchainInfos.resize(viewsCount);
    depthSwapchainInfos.resize(viewsCount);

    for (int viewIndex = 0; viewIndex < viewsCount; viewIndex++)
    {
        const XrViewConfigurationView& viewConfigurationView = activeViewConfigurationViews[viewIndex];
        XrSwapchainCreateInfo swapchainCreateInfo{};
        swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
        swapchainCreateInfo.sampleCount = viewConfigurationView.recommendedSwapchainSampleCount;
        swapchainCreateInfo.width = viewConfigurationView.recommendedImageRectWidth;
        swapchainCreateInfo.height = viewConfigurationView.recommendedImageRectHeight;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.arraySize = 1;
        swapchainCreateInfo.mipCount = 1;

        // Create Color Swapchain
        SwapchainConfig colorConfig{XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT, false,
                                    [](const std::vector<int64_t>& formats)
                                    {
                                        return OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SelectColorSwapchainFormat(formats);
                                    }};
        CreateSwapchain(swapchainCreateInfo, swapchainFormats, colorConfig, colorSwapchainInfos[viewIndex]);
        CreateSwapchainImages(colorSwapchainInfos[viewIndex]);
        CreateSwapchainImageViews(colorSwapchainInfos[viewIndex], colorConfig.isDepth);

        // Create Depth Swapchain
        SwapchainConfig depthConfig{XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT, true,
                                    [](const std::vector<int64_t>& formats)
                                    {
                                        return OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SelectDepthSwapchainFormat(formats);
                                    }};
        CreateSwapchain(swapchainCreateInfo, swapchainFormats, depthConfig, depthSwapchainInfos[viewIndex]);
        CreateSwapchainImages(depthSwapchainInfos[viewIndex]);
        CreateSwapchainImageViews(depthSwapchainInfos[viewIndex], depthConfig.isDepth);
    }
}

std::vector<int64_t> OpenXRDisplayMgr::GetAvailableSwapchainFormats()
{
    uint32_t swapchainFormatCount = 0;
    xrEnumerateSwapchainFormats(OpenXRCoreMgr::xrSession, 0, &swapchainFormatCount, nullptr);

    std::vector<int64_t> swapchainFormats(swapchainFormatCount);
    xrEnumerateSwapchainFormats(OpenXRCoreMgr::xrSession, swapchainFormatCount, &swapchainFormatCount, swapchainFormats.data());

    return swapchainFormats;
}

void OpenXRDisplayMgr::CreateSwapchain(const XrSwapchainCreateInfo& baseCreateInfo, const std::vector<int64_t>& swapchainFormats,
                                       const SwapchainConfig& config, SwapchainInfo& swapchainInfo)
{
    XrSwapchainCreateInfo swapchainCreateInfo = baseCreateInfo;
    swapchainCreateInfo.usageFlags = config.usageFlags;
    swapchainCreateInfo.format = config.formatSelector(swapchainFormats);

    const char* swapchainType = config.isDepth ? "depth" : "color";
    OPENXR_CHECK(xrCreateSwapchain(OpenXRCoreMgr::xrSession, &swapchainCreateInfo, &swapchainInfo.swapchain),
                 ("Failed to create OpenXR " + std::string(swapchainType) + " swapchain").c_str());
    swapchainInfo.swapchainFormat = swapchainCreateInfo.format;
}

void OpenXRDisplayMgr::CreateSwapchainImages(const SwapchainInfo& swapchainInfo)
{
    uint32_t swapchainImageCount = 0;
    xrEnumerateSwapchainImages(swapchainInfo.swapchain, 0, &swapchainImageCount, nullptr);
    XrSwapchainImageBaseHeader* swapChainImages =
        OpenXRCoreMgr::openxrGraphicsAPI->AllocateSwapchainImagesMemory(swapchainInfo.swapchain, swapchainImageCount);
    xrEnumerateSwapchainImages(swapchainInfo.swapchain, swapchainImageCount, &swapchainImageCount, swapChainImages);
}

void OpenXRDisplayMgr::CreateSwapchainImageViews(SwapchainInfo& swapchainInfo, const bool isDepth)
{
    uint32_t swapchainImageCount = 0;
    xrEnumerateSwapchainImages(swapchainInfo.swapchain, 0, &swapchainImageCount, nullptr);
    
    for (uint32_t j = 0; j < swapchainImageCount; ++j)
    {
        GraphicsAPI::ImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.image = OpenXRCoreMgr::openxrGraphicsAPI->GetSwapchainImage(swapchainInfo.swapchain, j);
        imageViewCreateInfo.type = isDepth ? GraphicsAPI::ImageViewCreateInfo::Type::DSV : GraphicsAPI::ImageViewCreateInfo::Type::RTV;
        imageViewCreateInfo.view = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
        imageViewCreateInfo.format = swapchainInfo.swapchainFormat;
        imageViewCreateInfo.aspect =
            isDepth ? GraphicsAPI::ImageViewCreateInfo::Aspect::DEPTH_BIT : GraphicsAPI::ImageViewCreateInfo::Aspect::COLOR_BIT;
        imageViewCreateInfo.baseMipLevel = 0;
        imageViewCreateInfo.levelCount = 1;
        imageViewCreateInfo.baseArrayLayer = 0;
        imageViewCreateInfo.layerCount = 1;
        swapchainInfo.imageViews.push_back(OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateImageView(imageViewCreateInfo));
    }
}

void OpenXRDisplayMgr::DestroySwapchains()
{
    for (size_t i = 0; i != GetViewsCount(); i++)
    {
        // Destroy Color Swapchain
        SwapchainInfo& colorSwapchainInfo = colorSwapchainInfos[i];
        for (auto& imageView : colorSwapchainInfo.imageViews)
        {
            OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyImageView(imageView);
        }
        OpenXRCoreMgr::openxrGraphicsAPI->FreeSwapchainImagesMemory(colorSwapchainInfo.swapchain);

        // Destroy Depth Swapchain
        SwapchainInfo& depthSwapchainInfo = depthSwapchainInfos[i];
        for (auto& imageView : depthSwapchainInfo.imageViews)
        {
            OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyImageView(imageView);
        }
        OpenXRCoreMgr::openxrGraphicsAPI->FreeSwapchainImagesMemory(depthSwapchainInfo.swapchain);
    }
}

size_t OpenXRDisplayMgr::GetViewsCount() { return static_cast<int>(activeViewConfigurationViews.size()); }

void OpenXRDisplayMgr::AcquireAndWaitSwapChainImages(int viewIndex, void*& colorImage, void*& depthImage)
{
    uint32_t colorImageIndex = 0, depthImageIndex = 0;
    SwapchainInfo& colorSwapchainInfo = colorSwapchainInfos[viewIndex];
    OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, nullptr, &colorImageIndex), "Failed to acquire color swapchain image");

    SwapchainInfo& depthSwapchainInfo = depthSwapchainInfos[viewIndex];
    OPENXR_CHECK(xrAcquireSwapchainImage(depthSwapchainInfo.swapchain, nullptr, &depthImageIndex), "Failed to acquire depth swapchain image");

    XrSwapchainImageWaitInfo waitInfo{};
    waitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
    waitInfo.timeout = XR_INFINITE_DURATION;
    OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for color swapchain image");
    OPENXR_CHECK(xrWaitSwapchainImage(depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for depth swapchain image");

    colorImage = colorSwapchainInfo.imageViews[colorImageIndex];
    depthImage = depthSwapchainInfo.imageViews[depthImageIndex];
}

void OpenXRDisplayMgr::ReleaseSwapChainImages(int viewIndex)
{
    XrSwapchainImageReleaseInfo releaseInfo{};
    releaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
    OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfos[viewIndex].swapchain, &releaseInfo),
                 "Failed to release Image back to the Color Swapchain");
    OPENXR_CHECK(xrReleaseSwapchainImage(depthSwapchainInfos[viewIndex].swapchain, &releaseInfo),
                 "Failed to release Image back to the Depth Swapchain");
}
