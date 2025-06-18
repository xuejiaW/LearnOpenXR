#pragma once

#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>

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
    struct RenderLayerInfo
    {
        XrTime predictedDisplayTime = 0;
        std::vector<XrCompositionLayerBaseHeader*> layers;
        XrCompositionLayerProjection projectionLayer = {XR_TYPE_COMPOSITION_LAYER_PROJECTION};
        std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
    };

    // Data used to render a cuboid
    struct CuboidConstants
    {
        XrMatrix4x4f viewProj;
        XrMatrix4x4f modelViewProj;
        XrMatrix4x4f model;
        XrVector4f color;
        XrVector4f pad1;
        XrVector4f pad2;
        XrVector4f pad3;
    };

    // Reused for every draw
    CuboidConstants cuboidsConstants;

    XrVector4f normals[6] = {
        {1.00f, 0.00f, 0.00f, 0},
        {-1.00f, 0.00f, 0.00f, 0},
        {0.00f, 1.00f, 0.00f, 0},
        {0.00f, -1.00f, 0.00f, 0},
        {0.00f, 0.00f, 1.00f, 0},
        {0.00f, 0.0f, -1.00f, 0}};

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

    void CreateReferenceSpaces();
    void DestroyReferenceSpace();

    void RenderFrame();
    bool RenderLayer(RenderLayerInfo& renderLayerInfo);

    void CreateResources();
    void RenderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color);
    void DestroyResources();

    float m_viewHeightM = 1.5f;

    // Vertex and index buffers: geometry for our cuboids.
    void* m_vertexBuffer = nullptr;
    void* m_indexBuffer = nullptr;
    // Camera values constant buffer for the shaders.
    void* m_uniformBuffer_Camera = nullptr;
    // The normals are stored in a uniform buffer to simplify our vertex geometry.
    void* m_uniformBuffer_Normals = nullptr;

    // We use only two shaders in this app.
    void *m_vertexShader = nullptr, *m_fragmentShader = nullptr;

    // The pipeline is a graphics-API specific state object.
    void* m_pipeline = nullptr;

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

    XrSpace m_ActiveSpaces = XR_NULL_HANDLE;

    bool m_applicationRunning = true;
    bool m_sessionRunning = false;
};