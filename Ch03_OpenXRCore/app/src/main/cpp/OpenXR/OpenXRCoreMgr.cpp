#include "OpenXRCoreMgr.h"

#include "DebugOutput.h"
#include "HelperFunctions.h"
#include "OpenXRHelper.h"

XrInstance OpenXRCoreMgr::m_xrInstance = XR_NULL_HANDLE;

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
}

void OpenXRCoreMgr::DestroyInstance()
{
    OPENXR_CHECK(xrDestroyInstance(m_xrInstance), "Failed to destroy OpenXR instance");
}