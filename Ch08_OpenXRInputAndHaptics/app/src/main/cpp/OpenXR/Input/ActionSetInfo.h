#pragma once

#include <openxr/openxr.h>
#include <vector>
#include <string>

#include "ActionInfo.h"

struct ActionSetInfo
{
    XrActionSet actionSet;
    std::string actionSetName;
    std::string localizedActionSetName;
    uint32_t priority;
    std::vector<ActionInfo> actions;
};
