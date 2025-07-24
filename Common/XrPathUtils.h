#pragma once

#include <openxr/openxr.h>
#include <string>
#include <unordered_map>

class XrPathUtils
{
public:
    // Path conversion functions
    static XrPath StringToPath(XrInstance instance, const std::string& pathString);
    static std::string PathToString(XrInstance instance, XrPath path);
    
    // Path caching for performance
    static void ClearPathCache();
    
    // Common path constants
    static const char* GetHandPath(int handIndex); // 0 = left, 1 = right
    static const char* GetSimpleControllerProfilePath();
    static const char* GetTouchControllerProfilePath();
    
private:
    static std::unordered_map<std::string, XrPath> m_StringToPathCache;
    static std::unordered_map<XrPath, std::string> m_PathToStringCache;
    
    // Common path strings
    static constexpr const char* HAND_LEFT_PATH = "/user/hand/left";
    static constexpr const char* HAND_RIGHT_PATH = "/user/hand/right";
    static constexpr const char* SIMPLE_CONTROLLER_PROFILE = "/interaction_profiles/khr/simple_controller";
    static constexpr const char* TOUCH_CONTROLLER_PROFILE = "/interaction_profiles/oculus/touch_controller";
};
