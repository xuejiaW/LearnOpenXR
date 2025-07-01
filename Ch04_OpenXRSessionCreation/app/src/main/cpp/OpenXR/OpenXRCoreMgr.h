#pragma once

#include <openxr/openxr.h>

#include <memory>
#include <string>
#include <vector>

#include "GraphicsAPI.h"

class OpenXRGraphicsAPI;
class OpenXRCoreMgr
{
public:
    static void CreateInstance();
    static void DestroyInstance();

    static void GetSystemID();

    static void CreateSession(GraphicsAPI_Type apiType);
    static void DestroySession();
    
    static XrSystemId systemID;
    static XrInstance m_xrInstance;
    static std::unique_ptr<OpenXRGraphicsAPI> openxrGraphicsAPI;
private:
    static void CreateRequiredExtensions(std::vector<std::string>& requiredExtensions);
    static void FindRequiredExtensions(const std::vector<std::string>& requestExtensions, std::vector<const char*>& activeExtensions);
};