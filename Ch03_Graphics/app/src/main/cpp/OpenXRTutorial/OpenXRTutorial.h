#pragma once

#include <GraphicsAPI.h>

#if defined(__ANDROID__)
#include <android_native_app_glue.h>
#endif

class OpenXRTutorial
{
public:
    OpenXRTutorial(GraphicsAPI_Type apiType);
    ~OpenXRTutorial();

    void Run();

#if defined(__ANDROID__)
  public:
    static android_app *androidApp;

    struct AndroidAppState
    {
        ANativeWindow *nativeWindow = nullptr;
        bool resumed = false;
    };
    static AndroidAppState androidAppState;

    static void AndroidAppHandleCmd(struct android_app *app, int32_t cmd);
#endif

private:
    void PollSystemEvents();
    void PollEvent();

    void CreateInstance();
    void DestroyInstance();

    void CreateDebugMessenger();
    void DestroyDebugMessenger();

    void GetSystemID();

    void CreateSession();
    void DestroySession();

    void ActiveAvailableApiLayers();
    void ActiveAvailableExtensions();

    void GetInstanceProperties();

    void GetViewConfigurationViews();
    void CreateSwapchains();
    void DestroySwapchains();

    void GetEnvironmentBlendModes();

    std::vector<XrViewConfigurationType> m_ExpectedViewConfiguration = {XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
                                                                        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO};
    std::vector<XrViewConfigurationType> m_AvailableViewConfigurations;
    XrViewConfigurationType m_ActiveViewConfiguration = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
    std::vector<XrViewConfigurationView> m_ActiveViewConfigurationViews;

    struct SwapchainInfo
    {
        XrSwapchain swapchain = XR_NULL_HANDLE;
        int64_t swapchainFormat = 0;
        std::vector<void*> imageViews;
    };

    std::vector<SwapchainInfo> m_ColorSwapchainInfos = {};
    std::vector<SwapchainInfo> m_DepthSwapchainInfos = {};

    XrInstance m_xrInstance = XR_NULL_HANDLE;  // This name can not be modified
    std::vector<std::string> m_RequestApiLayers = {};
    std::vector<const char*> m_ActiveApiLayers = {};

    std::vector<std::string> m_RequestExtensions = {};
    std::vector<const char*> m_ActiveExtensions = {};

    XrDebugUtilsMessengerEXT m_DebugUtilsMessenger = XR_NULL_HANDLE;

    GraphicsAPI_Type m_apiType;
    std::unique_ptr<GraphicsAPI> m_GraphicsAPI = nullptr;

    XrSystemId m_SystemID = XR_NULL_SYSTEM_ID;
    XrSession m_xrSession = XR_NULL_HANDLE;
    XrSessionState m_SessionState = XR_SESSION_STATE_UNKNOWN;

    std::vector<XrEnvironmentBlendMode> m_ExpectedEnvironmentBlendModes = {XR_ENVIRONMENT_BLEND_MODE_OPAQUE};
    std::vector<XrEnvironmentBlendMode> m_AvailableEnvironmentBlendModes = {};
    XrEnvironmentBlendMode m_ActiveEnvironmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;

    bool m_applicationRunning = true;
    bool m_sessionRunning = false;
};