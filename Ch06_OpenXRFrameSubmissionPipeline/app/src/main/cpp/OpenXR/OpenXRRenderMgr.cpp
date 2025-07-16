#include "OpenXRRenderMgr.h"

#include <openxr/openxr.h>

#include "DebugOutput.h"
#include "OpenXRCoreMgr.h"
#include "OpenXRDisplayMgr.h"
#include "OpenXRHelper.h"
#include "OpenXRSessionMgr.h"
#include "OpenXRSpaceMgr.h"


std::vector<XrView> OpenXRRenderMgr::views{};
XrViewState OpenXRRenderMgr::viewState{};

void OpenXRRenderMgr::RefreshViewsData()
{
    XrView viewTemplate{};
    viewTemplate.type = XR_TYPE_VIEW;

    uint32_t viewsCount = static_cast<uint32_t>(OpenXRDisplayMgr::GetViewsCount());

    views.resize(viewsCount, viewTemplate);
    viewState.type = XR_TYPE_VIEW_STATE;

    XrViewLocateInfo viewLocateInfo{};
    viewLocateInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
    viewLocateInfo.viewConfigurationType = OpenXRDisplayMgr::activeViewConfigurationType;
    viewLocateInfo.displayTime = OpenXRSessionMgr::frameState.predictedDisplayTime;
    viewLocateInfo.space = OpenXRSpaceMgr::activeSpaces;

    OPENXR_CHECK(xrLocateViews(OpenXRCoreMgr::xrSession, &viewLocateInfo, &viewState, viewsCount, &viewsCount, views.data()),
                 "Failed to locate OpenXR views");
}