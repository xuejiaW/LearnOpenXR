#pragma once
#include <openxr/openxr.h>
#include <vector>

struct RenderLayerInfo
{
    XrTime predictedDisplayTime = 0;
    std::vector<XrCompositionLayerBaseHeader*> layers;
    XrCompositionLayerProjection projectionLayer = {XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
};