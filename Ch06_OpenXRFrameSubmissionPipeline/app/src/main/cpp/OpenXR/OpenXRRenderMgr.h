#pragma once
#include <openxr/openxr.h>

#include <vector>

#include "OpenXRDisplay/RenderLayerInfo.h"

class OpenXRRenderMgr
{
public:
    static void RefreshViewsData();
    static void UpdateRenderLayerInfo();

    static std::vector<XrView> views;
    static XrViewState viewState;
    static RenderLayerInfo renderLayerInfo;
};