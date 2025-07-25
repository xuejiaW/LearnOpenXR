#include "XRPathUtils.h"
#include "DebugOutput.h"
#include "OpenXRHelper.h"

std::unordered_map<std::string, XrPath> XRPathUtils::m_StringToPathCache;
std::unordered_map<XrPath, std::string> XRPathUtils::m_PathToStringCache;

XrPath XRPathUtils::StringToPath(XrInstance instance, const std::string& pathString)
{
    // Check cache first
    auto it = m_StringToPathCache.find(pathString);
    if (it != m_StringToPathCache.end())
    {
        return it->second;
    }
    
    // Create new path
    XrPath xrPath;
    OPENXR_CHECK(xrStringToPath(instance, pathString.c_str(), &xrPath), 
                 "Failed to create XrPath from string");
    
    // Cache the result
    m_StringToPathCache[pathString] = xrPath;
    m_PathToStringCache[xrPath] = pathString;
    
    return xrPath;
}

std::string XRPathUtils::PathToString(XrInstance instance, XrPath path)
{
    // Check cache first
    auto it = m_PathToStringCache.find(path);
    if (it != m_PathToStringCache.end())
    {
        return it->second;
    }
    
    // Convert path to string
    uint32_t stringLength;
    char text[XR_MAX_PATH_LENGTH];
    XrResult result = xrPathToString(instance, path, XR_MAX_PATH_LENGTH, &stringLength, text);
    
    std::string pathString;
    if (result == XR_SUCCESS)
    {
        pathString = text;
        // Cache the result
        m_PathToStringCache[path] = pathString;
        m_StringToPathCache[pathString] = path;
    }
    else
    {
        OPENXR_CHECK(result, "Failed to retrieve path string");
    }
    
    return pathString;
}

void XRPathUtils::ClearPathCache()
{
    m_StringToPathCache.clear();
    m_PathToStringCache.clear();
}
