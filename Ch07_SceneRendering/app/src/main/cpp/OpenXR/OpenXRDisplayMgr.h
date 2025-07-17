#pragma once
#include <openxr/openxr.h>

#include <vector>

#include "OpenXRDisplay/SwapchainInfo.h"

class OpenXRDisplayMgr
{
public:
    static void GetViewConfigurationViewsInfo();

    static XrViewConfigurationType activeViewConfigurationType;
    static std::vector<XrViewConfigurationView> activeViewConfigurationViews;

    static void CreateSwapchains();
    static void DestroySwapchains();

    static void AcquireAndWaitSwapChainImages(int viewIndex, void*& colorImage, void*& depthImage);
    static void ReleaseSwapChainImages(int viewIndex);

    static std::vector<SwapchainInfo> colorSwapchainInfos;
    static std::vector<SwapchainInfo> depthSwapchainInfos;

    static size_t GetViewsCount();

private:
    static std::vector<XrViewConfigurationType> m_ExpectedViewConfigurationTypes;
    static std::vector<XrViewConfigurationType> m_AvailableViewConfigurationTypes;
};