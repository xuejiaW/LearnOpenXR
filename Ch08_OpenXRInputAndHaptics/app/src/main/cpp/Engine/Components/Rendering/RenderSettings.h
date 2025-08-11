#pragma once

#include <openxr/openxr.h>

struct RenderSettings {
    uint32_t width = 0;
    uint32_t height = 0;
    void* colorImage = nullptr;
    void* depthImage = nullptr;
    XrEnvironmentBlendMode blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    XrVector4f clearColor = {0.0f, 0.0f, 0.2f, 1.0f};
    void* pipeline = nullptr;
};
