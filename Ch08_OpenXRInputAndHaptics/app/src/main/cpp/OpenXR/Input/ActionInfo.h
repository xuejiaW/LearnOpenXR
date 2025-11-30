#pragma once

#include <openxr/openxr.h>
#include <vector>
#include <string>

struct ActionInfo
{
    XrAction action;
    XrActionType actionType;
    std::string actionName;
    std::string localizedActionName;
    std::vector<XrPath> subactionPaths;
};
