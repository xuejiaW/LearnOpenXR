#include "OpenXRFrameDataMgr.h"

#include "DebugOutput.h"
#include "OpenXRCoreMgr.h"
#include "OpenXRHelper.h"
#include "OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"

XrViewConfigurationType OpenXRFrameDataMgr::activeViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
std::vector<XrViewConfigurationView> OpenXRFrameDataMgr::activeViewConfigurationViews{};
std::vector<XrViewConfigurationType> OpenXRFrameDataMgr::m_ExpectedViewConfigurationTypes{};
std::vector<XrViewConfigurationType> OpenXRFrameDataMgr::m_AvailableViewConfigurationTypes{};


void OpenXRFrameDataMgr::GetViewConfigurationViews()
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

void OpenXRFrameDataMgr::CreateSwapchains()
{
    uint32_t swapchainFormatCount = 0;
    xrEnumerateSwapchainFormats(OpenXRCoreMgr::xrSession, 0, &swapchainFormatCount, nullptr);

    std::vector<int64_t> swapchainFormats(swapchainFormatCount);
    xrEnumerateSwapchainFormats(OpenXRCoreMgr::xrSession, swapchainFormatCount, &swapchainFormatCount, swapchainFormats.data());

    int viewsCount = static_cast<int>(activeViewConfigurationViews.size());
    colorSwapchainInfos.resize(viewsCount);
    depthSwapchainInfos.resize(viewsCount);

    for (size_t i = 0; i < viewsCount; i++)
    {
        const XrViewConfigurationView& viewConfigurationView = activeViewConfigurationViews[i];

        XrSwapchainCreateInfo swapchainCreateInfo{};
        swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
        
    }

}