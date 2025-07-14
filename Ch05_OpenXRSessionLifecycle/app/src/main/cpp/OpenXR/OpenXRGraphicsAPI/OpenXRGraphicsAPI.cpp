#include "OpenXRGraphicsAPI.h"
#include <openxr/openxr.h>

const char* OpenXRGraphicsAPI::GetGraphicsAPIInstanceExtensionString(GraphicsAPI_Type type)
{
    if (type == VULKAN)
    {
        return XR_KHR_VULKAN_ENABLE_EXTENSION_NAME;
    }
    
    std::cerr << "ERROR: Unknown Graphics API." << '\n';
    DEBUG_BREAK;
    return nullptr;
}