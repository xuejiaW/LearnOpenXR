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
    static std::vector<int64_t> GetAvailableSwapchainFormats();
    static void CreateSwapchain(const XrSwapchainCreateInfo& baseCreateInfo, const std::vector<int64_t>& swapchainFormats,
                                const SwapchainConfig& config, SwapchainInfo& swapchainInfo);
    static void CreateSwapchainImages(const SwapchainInfo& swapchainInfo);
    static void CreateSwapchainImageViews(SwapchainInfo& swapchainInfo, bool isDepth);
};