#pragma once

#include <GraphicsAPI.h>
#include <openxr/openxr.h>

class OpenXRCoreMgr
{
public:
    static void CreateInstance();
    static void DestroyInstance();

    static void CreateDebugMessenger();
    static void DestroyDebugMessenger();

    static void GetSystemID();

    static void CreateSession();
    static void DestroySession();

    static void GetInstanceProperties();

    static XrInstance m_xrInstance;
    static XrSystemId systemID;
    static XrSession xrSession;
    static std::unique_ptr<GraphicsAPI> graphicsAPI;

private:
    static void ActiveAvailableApiLayers();
    static void ActiveAvailableExtensions();

    static std::vector<std::string> m_RequestApiLayers;
    static std::vector<const char*> m_ActiveApiLayers;

    static std::vector<std::string> m_RequestExtensions;
    static std::vector<const char*> m_ActiveExtensions;

    static XrDebugUtilsMessengerEXT m_DebugUtilsMessenger;
};