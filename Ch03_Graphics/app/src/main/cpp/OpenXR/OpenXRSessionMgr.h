#pragma once
#include <openxr/openxr.h>
#include "OpenXRDisplay/RenderLayerInfo.h"

class OpenXRSessionMgr
{
public:
    static void PollEvent();
    static void OnSessionChanged(const XrEventDataSessionStateChanged* sessionStateChanged);
    static bool IsSessionRunning();

    static bool ShouldRender();

    static void WaitFrame();
    static void BeginFrame();
    static void EndFrame(const RenderLayerInfo& renderLayerInfo);

    static bool applicationRunning;

    static XrFrameState frameState;

private:
    static bool IsSessionActive();
    static XrSessionState m_SessionState;
    static bool m_sessionRunning;
};