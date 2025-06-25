// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

// OpenXR Tutorial for Khronos Group

#include "OpenXRGraphicsAPI_Vulkan.h"

#if defined(XR_USE_GRAPHICS_API_VULKAN) || defined(XR_TUTORIAL_USE_VULKAN)

#include <GraphicsAPI_Vulkan.h>
#include <OpenXRHelper.h>
#include <iostream>
#include <algorithm>
#include <sstream>

OpenXRGraphicsAPI_Vulkan::OpenXRGraphicsAPI_Vulkan(XrInstance xrInstance, XrSystemId systemId) {
    // Load OpenXR function pointers
    LoadPFN_XrFunctions(xrInstance);

    // Get graphics requirements from OpenXR
    XrGraphicsRequirementsVulkanKHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR};
    OPENXR_CHECK(xrGetVulkanGraphicsRequirementsKHR(xrInstance, systemId, &graphicsRequirements), "Failed to get Graphics Requirements for Vulkan.");

    // Get required instance and device extensions
    std::vector<std::string> instanceExtensions = GetInstanceExtensionsForOpenXR(xrInstance, systemId);
    std::vector<std::string> deviceExtensions = GetDeviceExtensionsForOpenXR(xrInstance, systemId);

    // Prepare application info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "OpenXR Tutorial - Vulkan";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "OpenXR Tutorial - Vulkan Engine";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_MAKE_VERSION(
        XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported), 
        XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported), 0);

    // Convert extensions to C strings (need to keep them alive)
    m_instanceExtensionNames.clear();
    for (const auto& ext : instanceExtensions) {
        m_instanceExtensionNames.push_back(ext.c_str());
    }

    m_deviceExtensionNames.clear();
    for (const auto& ext : deviceExtensions) {
        m_deviceExtensionNames.push_back(ext.c_str());
    }

    // Create VulkanInitInfo for the GraphicsAPI_Vulkan constructor
    VulkanInitInfo initInfo{};
    initInfo.applicationInfo = appInfo;
    initInfo.instanceExtensions = m_instanceExtensionNames;
    initInfo.deviceExtensions = m_deviceExtensionNames;
    // Let GraphicsAPI_Vulkan create the instance and select physical device
    initInfo.instance = VK_NULL_HANDLE;
    initInfo.physicalDevice = VK_NULL_HANDLE;
    initInfo.queueFamilyIndex = UINT32_MAX; // Let it find the graphics queue

    // However, we need to use OpenXR's physical device selection
    // So we need a special path for this
    
    // For now, create GraphicsAPI_Vulkan with the gathered requirements
    // and let it handle OpenXR device selection internally
    m_graphicsAPI = std::make_unique<GraphicsAPI_Vulkan>(xrInstance, systemId);

    // Now we need to get the Vulkan objects from GraphicsAPI to set up binding
    auto* vulkanAPI = static_cast<GraphicsAPI_Vulkan*>(m_graphicsAPI.get());
    
    // Set up graphics binding for OpenXR
    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = vulkanAPI->GetInstance();
    graphicsBinding.physicalDevice = vulkanAPI->GetPhysicalDevice();
    graphicsBinding.device = vulkanAPI->GetDevice();
    graphicsBinding.queueFamilyIndex = vulkanAPI->GetQueueFamilyIndex();
    graphicsBinding.queueIndex = vulkanAPI->GetQueueIndex();
}

OpenXRGraphicsAPI_Vulkan::~OpenXRGraphicsAPI_Vulkan() {
    // Cleanup will be handled by GraphicsAPI_Vulkan destructor
}

void OpenXRGraphicsAPI_Vulkan::LoadPFN_XrFunctions(XrInstance xrInstance) {
    OPENXR_CHECK(xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsRequirementsKHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsRequirementsKHR.");
    OPENXR_CHECK(xrGetInstanceProcAddr(xrInstance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanInstanceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanInstanceExtensionsKHR.");
    OPENXR_CHECK(xrGetInstanceProcAddr(xrInstance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanDeviceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanDeviceExtensionsKHR.");
    OPENXR_CHECK(xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsDeviceKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsDeviceKHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsDeviceKHR.");
}

std::vector<std::string> OpenXRGraphicsAPI_Vulkan::GetInstanceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId) {
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(xrInstance, systemId, 0, &extensionNamesSize, nullptr), "Failed to get Vulkan Instance Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(xrInstance, systemId, extensionNamesSize, &extensionNamesSize, extensionNames.data()), "Failed to get Vulkan Instance Extensions.");

    std::stringstream streamData(extensionNames.data());
    std::vector<std::string> extensions;
    std::string extension;
    while (std::getline(streamData, extension, ' ')) {
        extensions.push_back(extension);
    }
    return extensions;
}

std::vector<std::string> OpenXRGraphicsAPI_Vulkan::GetDeviceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId) {
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(xrInstance, systemId, 0, &extensionNamesSize, nullptr), "Failed to get Vulkan Device Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(xrInstance, systemId, extensionNamesSize, &extensionNamesSize, extensionNames.data()), "Failed to get Vulkan Device Extensions.");

    std::stringstream streamData(extensionNames.data());
    std::vector<std::string> extensions;
    std::string extension;
    while (std::getline(streamData, extension, ' ')) {
        extensions.push_back(extension);
    }
    return extensions;
}

void* OpenXRGraphicsAPI_Vulkan::GetGraphicsBinding() {
    return &graphicsBinding;
}

XrSwapchainImageBaseHeader* OpenXRGraphicsAPI_Vulkan::AllocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count) {
    swapchainImagesMap[swapchain].first = static_cast<int>(type);
    swapchainImagesMap[swapchain].second.resize(count, {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR});
    return reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImagesMap[swapchain].second.data());
}

void OpenXRGraphicsAPI_Vulkan::FreeSwapchainImageData(XrSwapchain swapchain) {
    swapchainImagesMap[swapchain].second.clear();
    swapchainImagesMap.erase(swapchain);
}

XrSwapchainImageBaseHeader* OpenXRGraphicsAPI_Vulkan::GetSwapchainImageData(XrSwapchain swapchain, uint32_t index) {
    return reinterpret_cast<XrSwapchainImageBaseHeader*>(&swapchainImagesMap[swapchain].second[index]);
}

void* OpenXRGraphicsAPI_Vulkan::GetSwapchainImage(XrSwapchain swapchain, uint32_t index) {
    return reinterpret_cast<void*>(swapchainImagesMap[swapchain].second[index].image);
}

#endif // XR_USE_GRAPHICS_API_VULKAN || XR_TUTORIAL_USE_VULKAN
