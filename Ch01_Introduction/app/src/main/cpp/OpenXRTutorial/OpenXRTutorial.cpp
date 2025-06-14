#include "OpenXRTutorial.h"
#include <DebugOutput.h>

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType) : m_apiType(apiType) {
    // Constructor implementation will be platform-specific or common
    // For now, keeping it minimal as in the original main.cpp
}

OpenXRTutorial::~OpenXRTutorial() = default;

void OpenXRTutorial::Run() {
    // Common Run logic here
    // The original Run() was empty, so keeping it that way for now.
    // Platform-specific PollSystemEvents will be called within a loop here if needed.
    while(m_applicationRunning) {
        PollSystemEvents();
        // Main loop logic would go here if any
    }
}

// Note: PollSystemEvents is platform-specific and will be in separate files.
// The main entry points (main and android_main) will also be in separate files.
