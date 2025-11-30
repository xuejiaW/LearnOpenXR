#include "TestControllerHaptics.h"
#include "../../Engine/Components/Input/InputMgr.h"
#include "../../OpenXR/OpenXRInputMgr.h"

void TestControllerHaptics::Tick(float deltaTime)
{
    if (InputMgr::GetSelectDown(1))
    {
        OpenXRInputMgr::TriggerHapticFeedback(1, 0.5f, 100000000);
    }
}
