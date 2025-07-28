#pragma once
#include <openxr/openxr.h>

#include <vector>

#include "OpenXRDisplay/SwapchainInfo.h"
#include "OpenXRDisplay/SwapchainConfig.h"

class OpenXRDisplayMgr
{
  public:
    static void GetActiveViewConfigurationType();
    static void GetViewConfigurationViewsInfo();
    static size_t GetViewsCount();

    static void CreateSwapchains();
    static void CreateSwapchainImages();
    static void CreateSwapchainImageViews();
    static void DestroySwapchainsRelatedData();

    static void AcquireAndWaitSwapChainImages(int viewIndex, void*& colorImage, void*& depthImage);
    static void ReleaseSwapChainImages(int viewIndex);
    
    // 新增：更高级的渲染视图管理（只管理索引状态）
    static void StartRenderingView(int viewIndex);
    static void StopRenderingView();
    
    // 新增：获取当前正在渲染的视图索引
    static int GetCurrentViewIndex();

    static std::vector<SwapchainInfo> colorSwapchainInfos;
    static std::vector<SwapchainInfo> depthSwapchainInfos;

    static XrViewConfigurationType activeViewConfigurationType;
    static std::vector<XrViewConfigurationView> activeViewConfigurationViews;

  private:
    // 新增：存储当前正在渲染的视图索引
    static int currentViewIndex;
    
    static std::vector<int64_t> GetAvailableSwapchainFormats();
    static void CreateSwapchain(const XrSwapchainCreateInfo& baseCreateInfo, const std::vector<int64_t>& availableSwapchainFormats,
                                const SwapchainConfig& config, SwapchainInfo& swapchainInfo);
    static void CreateSwapchainImages(const SwapchainInfo& swapchainInfo);
    static void CreateSwapchainImageViews(SwapchainInfo& swapchainInfo, bool isDepth);
};