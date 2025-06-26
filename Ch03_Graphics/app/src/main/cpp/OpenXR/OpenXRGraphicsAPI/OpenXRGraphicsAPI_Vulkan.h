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

class OpenXRGraphicsAPI_Vulkan : public OpenXRGraphicsAPI
{
public:
    OpenXRGraphicsAPI_Vulkan(XrInstance xrInstance, XrSystemId systemId);
    ~OpenXRGraphicsAPI_Vulkan() override;

    void* GetGraphicsBinding() override;
    XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count) override;
    void FreeSwapchainImageData(XrSwapchain swapchain) override;
    XrSwapchainImageBaseHeader* GetSwapchainImageData(XrSwapchain swapchain, uint32_t index) override;
    void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) override;

private:
    void LoadPFN_XrFunctions(XrInstance xrInstance);
    std::vector<std::string> GetInstanceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId);
    std::vector<std::string> GetDeviceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId);

    struct VulkanRequirements
    {
        XrVersion minApiVersionSupported;
        XrVersion maxApiVersionSupported;
    };

    struct VulkanDeviceInfo
    {
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

    XrGraphicsBindingVulkanKHR graphicsBinding{};

    std::unordered_map<XrSwapchain, std::pair<int, std::vector<XrSwapchainImageVulkanKHR>>> swapchainImagesMap{};

    std::vector<const char*> m_instanceExtensionNames;
    std::vector<const char*> m_deviceExtensionNames;
};

#endif // XR_USE_GRAPHICS_API_VULKAN || XR_TUTORIAL_USE_VULKAN