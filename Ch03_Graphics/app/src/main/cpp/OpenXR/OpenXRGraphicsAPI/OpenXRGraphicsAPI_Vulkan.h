// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

// OpenXR Tutorial for Khronos Group

#pragma once

#include "OpenXRGraphicsAPI.h"

#if defined(XR_USE_GRAPHICS_API_VULKAN) || defined(XR_TUTORIAL_USE_VULKAN)
#include <vulkan/vulkan.h>
#include <openxr/openxr.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr_platform.h>

#include <string>
#include <vector>
#include <unordered_map>

class OpenXRGraphicsAPI_Vulkan : public OpenXRGraphicsAPI {
public:
    OpenXRGraphicsAPI_Vulkan(XrInstance xrInstance, XrSystemId systemId);
    ~OpenXRGraphicsAPI_Vulkan();

    // OpenXR-specific implementations
    virtual void* GetGraphicsBinding() override;
    virtual XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count) override;
    virtual void FreeSwapchainImageData(XrSwapchain swapchain) override;
    virtual XrSwapchainImageBaseHeader* GetSwapchainImageData(XrSwapchain swapchain, uint32_t index) override;
    virtual void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) override;

private:
    void LoadPFN_XrFunctions(XrInstance xrInstance);
    std::vector<std::string> GetInstanceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId);
    std::vector<std::string> GetDeviceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId);
    
    // Vulkan data structures for OpenXR integration
    struct VulkanRequirements {
        XrVersion minApiVersionSupported;
        XrVersion maxApiVersionSupported;
    };
    
    struct VulkanDeviceInfo {
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        uint32_t queueFamilyIndex;
        uint32_t queueIndex;
    };

    // OpenXR function pointers
    PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR = nullptr;
    PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR = nullptr;
    PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR = nullptr;
    PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR = nullptr;

    // Graphics binding for OpenXR
    XrGraphicsBindingVulkanKHR graphicsBinding{};

    // Swapchain image management
    std::unordered_map<XrSwapchain, std::pair<int, std::vector<XrSwapchainImageVulkanKHR>>> swapchainImagesMap{};

    // Keep extension name strings alive
    std::vector<const char*> m_instanceExtensionNames;
    std::vector<const char*> m_deviceExtensionNames;
};

#endif // XR_USE_GRAPHICS_API_VULKAN || XR_TUTORIAL_USE_VULKAN
