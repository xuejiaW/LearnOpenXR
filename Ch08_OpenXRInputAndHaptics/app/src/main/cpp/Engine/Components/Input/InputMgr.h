#pragma once
#include <openxr/openxr.h>

class InputMgr
{
public:
    static bool GetSelectDown(int handIndex);  // Button just pressed
    static bool GetSelect(int handIndex);      // Button held down
    static bool GetSelectUp(int handIndex);    // Button just released
    static XrPosef GetHandPose(int handIndex, bool* isActive = nullptr);
    static void TriggerHapticFeedback(int handIndex, float amplitude = 0.5f, XrDuration duration = 100000000);
};
