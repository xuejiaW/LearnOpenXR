#pragma once

#include <openxr/openxr.h>
#include <vector>
#include <string>

struct InteractionProfileBinding
{
    std::string interactionProfilePath;
    std::vector<XrActionSuggestedBinding> bindings;
};
