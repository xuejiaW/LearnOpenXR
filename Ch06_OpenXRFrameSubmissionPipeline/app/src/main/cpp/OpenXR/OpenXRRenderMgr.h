#pragma once
#include <openxr/openxr.h>

#include <vector>

class OpenXRRenderMgr
{
public:
    static void RefreshViewsData();
    static std::vector<XrView> views;
    static XrViewState viewState;
};