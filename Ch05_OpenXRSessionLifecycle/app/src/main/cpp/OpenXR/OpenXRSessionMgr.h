#pragma once
#include <openxr/openxr.h>

class OpenXRSessionMgr
{
public:
    static void PollEvent();
    static void OnSessionChanged(const XrEventDataSessionStateChanged* sessionStateChanged);
    static bool IsSessionRunning();

private:
    static XrSessionState m_xrSessionState;
    static bool m_IsSessionRunning;
};