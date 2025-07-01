#pragma once

#include <openxr/openxr.h>

#include <string>
#include <vector>

class OpenXRCoreMgr
{
public:
    static void CreateInstance();
    static void DestroyInstance();

    static void GetSystemID();
    static XrSystemId systemID;
    static XrInstance m_xrInstance;

private:
    static void CreateRequiredExtensions(std::vector<std::string>& requiredExtensions);
    static void FindRequiredExtensions(const std::vector<std::string>& requestExtensions, std::vector<const char*>& activeExtensions);
};