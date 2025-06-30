#include "OpenXRCoreMgr.h"

#include "DebugOutput.h"
#include "HelperFunctions.h"
#include "OpenXRHelper.h"

XrInstance OpenXRCoreMgr::m_xrInstance = XR_NULL_HANDLE;
XrSystemId OpenXRCoreMgr::systemID = XR_NULL_SYSTEM_ID;

void OpenXRCoreMgr::CreateInstance()
{
    XrApplicationInfo appInfo = {};
    strncpy(appInfo.applicationName, "OpenXRTutorial Ch03_Graphics", XR_MAX_APPLICATION_NAME_SIZE);
    appInfo.applicationVersion = 1;
    strncpy(appInfo.engineName, "OpenXRTutorial Engine", XR_MAX_ENGINE_NAME_SIZE);
    appInfo.engineVersion = 1;
    appInfo.apiVersion = XR_CURRENT_API_VERSION;

    XrInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.applicationInfo = appInfo;
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
    XrSystemGetInfo systemGetInfo;
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