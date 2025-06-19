#include "OpenXRTutorial.h"
// If XR_TUTORIAL_GRAPHICS_API is defined elsewhere, ensure that header is included.
// For example, if it's in a config header or passed via build system.

// Forward declaration from main.cpp
// This might need to be adjusted if OpenXRTutorial_Main is also moved or its declaration changes.
void OpenXRTutorial_Main(GraphicsAPI_Type apiType);

#if !defined(__ANDROID__) && defined(_WIN32)
void OpenXRTutorial::PollSystemEvents() {
}

#endif
