#pragma once
#include <openxr/openxr.h>

class OpenXRSessionMgr
{
public:
    static void PollEvent();
    static void OnSessionChanged(const XrEventDataSessionStateChanged* sessionStateChanged);
    static bool IsSessionRunning();
    static void WaitFrame();
    static void BeginFrame();
    static void EndFrame();

    static bool IsShouldRender();

    static XrFrameState frameState;

private:
    static XrSessionState m_xrSessionState;
    static bool m_IsSessionRunning;
};