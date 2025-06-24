#pragma once
#include <openxr/openxr.h>
#include <vector>

struct RenderLayerInfo
{
    std::vector<XrCompositionLayerBaseHeader*> layers;
    XrCompositionLayerProjection projectionLayer;
    std::vector<XrCompositionLayerProjectionView> layerProjectionViews;

    RenderLayerInfo()
    {
        projectionLayer = {};
        projectionLayer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
        layers = {};
        layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&projectionLayer));
        
    }
};