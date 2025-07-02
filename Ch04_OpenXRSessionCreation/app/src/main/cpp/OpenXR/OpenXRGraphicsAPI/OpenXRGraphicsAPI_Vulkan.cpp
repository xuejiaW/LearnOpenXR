// ReSharper disable CppStringLiteralToCharPointerConversion
#include "OpenXRGraphicsAPI_Vulkan.h"

#include <GraphicsAPI_Vulkan.h>
#include <OpenXRHelper.h>

#include "DebugOutput.h"


OpenXRGraphicsAPI_Vulkan::OpenXRGraphicsAPI_Vulkan(XrInstance xrInstance, XrSystemId systemID)
{
    LoadXRFunctionsPointers(xrInstance);

    XrGraphicsRequirementsVulkanKHR graphicsReqs{};
    graphicsReqs.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;
    xrGetVulkanGraphicsRequirementsKHR(xrInstance, systemID, &graphicsReqs);

    std::vector<std::string> instanceExtensions = GetInstanceExtensionsForOpenXR(xrInstance, systemID);
    std::vector<std::string> deviceExtensions = GetDeviceExtensionsForOpenXR(xrInstance, systemID);

    static std::vector<const char*> activeInstanceExtensions;
    for (const auto& ext : instanceExtensions) { activeInstanceExtensions.push_back(ext.c_str()); }

    static std::vector<const char*> activeDeviceExtensions;
    for (const auto& ext : deviceExtensions) { activeDeviceExtensions.push_back(ext.c_str()); }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "OpenXR Tutorial - Vulkan";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "OpenXR Tutorial - Vulkan Engine";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_MAKE_VERSION(XR_VERSION_MAJOR(graphicsReqs.minApiVersionSupported),
                                         XR_VERSION_MINOR(graphicsReqs.minApiVersionSupported), 0);

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(activeInstanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = activeInstanceExtensions.data();
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;

    VkInstance vkInstance;
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);
    if (result != VK_SUCCESS)
    {
        std::cerr << "Failed to create Vulkan instance: " << result << '\n';
        DEBUG_BREAK;
    }

    VkPhysicalDevice physicalDevice;
    OPENXR_CHECK(xrGetVulkanGraphicsDeviceKHR(xrInstance, systemID, vkInstance, &physicalDevice),
                 "Failed to get Vulkan graphics device from OpenXR.");

    VulkanInitInfo initInfo{};
    initInfo.instanceExtensions = activeInstanceExtensions;
    initInfo.deviceExtensions = activeDeviceExtensions;
    initInfo.instance = vkInstance;// Use the instance we created
    initInfo.physicalDevice = physicalDevice; // Use OpenXR selected device

    graphicsAPI = std::make_unique<GraphicsAPI_Vulkan>(initInfo);

    auto* vulkanAPI = dynamic_cast<GraphicsAPI_Vulkan*>(graphicsAPI.get());

    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = vulkanAPI->GetInstance();
    graphicsBinding.physicalDevice = vulkanAPI->GetPhysicalDevice();
    graphicsBinding.device = vulkanAPI->GetDevice();
    graphicsBinding.queueFamilyIndex = vulkanAPI->GetQueueFamilyIndex();
    graphicsBinding.queueIndex = vulkanAPI->GetQueueIndex();

}

static std::vector<std::string> ParseExtensionString(const std::vector<char>& extensionNames)
{
    std::stringstream streamData(extensionNames.data());
    std::vector<std::string> extensions;
    std::string extension;
    while (std::getline(streamData, extension, ' '))
    {
        extensions.push_back(extension);
    }
    return extensions;
}

std::vector<std::string> OpenXRGraphicsAPI_Vulkan::GetInstanceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId)
{
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(xrInstance, systemId, 0, &extensionNamesSize, nullptr),
                 "Failed to get Vulkan Instance Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanInstanceExtensionsKHR(xrInstance, systemId, extensionNamesSize, &extensionNamesSize, extensionNames.data()),
                 "Failed to get Vulkan Instance Extensions.");

    XR_TUT_LOG("Vulkan Instance Extensions: " << std::string(extensionNames.data(), extensionNamesSize));

    return ParseExtensionString(extensionNames);
}

std::vector<std::string> OpenXRGraphicsAPI_Vulkan::GetDeviceExtensionsForOpenXR(XrInstance xrInstance, XrSystemId systemId)
{
    uint32_t extensionNamesSize = 0;
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(xrInstance, systemId, 0, &extensionNamesSize, nullptr),
                 "Failed to get Vulkan Device Extensions.");

    std::vector<char> extensionNames(extensionNamesSize);
    OPENXR_CHECK(xrGetVulkanDeviceExtensionsKHR(xrInstance, systemId, extensionNamesSize, &extensionNamesSize, extensionNames.data()),
                 "Failed to get Vulkan Device Extensions.");

    XR_TUT_LOG("Vulkan Device Extensions: " << std::string(extensionNames.data(), extensionNamesSize));

    return ParseExtensionString(extensionNames);
}

OpenXRGraphicsAPI_Vulkan::~OpenXRGraphicsAPI_Vulkan() = default;

void* OpenXRGraphicsAPI_Vulkan::GetGraphicsBinding()
{
    return &graphicsBinding;
}

void OpenXRGraphicsAPI_Vulkan::LoadXRFunctionsPointers(XrInstance xrInstance)
{
    xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsRequirementsKHR",
                          reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanGraphicsRequirementsKHR));
    xrGetInstanceProcAddr(xrInstance, "xrGetVulkanInstanceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanInstanceExtensionsKHR));
    xrGetInstanceProcAddr(xrInstance, "xrGetVulkanDeviceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanDeviceExtensionsKHR));
    xrGetInstanceProcAddr(xrInstance, "xrGetVulkanGraphicsDeviceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanGraphicsDeviceKHR));
}