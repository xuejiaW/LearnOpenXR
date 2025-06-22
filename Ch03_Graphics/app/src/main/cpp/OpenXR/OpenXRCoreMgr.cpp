#include "OpenXRCoreMgr.h"

#include <DebugOutput.h>
#include <OpenXRDebugUtils.h>
#include <openxr/openxr.h>

#include "GraphicsAPI_Vulkan.h"
#include "HelperFunctions.h"
#include "../OpenXRTutorial/OpenXRTutorial.h"

XrInstance OpenXRCoreMgr::m_xrInstance = XR_NULL_HANDLE;
XrSession OpenXRCoreMgr::xrSession = XR_NULL_HANDLE;
XrSystemId OpenXRCoreMgr::systemID = XR_NULL_SYSTEM_ID;
std::unique_ptr<GraphicsAPI> OpenXRCoreMgr::graphicsAPI = nullptr;

std::vector<std::string> OpenXRCoreMgr::m_RequestApiLayers{};
std::vector<const char*> OpenXRCoreMgr::m_ActiveApiLayers{};
std::vector<std::string> OpenXRCoreMgr::m_RequestExtensions{};
std::vector<const char*> OpenXRCoreMgr::m_ActiveExtensions{};
XrDebugUtilsMessengerEXT OpenXRCoreMgr::m_DebugUtilsMessenger{};


void OpenXRCoreMgr::CreateInstance()
{
    XrApplicationInfo appInfo = {};
    strncpy(appInfo.applicationName, "OpenXRTutorial Ch03_Graphics", XR_MAX_APPLICATION_NAME_SIZE);
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

void OpenXRCoreMgr::DestroyInstance() { OPENXR_CHECK(xrDestroyInstance(m_xrInstance), "Failed to destroy OpenXR instance"); }

void OpenXRCoreMgr::CreateDebugMessenger() { m_DebugUtilsMessenger = CreateOpenXRDebugUtilsMessenger(m_xrInstance); }

void OpenXRCoreMgr::DestroyDebugMessenger()
{
    if (m_DebugUtilsMessenger != XR_NULL_HANDLE)
    {
        DestroyOpenXRDebugUtilsMessenger(m_xrInstance, m_DebugUtilsMessenger);
        m_DebugUtilsMessenger = XR_NULL_HANDLE;
    }
}

void OpenXRCoreMgr::GetSystemID()
{
    XrSystemGetInfo systemGetInfo{XR_TYPE_SYSTEM_GET_INFO};
    systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    OPENXR_CHECK(xrGetSystem(m_xrInstance, &systemGetInfo, &systemID), "Failed to get OpenXR system ID");
    XR_TUT_LOG("OpenXR System ID: " << systemID);

    XrSystemProperties m_systemProperties{XR_TYPE_SYSTEM_PROPERTIES};
    OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, systemID, &m_systemProperties), "Failed to get OpenXR system properties");
}

void OpenXRCoreMgr::CreateSession()
{
    XrSessionCreateInfo sessionCreateInfo{XR_TYPE_SESSION_CREATE_INFO};
    graphicsAPI = std::make_unique<GraphicsAPI_Vulkan>(m_xrInstance, systemID);
    sessionCreateInfo.next = graphicsAPI->GetGraphicsBinding();
    sessionCreateInfo.createFlags = 0;  // There are currently no session creation flag bits defined. This is reserved for future use.
    sessionCreateInfo.systemId = systemID;

    OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCreateInfo, &xrSession), "Failed to create OpenXR session");
}

void OpenXRCoreMgr::DestroySession()
{
    if (xrSession != XR_NULL_HANDLE)
    {
        OPENXR_CHECK(xrDestroySession(xrSession), "Failed to destroy OpenXR session");
        xrSession = XR_NULL_HANDLE;
    }
}

void OpenXRCoreMgr::ActiveAvailableApiLayers()
{
    std::vector<XrApiLayerProperties> availableApiLayersProperties;

    uint32_t propertyCount = 0;
    xrEnumerateApiLayerProperties(0, &propertyCount, nullptr);
    availableApiLayersProperties.resize(propertyCount, {XR_TYPE_API_LAYER_PROPERTIES});
    xrEnumerateApiLayerProperties(propertyCount, &propertyCount, availableApiLayersProperties.data());

    for (auto& requestLayer : m_RequestApiLayers)
    {
        for (const auto& availableLayer : availableApiLayersProperties)
        {
            if (strcmp(requestLayer.c_str(), availableLayer.layerName) == 0)
            {
                m_ActiveApiLayers.push_back(availableLayer.layerName);
                XR_TUT_LOG_ERROR("Requested api layer not found: " << requestLayer);
                break;
            }
        }
    }
}

void OpenXRCoreMgr::ActiveAvailableExtensions()
{
    // We always request the debug utils extension and the graphics API instance extension.
    m_ActiveExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
    m_ActiveExtensions.push_back(GetGraphicsAPIInstanceExtensionString(OpenXRTutorial::m_apiType));

    uint32_t extensionCount = 0;
    std::vector<XrExtensionProperties> availableExtensions;
    xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr);
    availableExtensions.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, availableExtensions.data());

    for (const auto& requestExtension : m_RequestExtensions)
    {
        bool found = false;
        for (const auto& availableExtension : availableExtensions)
        {
            if (strcmp(requestExtension.c_str(), availableExtension.extensionName) == 0)
            {
                m_ActiveExtensions.push_back(availableExtension.extensionName);
                found = true;
                break;
            }
        }

        if (!found)
        {
            XR_TUT_LOG_ERROR("Requested extension not found: " << requestExtension);
        }
    }
}

void OpenXRCoreMgr::GetInstanceProperties()
{
    XrInstanceProperties instanceProperties = {XR_TYPE_INSTANCE_PROPERTIES};
    OPENXR_CHECK(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get OpenXR instance properties");
    XR_TUT_LOG("OpenXR Runtime: " << instanceProperties.runtimeName << " - " << XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
        << XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
        << XR_VERSION_PATCH(instanceProperties.runtimeVersion));
}