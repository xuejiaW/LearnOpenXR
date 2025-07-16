#include "OpenXRDisplayMgr.h"

#include "DebugOutput.h"
#include "OpenXRCoreMgr.h"
#include "OpenXRHelper.h"
#include "OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"

XrViewConfigurationType OpenXRDisplayMgr::activeViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
std::vector<XrViewConfigurationView> OpenXRDisplayMgr::activeViewConfigurationViews{};
std::vector<XrViewConfigurationType> OpenXRDisplayMgr::m_ExpectedViewConfigurationTypes{XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
                                                                                        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO};
std::vector<XrViewConfigurationType> OpenXRDisplayMgr::m_AvailableViewConfigurationTypes{};

std::vector<SwapchainInfo> OpenXRDisplayMgr::colorSwapchainInfos{};
std::vector<SwapchainInfo> OpenXRDisplayMgr::depthSwapchainInfos{};


void OpenXRDisplayMgr::GetViewConfigurationViewsInfo()
{
    uint32_t viewConfigurationCount = 0;
    xrEnumerateViewConfigurations(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, 0, &viewConfigurationCount, nullptr);
    m_AvailableViewConfigurationTypes.resize(viewConfigurationCount);
    xrEnumerateViewConfigurations(OpenXRCoreMgr::m_xrInstance, OpenXRCoreMgr::systemID, viewConfigurationCount, &viewConfigurationCount,
                                  m_AvailableViewConfigurationTypes.data());

    for (const auto& expectedViewConfigurationType : m_ExpectedViewConfigurationTypes)
    {
        if (std::find(m_AvailableViewConfigurationTypes.begin(), m_AvailableViewConfigurationTypes.end(), expectedViewConfigurationType) !=
            m_AvailableViewConfigurationTypes.end())
        {
            activeViewConfigurationType = expectedViewConfigurationType;
            break;
        }
    }

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
    uint32_t swapchainFormatCount = 0;
    xrEnumerateSwapchainFormats(OpenXRCoreMgr::xrSession, 0, &swapchainFormatCount, nullptr);

    std::vector<int64_t> swapchainFormats(swapchainFormatCount);
    xrEnumerateSwapchainFormats(OpenXRCoreMgr::xrSession, swapchainFormatCount, &swapchainFormatCount, swapchainFormats.data());

    int viewsCount = GetViewsCount();
    colorSwapchainInfos.resize(viewsCount);
    depthSwapchainInfos.resize(viewsCount);

    for (int i = 0; i < viewsCount; i++)
    {
        const XrViewConfigurationView& viewConfigurationView = activeViewConfigurationViews[i];

        XrSwapchainCreateInfo swapchainCreateInfo{};
        swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
        swapchainCreateInfo.sampleCount = viewConfigurationView.recommendedSwapchainSampleCount;
        swapchainCreateInfo.width = viewConfigurationView.recommendedImageRectWidth;
        swapchainCreateInfo.height = viewConfigurationView.recommendedImageRectHeight;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.arraySize = 1;
        swapchainCreateInfo.mipCount = 1;

        // Create Color Swapchain
        SwapchainInfo& colorSwapchainInfo = colorSwapchainInfos[i];
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
        swapchainCreateInfo.format = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SelectColorSwapchainFormat(swapchainFormats);
        OPENXR_CHECK(xrCreateSwapchain(OpenXRCoreMgr::xrSession, &swapchainCreateInfo, &colorSwapchainInfo.swapchain),
                     "Failed to create OpenXR color swapchain");
        colorSwapchainInfo.swapchainFormat = swapchainCreateInfo.format;

        // Create Depth Swapchain
        SwapchainInfo& depthSwapchainInfo = depthSwapchainInfos[i];
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
        swapchainCreateInfo.format = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SelectDepthSwapchainFormat(swapchainFormats);
        OPENXR_CHECK(xrCreateSwapchain(OpenXRCoreMgr::xrSession, &swapchainCreateInfo, &depthSwapchainInfo.swapchain),
                     "Failed to create OpenXR depth swapchain");
        depthSwapchainInfo.swapchainFormat = swapchainCreateInfo.format;

        // Create Color Swapchain Images
        uint32_t colorSwapchainImageCount = 0;
        xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr);
        XrSwapchainImageBaseHeader* colorSwapChainImages = OpenXRCoreMgr::openxrGraphicsAPI->AllocateSwapchainImagesMemory(
            colorSwapchainInfo.swapchain, colorSwapchainImageCount);
        xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, colorSwapchainImageCount, &colorSwapchainImageCount, colorSwapChainImages);

        for (uint32_t j = 0; j < colorSwapchainImageCount; ++j)
        {
            GraphicsAPI::ImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.image = OpenXRCoreMgr::openxrGraphicsAPI->GetSwapchainImage(colorSwapchainInfo.swapchain, j);
            imageViewCreateInfo.type = GraphicsAPI::ImageViewCreateInfo::Type::RTV;  // Render Target View
            imageViewCreateInfo.view = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCreateInfo.format = colorSwapchainInfo.swapchainFormat;
            imageViewCreateInfo.aspect = GraphicsAPI::ImageViewCreateInfo::Aspect::COLOR_BIT;
            imageViewCreateInfo.baseMipLevel = 0;
            imageViewCreateInfo.levelCount = 1;
            imageViewCreateInfo.baseArrayLayer = 0;
            imageViewCreateInfo.layerCount = 1;
            colorSwapchainInfo.imageViews.push_back(OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateImageView(imageViewCreateInfo));
        }

        // Create Depth Swapchain Images
        uint32_t depthSwapchainImageCount = 0;
        xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &depthSwapchainImageCount, nullptr);
        XrSwapchainImageBaseHeader* depthSwapChainImages = OpenXRCoreMgr::openxrGraphicsAPI->AllocateSwapchainImagesMemory(
            depthSwapchainInfo.swapchain, depthSwapchainImageCount);
        xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, depthSwapchainImageCount, &depthSwapchainImageCount, depthSwapChainImages);

        for (uint32_t j = 0; j < depthSwapchainImageCount; ++j)
        {
            GraphicsAPI::ImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.image = OpenXRCoreMgr::openxrGraphicsAPI->GetSwapchainImage(depthSwapchainInfo.swapchain, j);
            imageViewCreateInfo.type = GraphicsAPI::ImageViewCreateInfo::Type::DSV;  // Depth Stencil View
            imageViewCreateInfo.view = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
            imageViewCreateInfo.format = depthSwapchainInfo.swapchainFormat;
            imageViewCreateInfo.aspect = GraphicsAPI::ImageViewCreateInfo::Aspect::DEPTH_BIT;
            imageViewCreateInfo.baseMipLevel = 0;
            imageViewCreateInfo.levelCount = 1;
            imageViewCreateInfo.baseArrayLayer = 0;
            imageViewCreateInfo.layerCount = 1;
            depthSwapchainInfo.imageViews.push_back(OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateImageView(imageViewCreateInfo));
        }
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

size_t OpenXRDisplayMgr::GetViewsCount()
{
    return static_cast<int>(activeViewConfigurationViews.size());
}

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