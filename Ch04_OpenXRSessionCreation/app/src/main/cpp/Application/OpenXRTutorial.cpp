#include "OpenXRTutorial.h"

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
    OpenXRCoreMgr::GetSystemID();
    OpenXRCoreMgr::CreateSession(m_apiType);
}

void OpenXRTutorial::ShutDownOpenXR()
{
    OpenXRCoreMgr::DestroySession();
    OpenXRCoreMgr::DestroyInstance();
}