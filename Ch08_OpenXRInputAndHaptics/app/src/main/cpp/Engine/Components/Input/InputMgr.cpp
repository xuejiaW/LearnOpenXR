#include "InputMgr.h"

#include "../../../OpenXR/OpenXRInputMgr.h"

bool InputMgr::GetSelectDown(int handIndex)
{
    return OpenXRInputMgr::handStates[handIndex].currentSelectPressed && !OpenXRInputMgr::handStates[handIndex].lastSelectPressed;
}

bool InputMgr::GetSelect(int handIndex)
{
    return OpenXRInputMgr::handStates[handIndex].currentSelectPressed;
}

bool InputMgr::GetSelectUp(int handIndex)
{
    return !OpenXRInputMgr::handStates[handIndex].currentSelectPressed && OpenXRInputMgr::handStates[handIndex].lastSelectPressed;
}

XrPosef InputMgr::GetHandPose(int handIndex, bool* isActive)
{
    if (isActive) *isActive = OpenXRInputMgr::handStates[handIndex].poseActive;
    return OpenXRInputMgr::handStates[handIndex].pose;
}

void InputMgr::TriggerHapticFeedback(int handIndex, float amplitude, XrDuration duration)
{
     OpenXRInputMgr::TriggerHapticFeedback(handIndex,amplitude,duration);
}
