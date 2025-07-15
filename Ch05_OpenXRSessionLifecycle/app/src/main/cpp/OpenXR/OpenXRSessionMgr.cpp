// ReSharper disable CppClangTidyClangDiagnosticSwitchEnum
#include "OpenXRSessionMgr.h"

#include "DebugOutput.h"
#include "OpenXRCoreMgr.h"

XrSessionState OpenXRSessionMgr::m_xrSessionState = XR_SESSION_STATE_UNKNOWN;
bool OpenXRSessionMgr::m_IsSessionRunning = false;

void OpenXRSessionMgr::PollEvent()
{
    XrEventDataBuffer eventDataBuffer{};
    eventDataBuffer.type = XR_TYPE_EVENT_DATA_BUFFER;

    bool pollSucceed = xrPollEvent(OpenXRCoreMgr::m_xrInstance, &eventDataBuffer) == XR_SUCCESS;

    while (pollSucceed)
    {
        switch (eventDataBuffer.type)
        {
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            {
                auto* sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventDataBuffer);
                OnSessionChanged(sessionStateChanged);
                break;
            }
            default:
            {
                XR_TUT_LOG("OpenXR event data type: " << eventDataBuffer.type);
                break;
            }
        }

        eventDataBuffer = {};
        eventDataBuffer.type = XR_TYPE_EVENT_DATA_BUFFER;
        pollSucceed = xrPollEvent(OpenXRCoreMgr::m_xrInstance, &eventDataBuffer) == XR_SUCCESS;
    }
}

void OpenXRSessionMgr::OnSessionChanged(const XrEventDataSessionStateChanged* sessionStateChanged)
{
    m_xrSessionState = sessionStateChanged->state;
    XR_TUT_LOG("OpenXR session state changed: " << XR_ENUM_STR(m_xrSessionState));

    if (m_xrSessionState == XR_SESSION_STATE_READY)
    {
        XrSessionBeginInfo sessionBeginInfo{};
        sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
        sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        OPENXR_CHECK(xrBeginSession(OpenXRCoreMgr::OpenXRCoreMgr::xrSession, &sessionBeginInfo), "Failed to begin OpenXR session");
        XR_TUT_LOG("OpenXR session started successfully");
        m_IsSessionRunning = true;
    }
    else if (m_xrSessionState == XR_SESSION_STATE_STOPPING)
    {
        OPENXR_CHECK(xrEndSession(OpenXRCoreMgr::xrSession), "Failed to end OpenXR session");
        m_IsSessionRunning = false;
    }
    else if (m_xrSessionState == XR_SESSION_STATE_EXITING || m_xrSessionState == XR_SESSION_STATE_LOSS_PENDING)
    {
        OpenXRCoreMgr::DestroySession();
    }
}

bool OpenXRSessionMgr::IsSessionRunning()
{
    return m_IsSessionRunning;
}