#pragma once
#include "OpenXRGraphicsAPI.h"

class OpenXRGraphicsAPI_Vulkan : public OpenXRGraphicsAPI
{
public:
    OpenXRGraphicsAPI_Vulkan(XrInstance xrInstance, XrSystemId systemID);
    ~OpenXRGraphicsAPI_Vulkan() override;

    void* GetGraphicsBinding() override;

    XrSwapchainImageBaseHeader* AllocateSwapchainImagesMemory(XrSwapchain swapchain, uint32_t count) override;
    void FreeSwapchainImagesMemory(XrSwapchain swapchain) override;
    XrSwapchainImageBaseHeader* GetSwapchainImageData(XrSwapchain swapchain, uint32_t index) override;
    void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) override;

private:
    void LoadXRFunctionsPointers(XrInstance xrInstance);
    PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR = nullptr;
    PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR = nullptr;
    PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR = nullptr;
    PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR = nullptr;
    XrGraphicsBindingVulkanKHR graphicsBinding{};

    std::vector<std::string> GetInstanceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId);
    std::vector<std::string> GetDeviceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId);

    // Key: xrSwapchain handle
    std::unordered_map<XrSwapchain, std::vector<XrSwapchainImageVulkanKHR>> swapchainImagesMap{};
};