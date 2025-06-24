#pragma once

#include <GraphicsAPI.h>
#include <openxr/openxr.h>
#include <xr_linear_algebra.h>

// 数学工具类，用于常见的 OpenXR 矩阵计算
class XRMathUtils
{
public:
    // 创建投影矩阵
    static XrMatrix4x4f CreateProjectionMatrix(GraphicsAPI_Type apiType, 
                                               const XrFovf& fov, 
                                               float nearPlane, 
                                               float farPlane);

    // 创建视图矩阵
    static XrMatrix4x4f CreateViewMatrix(const XrPosef& pose);

    // 创建视图投影矩阵
    static XrMatrix4x4f CreateViewProjectionMatrix(GraphicsAPI_Type apiType,
                                                   const XrView& view,
                                                   float nearPlane,
                                                   float farPlane);

    // 创建模型矩阵
    static XrMatrix4x4f CreateModelMatrix(const XrPosef& pose, const XrVector3f& scale);

    // 创建模型视图投影矩阵
    static XrMatrix4x4f CreateModelViewProjectionMatrix(const XrMatrix4x4f& viewProj, 
                                                        const XrPosef& modelPose, 
                                                        const XrVector3f& modelScale);
};
