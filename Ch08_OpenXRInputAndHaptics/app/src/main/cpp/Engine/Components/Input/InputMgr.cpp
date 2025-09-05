#include "InputMgr.h"

#include "../../../OpenXR/OpenXRInputMgr.h"

bool InputMgr::GetSelectDown(int handIndex)
{
    return OpenXRInputMgr::controllerStates[handIndex].currentSelectPressed && !OpenXRInputMgr::controllerStates[handIndex].lastSelectPressed;
}

bool InputMgr::GetSelect(int handIndex)
{
    return OpenXRInputMgr::controllerStates[handIndex].currentSelectPressed;
}

bool InputMgr::GetSelectUp(int handIndex)
{
    return !OpenXRInputMgr::controllerStates[handIndex].currentSelectPressed && OpenXRInputMgr::controllerStates[handIndex].lastSelectPressed;
}

XrPosef InputMgr::GetHandPose(int handIndex, bool* isActive)
{
    *isActive = OpenXRInputMgr::controllerStates[handIndex].poseActive;
    return OpenXRInputMgr::controllerStates[handIndex].pose;
}

void InputMgr::TriggerHapticFeedback(int handIndex, float amplitude, XrDuration duration)
{
    OpenXRInputMgr::TriggerHapticFeedback(handIndex, amplitude, duration);
}