#pragma once
#include <openxr/openxr.h>

#include <vector>

#include "OpenXRDisplay/SwapchainInfo.h"

class OpenXRFrameDataMgr
{
public:
    static void GetViewConfigurationViews();

    static XrViewConfigurationType activeViewConfigurationType;
    static std::vector<XrViewConfigurationView> activeViewConfigurationViews;

    static void CreateSwapchains();
    static void DestroySwapchains();

    static std::vector<SwapchainInfo> colorSwapchainInfos;
    static std::vector<SwapchainInfo> depthSwapchainInfos;

private:
    static std::vector<XrViewConfigurationType> m_ExpectedViewConfigurationTypes;
    static std::vector<XrViewConfigurationType> m_AvailableViewConfigurationTypes;
};