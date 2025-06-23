#include "OpenXRSessionMgr.h"

#include "DebugOutput.h"
#include "OpenXRHelper.h"
#include "OpenXRCoreMgr.h"
#include "OpenXRDisplayMgr.h"
#include "OpenXRDisplay/RenderLayerInfo.h"

XrSessionState OpenXRSessionMgr::m_SessionState = XR_SESSION_STATE_UNKNOWN;
bool OpenXRSessionMgr::m_sessionRunning = false;
bool OpenXRSessionMgr::applicationRunning = true;
XrFrameState OpenXRSessionMgr::frameState{};


void OpenXRSessionMgr::PollEvent()
{
    XrEventDataBuffer eventDataBuffer{};
    eventDataBuffer.type = XR_TYPE_EVENT_DATA_BUFFER;

    auto XrPollEvent = [&]() -> bool
    {
        eventDataBuffer = {};
        eventDataBuffer.type = XR_TYPE_EVENT_DATA_BUFFER;
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

void OpenXRSessionMgr::OnSessionChanged(const XrEventDataSessionStateChanged* sessionStateChanged)
{

    m_SessionState = sessionStateChanged->state;
    XR_TUT_LOG("OpenXR session state changed: " << m_SessionState);

    if (m_SessionState == XR_SESSION_STATE_READY)
    {
        XrSessionBeginInfo sessionBeginInfo{};
        sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
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
    else if (m_SessionState == XR_SESSION_STATE_EXITING || m_SessionState == XR_SESSION_STATE_LOSS_PENDING)
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

bool OpenXRSessionMgr::ShouldRender()
{
    return frameState.shouldRender && IsSessionActive();
}

void OpenXRSessionMgr::WaitFrame()
{
    frameState.type = XR_TYPE_FRAME_STATE;
    frameState.next = nullptr;
    XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO, nullptr};
    OPENXR_CHECK(xrWaitFrame(OpenXRCoreMgr::xrSession, &frameWaitInfo, &frameState), "Failed to wait for OpenXR frame");
}

void OpenXRSessionMgr::BeginFrame()
{
    XrFrameBeginInfo frameBeginInfo{};
    frameBeginInfo.type = XR_TYPE_FRAME_BEGIN_INFO;
    OPENXR_CHECK(xrBeginFrame(OpenXRCoreMgr::xrSession, &frameBeginInfo), "Failed to begin OpenXR frame");
}

void OpenXRSessionMgr::EndFrame(const RenderLayerInfo& renderLayerInfo)
{
    XrFrameEndInfo frameEndInfo{};
    frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
    frameEndInfo.displayTime = frameState.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = OpenXRDisplayMgr::m_ActiveEnvironmentBlendMode;
    frameEndInfo.layerCount = static_cast<uint32_t>(renderLayerInfo.layers.size());
    frameEndInfo.layers = renderLayerInfo.layers.data();
    OPENXR_CHECK(xrEndFrame(OpenXRCoreMgr::xrSession, &frameEndInfo), "Failed to end the XR Frame.");
}