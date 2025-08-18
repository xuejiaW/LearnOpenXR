#include "XRControllerDriver.h"
#include "../Core/Transform.h"
#include "../Rendering/Camera.h"
#include "../../Core/GameObject.h"
#include "../Input/InputMgr.h"

void XRControllerDriver::SetHandedness(int handedness)
{
   m_Handedness = handedness; 
}

void XRControllerDriver::PreTick(float deltaTime)
{
    Transform* transform = GetGameObject()->GetComponent<Transform>();
    if (transform) {
        bool poseActive;
        const XrPosef& pose = InputMgr::GetHandPose(m_Handedness, &poseActive);
        transform->SetPosition(pose.position);
        transform->SetRotation(pose.orientation);
    }
}
