#pragma once

#include <openxr/openxr.h>
#include <memory>
#include "GraphicsAPI.h"

class GraphicsAPI;

enum class SwapchainType : uint8_t
{
    COLOR,
    DEPTH
};

const char* GetGraphicsAPIInstanceExtensionString(GraphicsAPI_Type type);

class OpenXRGraphicsAPI
{
public:
    OpenXRGraphicsAPI() = default;
    virtual ~OpenXRGraphicsAPI() = default;

    virtual void* GetGraphicsBinding() = 0;
    virtual XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count) = 0;
    virtual void FreeSwapchainImageData(XrSwapchain swapchain) = 0;
    virtual XrSwapchainImageBaseHeader* GetSwapchainImageData(XrSwapchain swapchain, uint32_t index) = 0;
    virtual void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) = 0;

    GraphicsAPI* GetGraphicsAPI() { return m_graphicsAPI.get(); }
    const GraphicsAPI* GetGraphicsAPI() const { return m_graphicsAPI.get(); }

protected:
    std::unique_ptr<GraphicsAPI> m_graphicsAPI;
};