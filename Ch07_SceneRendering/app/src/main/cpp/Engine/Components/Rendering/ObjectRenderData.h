#pragma once
#include <xr_linear_algebra.h>

struct ObjectRenderData
{
    XrMatrix4x4f viewProj;
    XrMatrix4x4f modelViewProj;
    XrMatrix4x4f model;
    XrVector4f color;
    XrVector4f pad1, pad2, pad3;
};