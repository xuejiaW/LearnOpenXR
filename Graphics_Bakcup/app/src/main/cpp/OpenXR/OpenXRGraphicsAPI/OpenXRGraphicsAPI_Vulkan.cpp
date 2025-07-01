// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

// OpenXR Tutorial for Khronos Group

#include "OpenXRGraphicsAPI_Vulkan.h"

#if defined(XR_USE_GRAPHICS_API_VULKAN) || defined(XR_TUTORIAL_USE_VULKAN)

#include <GraphicsAPI_Vulkan.h>
#include <OpenXRHelper.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

OpenXRGraphicsAPI_Vulkan::OpenXRGraphicsAPI_Vulkan(XrInstance xrInstance, XrSystemId systemId)
{
    // Load OpenXR function pointers
    LoadPFN_XrFunctions(xrInstance);

    // Get graphics requirements from OpenXR
    XrGraphicsRequirementsVulkanKHR graphicsRequirements{};
    graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;
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
    for (const auto& ext : instanceExtensions)
    {
        m_instanceExtensionNames.push_back(ext.c_str());
    }

    m_deviceExtensionNames.clear();
    for (const auto& ext : deviceExtensions)
    {
        m_deviceExtensionNames.push_back(ext.c_str());
    }

    // Create Vulkan instance first
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_instanceExtensionNames.size());
    instanceCreateInfo.ppEnabledExtensionNames = m_instanceExtensionNames.data();
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;

    VkInstance instance;
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }

    // Let OpenXR select the physical device
    VkPhysicalDevice physicalDevice;
    OPENXR_CHECK(xrGetVulkanGraphicsDeviceKHR(xrInstance, systemId, instance, &physicalDevice),
                 "Failed to get Vulkan graphics device from OpenXR.");

    // Create VulkanInitInfo for the GraphicsAPI_Vulkan constructor
    VulkanInitInfo initInfo{};
    initInfo.applicationInfo = appInfo;
    initInfo.instanceExtensions = m_instanceExtensionNames;
    initInfo.deviceExtensions = m_deviceExtensionNames;
    initInfo.instance = instance; // Use the instance we created
    initInfo.physicalDevice = physicalDevice; // Use OpenXR selected device
    initInfo.queueFamilyIndex = UINT32_MAX; // Let it find the graphics queue

    m_graphicsAPI = std::make_unique<GraphicsAPI_Vulkan>(initInfo);

    auto* vulkanAPI = static_cast<GraphicsAPI_Vulkan*>(m_graphicsAPI.get());

    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = vulkanAPI->GetInstance();
    graphicsBinding.physicalDevice = vulkanAPI->GetPhysicalDevice();
    graphicsBinding.device = vulkanAPI->GetDevice();
    graphicsBinding.queueFamilyIndex = vulkanAPI->GetQueueFamilyIndex();
    graphicsBinding.queueIndex = vulkanAPI->GetQueueIndex();
}

OpenXRGraphicsAPI_Vulkan::~OpenXRGraphicsAPI_Vulkan() {}

void OpenXRGraphicsAPI_Vulkan::LoadPFN_XrFunctions(XrInstance xrInstance)
{
    OPENXR_CHECK(xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsRequirementsKHR),
                 "Failed to get InstanceProcAddr for xrGetVulkanGraphicsRequirementsKHR.");
    OPENXR_CHECK(xrGetInstanceProcAddr(xrInstance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanInstanceExtensionsKHR),
                 "Failed to get InstanceProcAddr for xrGetVulkanInstanceExtensionsKHR.");
    OPENXR_CHECK(xrGetInstanceProcAddr(xrInstance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanDeviceExtensionsKHR),
                 "Failed to get InstanceProcAddr for xrGetVulkanDeviceExtensionsKHR.");
    OPENXR_CHECK(xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsDeviceKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsDeviceKHR),
                 "Failed to get InstanceProcAddr for xrGetVulkanGraphicsDeviceKHR.");
}

std::vector<std::string> OpenXRGraphicsAPI_Vulkan::GetInstanceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId)
{
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(xrInstance, systemId, 0, &extensionNamesSize, nullptr),
                 "Failed to get Vulkan Instance Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(xrInstance, systemId, extensionNamesSize, &extensionNamesSize, extensionNames.data()),
                 "Failed to get Vulkan Instance Extensions.");

    std::stringstream streamData(extensionNames.data());
    std::vector<std::string> extensions;
    std::string extension;
    while (std::getline(streamData, extension, ' '))
    {
        extensions.push_back(extension);
    }
    return extensions;
}

std::vector<std::string> OpenXRGraphicsAPI_Vulkan::GetDeviceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId)
{
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(xrInstance, systemId, 0, &extensionNamesSize, nullptr), "Failed to get Vulkan Device Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(xrInstance, systemId, extensionNamesSize, &extensionNamesSize, extensionNames.data()),
                 "Failed to get Vulkan Device Extensions.");

    std::stringstream streamData(extensionNames.data());
    std::vector<std::string> extensions;
    std::string extension;
    while (std::getline(streamData, extension, ' '))
    {
        extensions.push_back(extension);
    }
    return extensions;
}

void* OpenXRGraphicsAPI_Vulkan::GetGraphicsBinding()
{
    return &graphicsBinding;
}

XrSwapchainImageBaseHeader* OpenXRGraphicsAPI_Vulkan::AllocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count)
{
    XrSwapchainImageVulkanKHR swapchainImageTemplate{};
    swapchainImageTemplate.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
    swapchainImagesMap[swapchain].first = static_cast<int>(type);
    swapchainImagesMap[swapchain].second.resize(count, swapchainImageTemplate);
    return reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImagesMap[swapchain].second.data());
}

void OpenXRGraphicsAPI_Vulkan::FreeSwapchainImageData(XrSwapchain swapchain)
{
    swapchainImagesMap[swapchain].second.clear();
    swapchainImagesMap.erase(swapchain);
}

XrSwapchainImageBaseHeader* OpenXRGraphicsAPI_Vulkan::GetSwapchainImageData(XrSwapchain swapchain, uint32_t index)
{
    return reinterpret_cast<XrSwapchainImageBaseHeader*>(&swapchainImagesMap[swapchain].second[index]);
}

void* OpenXRGraphicsAPI_Vulkan::GetSwapchainImage(XrSwapchain swapchain, uint32_t index)
{
    return reinterpret_cast<void*>(swapchainImagesMap[swapchain].second[index].image);
}

#endif // XR_USE_GRAPHICS_API_VULKAN || XR_TUTORIAL_USE_VULKAN