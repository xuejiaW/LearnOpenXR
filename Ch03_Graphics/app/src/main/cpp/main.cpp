#define XR_DOCS_CHAPTER_VERSION XR_DOCS_CHAPTER_1_4

#include <DebugOutput.h>
#include "OpenXRTutorial/OpenXRTutorial.h"

inline std::string GraphicsAPITypeToString(GraphicsAPI_Type type)
{
    const char* names[] = {"UNKNOWN", "D3D11", "D3D12", "OPENGL", "OPENGL_ES", "VULKAN"};
    int index = static_cast<int>(type);

    if (index >= 0 && index < sizeof(names) / sizeof(names[0]))
    {
        return std::string(names[index]) + " (" + std::to_string(index) + ")";
    }
    return "INVALID";
}

void OpenXRTutorial_Main(GraphicsAPI_Type apiType)
{
    DebugOutput debugOutput;
    XR_TUT_LOG("OpenXR Tutorial Ch03_Graphics");

    XR_TUT_LOG("Graphics API Type: " + GraphicsAPITypeToString(apiType));

    OpenXRTutorial app(apiType);
    app.Run();
}

