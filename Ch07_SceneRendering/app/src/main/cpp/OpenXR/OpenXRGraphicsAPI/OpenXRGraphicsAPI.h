#pragma once
#include "GraphicsAPI.h"

class OpenXRGraphicsAPI
{
public:
    virtual ~OpenXRGraphicsAPI() = default;
    OpenXRGraphicsAPI() = default;

    static const char* GetGraphicsAPIInstanceExtensionString(GraphicsAPI_Type type);

    virtual void* GetGraphicsBinding() = 0;

    virtual XrSwapchainImageBaseHeader* AllocateSwapchainImagesMemory(XrSwapchain swapchain, uint32_t count) = 0;
    virtual void FreeSwapchainImagesMemory(XrSwapchain swapchain) = 0;
    virtual XrSwapchainImageBaseHeader* GetSwapchainImageData(XrSwapchain swapchain, uint32_t index) = 0;
    virtual void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) = 0;

    std::unique_ptr<GraphicsAPI> graphicsAPI;
};