#pragma once
#include <openxr/openxr.h>

#include <functional>

struct SwapchainConfig
{
    XrSwapchainUsageFlags usageFlags;
    bool isDepth;
    std::function<int64_t(const std::vector<int64_t>&)> formatSelector;
};
