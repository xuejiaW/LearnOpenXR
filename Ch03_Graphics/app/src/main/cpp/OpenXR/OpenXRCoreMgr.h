#pragma once

#include <openxr/openxr.h>
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class OpenXRGraphicsAPI;
class GraphicsAPI;

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

    static void CreateReferenceSpaces();
    static void DestroyReferenceSpace();    static XrInstance m_xrInstance;
    static XrSystemId systemID;
    static XrSession xrSession;
    static std::unique_ptr<OpenXRGraphicsAPI> openxrGraphicsAPI;
    static GraphicsAPI* GetGraphicsAPI();
    
    static OpenXRGraphicsAPI* GetOpenXRGraphicsAPI();

    static XrSpace m_ActiveSpaces;

private:
    static void ActiveAvailableApiLayers();
    static void ActiveAvailableExtensions();

    static std::vector<std::string> m_RequestApiLayers;
    static std::vector<const char*> m_ActiveApiLayers;

    static std::vector<std::string> m_RequestExtensions;
    static std::vector<const char*> m_ActiveExtensions;

    static XrDebugUtilsMessengerEXT m_DebugUtilsMessenger;
};