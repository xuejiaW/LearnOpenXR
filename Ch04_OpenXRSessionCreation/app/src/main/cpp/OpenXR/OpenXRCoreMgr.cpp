#include "OpenXRCoreMgr.h"

#include "DebugOutput.h"
#include "HelperFunctions.h"
#include "OpenXRHelper.h"
#include "../Application/OpenXRTutorial.h"
#include "OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "OpenXRGraphicsAPI/OpenXRGraphicsAPI_Vulkan.h"

XrInstance OpenXRCoreMgr::m_xrInstance = XR_NULL_HANDLE;
XrSystemId OpenXRCoreMgr::systemID = XR_NULL_SYSTEM_ID;
XrSession OpenXRCoreMgr::xrSession = XR_NULL_SYSTEM_ID;

std::unique_ptr<OpenXRGraphicsAPI> OpenXRCoreMgr::openxrGraphicsAPI = nullptr;

void OpenXRCoreMgr::CreateInstance()
{
    XrApplicationInfo appInfo = {};
    strncpy(appInfo.applicationName, "OpenXRTutorial Ch03_Graphics", XR_MAX_APPLICATION_NAME_SIZE);
    appInfo.applicationVersion = 1;
    strncpy(appInfo.engineName, "OpenXRTutorial Engine", XR_MAX_ENGINE_NAME_SIZE);
    appInfo.engineVersion = 1;
    appInfo.apiVersion = XR_CURRENT_API_VERSION;

    std::vector<std::string> requiredExtensions{};
    std::vector<const char*> activeExtensions{};
    CreateRequiredExtensions(requiredExtensions);
    FindRequiredExtensions(requiredExtensions, activeExtensions);

    XrInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.applicationInfo = appInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(activeExtensions.size());
    instanceCreateInfo.enabledExtensionNames = activeExtensions.data();
    OPENXR_CHECK(xrCreateInstance(&instanceCreateInfo, &m_xrInstance), "Failed to create OpenXR instance");
    XR_TUT_LOG("OpenXR instance created successfully");

    XrInstanceProperties instanceProperties = {};
    instanceProperties.type = XR_TYPE_INSTANCE_PROPERTIES;
    OPENXR_CHECK(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get OpenXR instance properties");
    XR_TUT_LOG("OpenXR Runtime: " << instanceProperties.runtimeName << " - " << XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
        << XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
        << XR_VERSION_PATCH(instanceProperties.runtimeVersion));
}

void OpenXRCoreMgr::DestroyInstance()
{
    OPENXR_CHECK(xrDestroyInstance(m_xrInstance), "Failed to destroy OpenXR instance");
}

void OpenXRCoreMgr::GetSystemID()
{
    XrSystemGetInfo systemGetInfo{};
    systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    OPENXR_CHECK(xrGetSystem(m_xrInstance, &systemGetInfo, &systemID), "Failed to get OpenXR system ID");
    XR_TUT_LOG("OpenXR System ID: " << systemID);

    XrSystemProperties systemProperties{};
    systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
    OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, systemID, &systemProperties), "Failed to get OpenXR system properties");

    XR_TUT_LOG("OpenXR System Properties: " << systemProperties.systemName << " - " << systemProperties.systemId);
    XR_TUT_LOG("OpenXR System Graphics Properties: ");
    XR_TUT_LOG("OpenXR Max layer count - " << systemProperties.graphicsProperties.maxLayerCount);
    XR_TUT_LOG("OpenXR Max swapchain image width - " << systemProperties.graphicsProperties.maxSwapchainImageWidth);
    XR_TUT_LOG("OpenXR Max Swapchain image height - " << systemProperties.graphicsProperties.maxSwapchainImageHeight);
    XR_TUT_LOG("OpenXR Orientation Tracking - " << (systemProperties.trackingProperties.orientationTracking ? "enabled" : "disabled"));
    XR_TUT_LOG("OpenXR Position tracking - " << (systemProperties.trackingProperties.positionTracking ? "enabled" : "disabled"));
}

void OpenXRCoreMgr::CreateRequiredExtensions(std::vector<std::string>& requiredExtensions)
{
    requiredExtensions.clear();
    requiredExtensions.emplace_back(OpenXRGraphicsAPI::GetGraphicsAPIInstanceExtensionString(OpenXRTutorial::m_apiType));
}

void OpenXRCoreMgr::FindRequiredExtensions(const std::vector<std::string>& requestExtensions, std::vector<const char*>& activeExtensions)
{
    uint32_t extensionCount = 0;
    OPENXR_CHECK(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr), "Failed to enumerate OpenXR instance extensions");

    XrExtensionProperties extPropsTemplate = {};
    extPropsTemplate.type = XR_TYPE_EXTENSION_PROPERTIES;
    std::vector<XrExtensionProperties> availableExtensions(extensionCount, extPropsTemplate);
    OPENXR_CHECK(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, availableExtensions.data()),
                 "Failed to enumerate OpenXR instance extensions");

    for (const auto& requestExtension : requestExtensions)
    {
        bool found = false;
        for (const auto& ext : availableExtensions)
        {
            if (strcmp(ext.extensionName, requestExtension.c_str()) == 0)
            {
                activeExtensions.push_back(ext.extensionName);
                found = true;
                break;
            }
        }

        if (!found)
        {
            XR_TUT_LOG("Required OpenXR Extension " << requestExtension << " not found");
        }
    }
}

void OpenXRCoreMgr::CreateSession(GraphicsAPI_Type apiType)
{
    XrSessionCreateInfo sessionCreateInfo{};
    sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    if (apiType == VULKAN)
    {
        openxrGraphicsAPI = std::make_unique<OpenXRGraphicsAPI_Vulkan>(m_xrInstance, systemID);
    }
    else
    {
        XR_TUT_LOG_ERROR("Unsupported Graphics API type for OpenXR session creation: " << apiType);
        return;
    }
    sessionCreateInfo.next = openxrGraphicsAPI->GetGraphicsBinding();
    sessionCreateInfo.systemId = systemID;

    OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCreateInfo, &xrSession), "Failed to create OpenXR session");

    XR_TUT_LOG("Session Created");
}

void OpenXRCoreMgr::DestroySession()
{
    OPENXR_CHECK(xrDestroySession(xrSession), "Failed to destroy OpenXR session");
}