#include "OpenXRTutorial.h"
#include <DebugOutput.h>

#include <GraphicsAPI_Vulkan.h>

GraphicsAPI_Type OpenXRTutorial::m_apiType = UNKNOWN;

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType)
{
    m_apiType = apiType;
}

OpenXRTutorial::~OpenXRTutorial() { }

void OpenXRTutorial::Run()
{
    while (true)
    {
        PollSystemEvents();
    }
}