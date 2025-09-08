#pragma once

#include <openxr/openxr.h>
#include <string>

struct ActionSetInfo
{
    XrActionSet actionSet;
    std::string actionSetName;
    std::string localizedActionSetName;
    uint32_t priority;
};