#include "OpenXRSessionMgr.h"

#include "DebugOutput.h"
#include "OpenXRHelper.h"
#include "OpenXRCoreMgr.h"

XrSessionState OpenXRSessionMgr::m_SessionState = XR_SESSION_STATE_UNKNOWN;
bool OpenXRSessionMgr::m_sessionRunning = false;
bool OpenXRSessionMgr::applicationRunning = true;

void OpenXRSessionMgr::PollEvent()
{
    XrEventDataBuffer eventDataBuffer{XR_TYPE_EVENT_DATA_BUFFER};

    auto XrPollEvent = [&]() -> bool
    {
        eventDataBuffer = {XR_TYPE_EVENT_DATA_BUFFER};
        return xrPollEvent(OpenXRCoreMgr::m_xrInstance, &eventDataBuffer) == XR_SUCCESS;
    };

    while (XrPollEvent())
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
    }
}

void OpenXRSessionMgr::OnSessionChanged(XrEventDataSessionStateChanged* sessionStateChanged)
{

    m_SessionState = sessionStateChanged->state;
    XR_TUT_LOG("OpenXR session state changed: " << m_SessionState);

    if (m_SessionState == XR_SESSION_STATE_READY)
    {
        XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
        sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        OPENXR_CHECK(xrBeginSession(OpenXRCoreMgr::OpenXRCoreMgr::xrSession, &sessionBeginInfo), "Failed to begin OpenXR session");
        XR_TUT_LOG("OpenXR session started successfully");
        m_sessionRunning = true;
    }
    else if (m_SessionState == XR_SESSION_STATE_STOPPING)
    {
        OPENXR_CHECK(xrEndSession(OpenXRCoreMgr::xrSession), "Failed to end OpenXR session");
        m_sessionRunning = false;
    }
    else if (m_SessionState == XR_SESSION_STATE_EXITING)
    {
        m_sessionRunning = false;
        applicationRunning = false;
    }
    else if (m_SessionState == XR_SESSION_STATE_LOSS_PENDING)
    {
        m_sessionRunning = false;
        applicationRunning = false;
    }
}

bool OpenXRSessionMgr::IsSessionActive()
{
    return m_sessionRunning && (m_SessionState == XR_SESSION_STATE_SYNCHRONIZED || m_SessionState == XR_SESSION_STATE_VISIBLE ||
                                m_SessionState == XR_SESSION_STATE_FOCUSED);
}

bool OpenXRSessionMgr::IsSessionRunning()
{
    return m_sessionRunning;
}

