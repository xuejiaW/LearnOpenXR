// ReSharper disable CppStringLiteralToCharPointerConversion
#include "OpenXRGraphicsAPI_Vulkan.h"

#include <GraphicsAPI_Vulkan.h>
#include <OpenXRHelper.h>
#include <iostream>

OpenXRGraphicsAPI_Vulkan::OpenXRGraphicsAPI_Vulkan(XrInstance xrInstance, XrSystemId systemID)
{
    // LoadXRFunctionsPointers(xrInstance);
}

OpenXRGraphicsAPI_Vulkan::~OpenXRGraphicsAPI_Vulkan()
{
    // Cleanup if necessary
}

void* OpenXRGraphicsAPI_Vulkan::GetGraphicsBinding()
{
    return nullptr;
}

void OpenXRGraphicsAPI_Vulkan::LoadXRFunctionsPointers(XrInstance xrInstance)
{
    OPENXR_CHECK(
        xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&
            xrGetVulkanGraphicsRequirementsKHR)), "Failed to get PFN for xrGetVulkanGraphicsRequirementsKHR");

    OPENXR_CHECK(
        xrGetInstanceProcAddr(xrInstance, "xrGetVulkanInstanceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanInstanceExtensionsKHR)
        ), "Failed to get InstanceProcAddr for xrGetVulkanInstanceExtensionsKHR.");

    OPENXR_CHECK(
        xrGetInstanceProcAddr(xrInstance, "xrGetVulkanDeviceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanDeviceExtensionsKHR)),
        "Failed to get InstanceProcAddr for xrGetVulkanDeviceExtensionsKHR.");

    OPENXR_CHECK(
        xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsDeviceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanGraphicsDeviceKHR)),
        "Failed to get InstanceProcAddr for xrGetVulkanGraphicsDeviceKHR.");
}