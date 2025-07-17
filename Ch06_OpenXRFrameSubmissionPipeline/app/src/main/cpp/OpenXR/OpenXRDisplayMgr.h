#pragma once
#include <openxr/openxr.h>

#include <functional>
#include <vector>


#include "OpenXRDisplay/SwapchainInfo.h"

class OpenXRDisplayMgr
{
  public:
    // Swapchain configuration structure
    struct SwapchainConfig
    {
        XrSwapchainUsageFlags usageFlags;
        bool isDepth;
        std::function<int64_t(const std::vector<int64_t>&)> formatSelector;
    };

    static void GetActiveViewConfigurationType();
    static void GetViewConfigurationViewsInfo();
    static size_t GetViewsCount();

    static void CreateSwapchains();
    static void DestroySwapchains();

    static void AcquireAndWaitSwapChainImages(int viewIndex, void*& colorImage, void*& depthImage);
    static void ReleaseSwapChainImages(int viewIndex);

    static std::vector<SwapchainInfo> colorSwapchainInfos;
    static std::vector<SwapchainInfo> depthSwapchainInfos;

    static XrViewConfigurationType activeViewConfigurationType;
    static std::vector<XrViewConfigurationView> activeViewConfigurationViews;

  private:
    // Helper methods for swapchain creation
    static std::vector<int64_t> GetSupportedSwapchainFormats();
    static void CreateSwapchainForView(int viewIndex, const XrViewConfigurationView& viewConfigurationView,
                                       const std::vector<int64_t>& swapchainFormats);
    static void CreateSwapchain(int viewIndex, const XrSwapchainCreateInfo& baseCreateInfo, const std::vector<int64_t>& swapchainFormats,
                                const SwapchainConfig& config, SwapchainInfo& swapchainInfo);
    static void CreateSwapchainImages(SwapchainInfo& swapchainInfo, bool isDepth);
};