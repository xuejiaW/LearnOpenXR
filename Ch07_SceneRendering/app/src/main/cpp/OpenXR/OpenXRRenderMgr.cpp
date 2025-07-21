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
RenderLayerInfo OpenXRRenderMgr::renderLayerInfo{};

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

void OpenXRRenderMgr::UpdateRenderLayerInfo()
{
    XrCompositionLayerProjectionView layerProjectionViewTemplate = {};
    layerProjectionViewTemplate.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
    renderLayerInfo.layerProjectionViews.resize(views.size(), layerProjectionViewTemplate);

    renderLayerInfo.projectionLayer.layerFlags =
        XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
    renderLayerInfo.projectionLayer.space = OpenXRSpaceMgr::activeSpaces;
    renderLayerInfo.projectionLayer.viewCount = static_cast<uint32_t>(renderLayerInfo.layerProjectionViews.size());
    renderLayerInfo.projectionLayer.views = renderLayerInfo.layerProjectionViews.data();

    for (int viewIndex = 0; viewIndex != static_cast<int>(views.size()); ++viewIndex)
    {
        const uint32_t& width = OpenXRDisplayMgr::activeViewConfigurationViews[viewIndex].recommendedImageRectWidth;
        const uint32_t& height = OpenXRDisplayMgr::activeViewConfigurationViews[viewIndex].recommendedImageRectHeight;

        renderLayerInfo.layerProjectionViews[viewIndex].pose = views[viewIndex].pose;
        renderLayerInfo.layerProjectionViews[viewIndex].fov = views[viewIndex].fov;
        renderLayerInfo.layerProjectionViews[viewIndex].subImage.swapchain = OpenXRDisplayMgr::colorSwapchainInfos[viewIndex].swapchain;
        renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageRect.offset.x = 0;
        renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageRect.offset.y = 0;
        renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageRect.extent.width = static_cast<int32_t>(width);
        renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageRect.extent.height = static_cast<int32_t>(height);
        renderLayerInfo.layerProjectionViews[viewIndex].subImage.imageArrayIndex = 0;
    }
}