#pragma once
#include <openxr/openxr.h>

class OpenXRSessionMgr
{
public:
    static void OnSessionChanged(XrEventDataSessionStateChanged* sessionStateChanged);
    static bool IsSessionActive();
    static bool IsSessionRunning();

    static bool applicationRunning;

private:
    static XrSessionState m_SessionState;
    static bool m_sessionRunning;
};