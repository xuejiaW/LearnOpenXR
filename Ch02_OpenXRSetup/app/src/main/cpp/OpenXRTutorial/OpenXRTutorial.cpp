#include "OpenXRTutorial.h"
#include <DebugOutput.h>

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType) : m_apiType(apiType)
{
}

OpenXRTutorial::~OpenXRTutorial() = default;

void OpenXRTutorial::Run()
{
    while (m_applicationRunning)
    {
        PollSystemEvents();
    }
}
