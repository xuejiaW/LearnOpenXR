#pragma once
#include <openxr/openxr.h>
#include <vector>

struct SwapchainInfo
{
    XrSwapchain swapchain = XR_NULL_HANDLE;
    int64_t swapchainFormat = 0;
    std::vector<void*> imageViews;
};