#pragma once
#include <openxr/openxr.h>

struct HandState
{
    bool currentSelectPressed = false;
    bool lastSelectPressed = false;
    XrPosef pose = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
    bool poseActive = false;
};