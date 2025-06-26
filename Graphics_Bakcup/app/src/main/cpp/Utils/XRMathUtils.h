#pragma once

#include <GraphicsAPI.h>
#include <openxr/openxr.h>
#include <xr_linear_algebra.h>

class XRMathUtils
{
public:
    static XrMatrix4x4f CreateProjectionMatrix(GraphicsAPI_Type apiType, 
                                               const XrFovf& fov, 
                                               float nearPlane, 
                                               float farPlane);

    static XrMatrix4x4f CreateViewMatrix(const XrPosef& pose);

    static XrMatrix4x4f CreateViewProjectionMatrix(GraphicsAPI_Type apiType,
                                                   const XrView& view,
                                                   float nearPlane,
                                                   float farPlane);

    static XrMatrix4x4f CreateModelMatrix(const XrPosef& pose, const XrVector3f& scale);

    static XrMatrix4x4f CreateModelViewProjectionMatrix(const XrMatrix4x4f& viewProj, 
                                                        const XrPosef& modelPose, 
                                                        const XrVector3f& modelScale);
};
