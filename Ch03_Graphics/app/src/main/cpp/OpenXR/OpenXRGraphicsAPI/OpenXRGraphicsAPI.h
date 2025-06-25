// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

// OpenXR Tutorial for Khronos Group

#pragma once

#include <openxr/openxr.h>
#include <memory>

// Forward declaration to avoid circular includes
class GraphicsAPI;

// Import SwapchainType enum to avoid incomplete type issues
enum class SwapchainType : uint8_t {
    COLOR,
    DEPTH
};

class OpenXRGraphicsAPI {
public:
    OpenXRGraphicsAPI() = default;
    virtual ~OpenXRGraphicsAPI() = default;

    // OpenXR-specific methods that must be implemented by derived classes
    virtual void* GetGraphicsBinding() = 0;
    virtual XrSwapchainImageBaseHeader* AllocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count) = 0;
    virtual void FreeSwapchainImageData(XrSwapchain swapchain) = 0;
    virtual XrSwapchainImageBaseHeader* GetSwapchainImageData(XrSwapchain swapchain, uint32_t index) = 0;
    virtual void* GetSwapchainImage(XrSwapchain swapchain, uint32_t index) = 0;

    // Access to underlying GraphicsAPI for rendering operations
    GraphicsAPI* GetGraphicsAPI() { return m_graphicsAPI.get(); }
    const GraphicsAPI* GetGraphicsAPI() const { return m_graphicsAPI.get(); }

protected:
    std::unique_ptr<GraphicsAPI> m_graphicsAPI;
};
