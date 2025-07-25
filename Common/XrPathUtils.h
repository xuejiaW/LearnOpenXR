#pragma once

#include <openxr/openxr.h>
#include <string>
#include <unordered_map>

class XRPathUtils
{
public:
    // Path conversion functions
    static XrPath StringToPath(XrInstance instance, const std::string& pathString);
    static std::string PathToString(XrInstance instance, XrPath path);
    
    // Path caching for performance
    static void ClearPathCache();
    
private:
    static std::unordered_map<std::string, XrPath> m_StringToPathCache;
    static std::unordered_map<XrPath, std::string> m_PathToStringCache;
};
