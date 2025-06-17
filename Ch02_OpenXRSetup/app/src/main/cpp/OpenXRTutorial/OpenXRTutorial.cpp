#include "OpenXRTutorial.h"
#include <DebugOutput.h>
#include <OpenXRDebugUtils.h>

#include <GraphicsAPI_Vulkan.h>

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType) : m_apiType(apiType) {}

OpenXRTutorial::~OpenXRTutorial() = default;

void OpenXRTutorial::Run()
{
    CreateInstance();
    CreateDebugMessenger();
    GetSystemID();
    CreateSession();

    while (m_applicationRunning)
    {
        PollSystemEvents();
        PollEvent();
    }

    DestroySession();
    DestroyDebugMessenger();
    DestroyInstance();
}

void OpenXRTutorial::ActiveAvailableApiLayers()
{
    std::vector<XrApiLayerProperties> availableApiLayersProperties;

    uint32_t propertyCount = 0;
    xrEnumerateApiLayerProperties(0, &propertyCount, nullptr);
    availableApiLayersProperties.resize(propertyCount, {XR_TYPE_API_LAYER_PROPERTIES});
    xrEnumerateApiLayerProperties(propertyCount, &propertyCount, availableApiLayersProperties.data());

    for (auto &requestLayer : m_RequestApiLayers)
    {
        for (const auto &availableLayer : availableApiLayersProperties)
        {
            if (strcmp(requestLayer.c_str(), availableLayer.layerName) != 0)
            {
                continue;
            }
            else
            {
                m_ActiveApiLayers.push_back(availableLayer.layerName);
                XR_TUT_LOG_ERROR("Requested api layer not found: " << requestLayer);
                break;
            }
        }
    }
}

void OpenXRTutorial::ActiveAvailableExtensions()
{
    // We always request the debug utils extension and the graphics API instance extension.
    m_ActiveExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
    m_ActiveExtensions.push_back(GetGraphicsAPIInstanceExtensionString(m_apiType));

    uint32_t extensionCount = 0;
    std::vector<XrExtensionProperties> availableExtensions;
    xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr);
    availableExtensions.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, availableExtensions.data());

    for (const auto &requestExtension : m_RequestExtensions)
    {
        bool found = false;
        for (const auto &availableExtension : availableExtensions)
        {
            if (strcmp(requestExtension.c_str(), availableExtension.extensionName) != 0)
            {
                continue;
            }
            else
            {
                m_ActiveExtensions.push_back(availableExtension.extensionName);
                found = true;
                break;
            }

            if (!found)
            {
                XR_TUT_LOG_ERROR("Requested extension not found: " << requestExtension);
            }
        }
    }
}

void OpenXRTutorial::GetInstanceProperties()
{
    XrInstanceProperties instanceProperties = {XR_TYPE_INSTANCE_PROPERTIES};
    OPENXR_CHECK(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get OpenXR instance properties");
    XR_TUT_LOG("OpenXR Runtime: " << instanceProperties.runtimeName << " - " << XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
                                  << XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
                                  << XR_VERSION_PATCH(instanceProperties.runtimeVersion));
}

void OpenXRTutorial::CreateInstance()
{
    XrApplicationInfo appInfo = {};
    strncpy(appInfo.applicationName, "OpenXRTutorial Ch02_OpenXRSetup", XR_MAX_APPLICATION_NAME_SIZE);
    appInfo.applicationVersion = 1;
    strncpy(appInfo.engineName, "OpenXRTutorial Engine", XR_MAX_ENGINE_NAME_SIZE);
    appInfo.engineVersion = 1;
    appInfo.apiVersion = XR_CURRENT_API_VERSION;

    ActiveAvailableApiLayers();
    ActiveAvailableExtensions();

    XrInstanceCreateInfo instanceCreateInfo = {XR_TYPE_INSTANCE_CREATE_INFO};
    instanceCreateInfo.createFlags = 0;  // There are currently no instance creation flag bits defined. This is reserved for future use.
    instanceCreateInfo.applicationInfo = appInfo;
    instanceCreateInfo.enabledApiLayerCount = static_cast<uint32_t>(m_ActiveApiLayers.size());
    instanceCreateInfo.enabledApiLayerNames = m_ActiveApiLayers.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_ActiveExtensions.size());
    instanceCreateInfo.enabledExtensionNames = m_ActiveExtensions.data();
    OPENXR_CHECK(xrCreateInstance(&instanceCreateInfo, &m_xrInstance), "Failed to create OpenXR instance");
    XR_TUT_LOG("OpenXR instance created successfully");

    GetInstanceProperties();
}

void OpenXRTutorial::DestroyInstance() { OPENXR_CHECK(xrDestroyInstance(m_xrInstance), "Failed to destroy OpenXR instance"); }

void OpenXRTutorial::CreateDebugMessenger() { m_DebugUtilsMessenger = CreateOpenXRDebugUtilsMessenger(m_xrInstance); }

void OpenXRTutorial::DestroyDebugMessenger()
{
    if (m_DebugUtilsMessenger != XR_NULL_HANDLE)
    {
        DestroyOpenXRDebugUtilsMessenger(m_xrInstance, m_DebugUtilsMessenger);
        m_DebugUtilsMessenger = XR_NULL_HANDLE;
    }
}

void OpenXRTutorial::GetSystemID()
{
    XrSystemGetInfo systemGetInfo{XR_TYPE_SYSTEM_GET_INFO};
    systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    OPENXR_CHECK(xrGetSystem(m_xrInstance, &systemGetInfo, &m_SystemID), "Failed to get OpenXR system ID");
    XR_TUT_LOG("OpenXR System ID: " << m_SystemID);

    XrSystemProperties m_systemProperties{XR_TYPE_SYSTEM_PROPERTIES};
    OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, m_SystemID, &m_systemProperties), "Failed to get OpenXR system properties");
}

void OpenXRTutorial::CreateSession()
{
    XrSessionCreateInfo sessionCreateInfo{XR_TYPE_SESSION_CREATE_INFO};
    m_GraphicsAPI = std::make_unique<GraphicsAPI_Vulkan>(m_xrInstance, m_SystemID);
    sessionCreateInfo.next = m_GraphicsAPI->GetGraphicsBinding();
    sessionCreateInfo.createFlags = 0;  // There are currently no session creation flag bits defined. This is reserved for future use.
    sessionCreateInfo.systemId = m_SystemID;

    OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCreateInfo, &m_xrSession), "Failed to create OpenXR session");
}

void OpenXRTutorial::DestroySession()
{
    if (m_xrSession != XR_NULL_HANDLE)
    {
        OPENXR_CHECK(xrDestroySession(m_xrSession), "Failed to destroy OpenXR session");
        m_xrSession = XR_NULL_HANDLE;
    }
}

void OpenXRTutorial::PollEvent()
{
    XrEventDataBuffer eventDataBuffer{XR_TYPE_EVENT_DATA_BUFFER};

    auto XrPollEvent = [&]() -> bool
    {
        eventDataBuffer = {XR_TYPE_EVENT_DATA_BUFFER};
        return xrPollEvent(m_xrInstance, &eventDataBuffer) == XR_SUCCESS;
    };

    while (XrPollEvent())
    {
        switch (eventDataBuffer.type)
        {
            case XR_TYPE_EVENT_DATA_EVENTS_LOST:
            {
                auto *eventsLost = reinterpret_cast<XrEventDataEventsLost *>(&eventDataBuffer);
                XR_TUT_LOG_ERROR("OpenXR events lost: " << eventsLost->lostEventCount);
                break;
            }
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            {
                auto *sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged *>(&eventDataBuffer);
                m_SessionState = sessionStateChanged->state;
                XR_TUT_LOG("OpenXR session state changed: " << m_SessionState);

                if (m_SessionState == XR_SESSION_STATE_READY)
                {
                    XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
                    sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                    OPENXR_CHECK(xrBeginSession(m_xrSession, &sessionBeginInfo), "Failed to begin OpenXR session");
                    XR_TUT_LOG("OpenXR session started successfully");
                    m_sessionRunning = true;
                }
                else if (m_SessionState == XR_SESSION_STATE_STOPPING)
                {
                    OPENXR_CHECK(xrEndSession(m_xrSession), "Failed to end OpenXR session");
                    m_sessionRunning = false;
                }
                else if (m_SessionState == XR_SESSION_STATE_EXITING)
                {
                    m_sessionRunning = false;
                    m_applicationRunning = false;
                }
                else if (m_SessionState == XR_SESSION_STATE_LOSS_PENDING)
                {
                    m_sessionRunning = false;
                    m_applicationRunning = false;
                }

                break;
            }
            default:
            {
                XR_TUT_LOG("OpenXR event data type: " << eventDataBuffer.type);
                break;
            }
        }
    }
}
