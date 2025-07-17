#include "XRMathUtils.h"

XrMatrix4x4f XRMathUtils::CreateProjectionMatrix(GraphicsAPI_Type apiType, 
                                                 const XrFovf& fov, 
                                                 float nearPlane, 
                                                 float farPlane)
{
    XrMatrix4x4f proj;
    XrMatrix4x4f_CreateProjectionFov(&proj, apiType, fov, nearPlane, farPlane);
    return proj;
}

XrMatrix4x4f XRMathUtils::CreateViewMatrix(const XrPosef& pose)
{
    XrMatrix4x4f toView;
    XrVector3f scale1m{1.0f, 1.0f, 1.0f};
    XrMatrix4x4f_CreateTranslationRotationScale(&toView, &pose.position, &pose.orientation, &scale1m);
    
    XrMatrix4x4f view;
    XrMatrix4x4f_InvertRigidBody(&view, &toView);
    
    return view;
}

XrMatrix4x4f XRMathUtils::CreateViewProjectionMatrix(GraphicsAPI_Type apiType,
                                                     const XrView& view,
                                                     float nearPlane,
                                                     float farPlane)
{
    XrMatrix4x4f proj = CreateProjectionMatrix(apiType, view.fov, nearPlane, farPlane);
    XrMatrix4x4f viewMatrix = CreateViewMatrix(view.pose);
    
    XrMatrix4x4f viewProj;
    XrMatrix4x4f_Multiply(&viewProj, &proj, &viewMatrix);
    
    return viewProj;
}

XrMatrix4x4f XRMathUtils::CreateModelMatrix(const XrPosef& pose, const XrVector3f& scale)
{
    XrMatrix4x4f model;
    XrMatrix4x4f_CreateTranslationRotationScale(&model, &pose.position, &pose.orientation, &scale);
    return model;
}

XrMatrix4x4f XRMathUtils::CreateModelViewProjectionMatrix(const XrMatrix4x4f& viewProj, 
                                                          const XrPosef& modelPose, 
                                                          const XrVector3f& modelScale)
{
    XrMatrix4x4f model = CreateModelMatrix(modelPose, modelScale);
    
    XrMatrix4x4f mvp;
    XrMatrix4x4f_Multiply(&mvp, &viewProj, &model);
    
    return mvp;
}
