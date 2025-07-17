// ReSharper disable CppClangTidyClangDiagnosticSwitchEnum
#include "OpenXRSessionMgr.h"

#include "DebugOutput.h"
#include "OpenXRCoreMgr.h"
#include "OpenXRDisplayMgr.h"
#include "OpenXRRenderMgr.h"

XrSessionState OpenXRSessionMgr::m_xrSessionState = XR_SESSION_STATE_UNKNOWN;
bool OpenXRSessionMgr::m_IsSessionRunning = false;
XrFrameState OpenXRSessionMgr::frameState{};

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

void OpenXRSessionMgr::EndFrame(const bool rendered)
{
    XrFrameEndInfo frameEndInfo{};
    frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
    frameEndInfo.displayTime = frameState.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    frameEndInfo.layerCount = rendered ? static_cast<uint32_t>(OpenXRRenderMgr::renderLayerInfo.layers.size()) : 0;
    frameEndInfo.layers = rendered ? OpenXRRenderMgr::renderLayerInfo.layers.data() : nullptr;
    OPENXR_CHECK(xrEndFrame(OpenXRCoreMgr::xrSession, &frameEndInfo), "Failed to end the XR Frame.");
}

bool OpenXRSessionMgr::IsShouldRender()
{
    return frameState.shouldRender && m_IsSessionRunning
           && (m_xrSessionState == XR_SESSION_STATE_FOCUSED || m_xrSessionState == XR_SESSION_STATE_VISIBLE);
}