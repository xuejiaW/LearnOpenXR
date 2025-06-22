#pragma once
#include <vector>
#include <openxr/openxr.h>

#include "OpenXRDisplay/SwapchainInfo.h"

class OpenXRDisplayMgr
{
public:
    static void GetViewConfigurationViews();
    static void GetEnvironmentBlendModes();
    static void CreateSwapchains();
    static void DestroySwapchains();

    static void ReleaseSwapChainImage();

    static XrViewConfigurationType m_ActiveViewConfiguration;
    static XrEnvironmentBlendMode m_ActiveEnvironmentBlendMode;
    static std::vector<SwapchainInfo> m_ColorSwapchainInfos;
    static std::vector<SwapchainInfo> m_DepthSwapchainInfos;
    static std::vector<XrViewConfigurationView> m_ActiveViewConfigurationViews;

private:

    static std::vector<XrViewConfigurationType> m_ExpectedViewConfiguration;
    static std::vector<XrViewConfigurationType> m_AvailableViewConfigurations;
    static std::vector<XrEnvironmentBlendMode> m_ExpectedEnvironmentBlendModes;
    static std::vector<XrEnvironmentBlendMode> m_AvailableEnvironmentBlendModes;
};

