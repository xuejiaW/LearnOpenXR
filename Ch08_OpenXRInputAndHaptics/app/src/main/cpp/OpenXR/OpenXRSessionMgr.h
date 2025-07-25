#pragma once
#include <openxr/openxr.h>

class OpenXRSessionMgr
{
public:
    static void PollEvent();
    static void OnSessionChanged(const XrEventDataSessionStateChanged* sessionStateChanged);
    static bool IsSessionRunning();
    static bool IsShouldRender();
    static bool IsShouldProcessInput();
    static void WaitFrame();
    static void BeginFrame();
    static void EndFrame(bool rendered);

    static XrFrameState frameState;

private:
    static XrSessionState m_xrSessionState;
    static bool m_IsSessionRunning;
};