#include "OpenXRTutorial.h"
#include <DebugOutput.h>

#include <GraphicsAPI_Vulkan.h>

#include "../OpenXR/OpenXRCoreMgr.h"

GraphicsAPI_Type OpenXRTutorial::m_apiType = UNKNOWN;

OpenXRTutorial::OpenXRTutorial(GraphicsAPI_Type apiType)
{
    m_apiType = apiType;
}

OpenXRTutorial::~OpenXRTutorial() {}

void OpenXRTutorial::Run()
{
    InitializeOpenXR();
    while (true)
    {
        PollSystemEvents();
    }
}

void OpenXRTutorial::InitializeOpenXR()
{
    OpenXRCoreMgr::CreateInstance();
}

void OpenXRTutorial::ShutDownOpenXR()
{
    OpenXRCoreMgr::DestroyInstance();
}