#include "XRHmdDriver.h"
#include "Camera.h"
#include "../Core/Transform.h"
#include "../../Core/GameObject.h"
#include "../../../OpenXR/OpenXRDisplayMgr.h"
#include "../../../OpenXR/OpenXRRenderMgr.h"

void XRHmdDriver::PreTick(float deltaTime) {
    int currentViewIndex = OpenXRDisplayMgr::GetCurrentViewIndex();
    if (currentViewIndex >= 0) {
        SyncFromOpenXR();
    }
}

void XRHmdDriver::SyncFromOpenXR() {
    int viewIndex = OpenXRDisplayMgr::GetCurrentViewIndex();
    
    UpdateTransformFromOpenXR(viewIndex);
    UpdateCameraFOVFromOpenXR(viewIndex);
}

void XRHmdDriver::UpdateTransformFromOpenXR(int viewIndex) {
    Transform* transform = GetGameObject()->GetComponent<Transform>();
    if (transform) {
        const XrPosef& pose = OpenXRRenderMgr::views[viewIndex].pose;
        transform->SetPosition(pose.position);
        transform->SetRotation(pose.orientation);
    }
}

void XRHmdDriver::UpdateCameraFOVFromOpenXR(int viewIndex) {
    Camera* camera = GetGameObject()->GetComponent<Camera>();
    if (camera) {
        const XrFovf& fov = OpenXRRenderMgr::views[viewIndex].fov;
        camera->SetFieldOfView(fov);
    }
}
