#include "OpenXRTutorial.h"
#include <DebugOutput.h> // For XR_TUTORIAL_GRAPHICS_API (assuming it's defined via this or similar headers)
// If XR_TUTORIAL_GRAPHICS_API is defined elsewhere, ensure that header is included.
// For example, if it's in a config header or passed via build system.

// Forward declaration from main.cpp
// This might need to be adjusted if OpenXRTutorial_Main is also moved or its declaration changes.
void OpenXRTutorial_Main(GraphicsAPI_Type apiType);

#if !defined(__ANDROID__) && defined(_WIN32)
void OpenXRTutorial::PollSystemEvents() {
    // Windows-specific event polling if any.
    // Original was just return;
    return;
}

int main(int argc, char **argv) {
    OpenXRTutorial_Main(XR_TUTORIAL_GRAPHICS_API); // XR_TUTORIAL_GRAPHICS_API needs to be defined
    return 0;
}
#endif
