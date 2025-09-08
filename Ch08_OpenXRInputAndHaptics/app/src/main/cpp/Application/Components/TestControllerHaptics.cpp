#include "TestControllerHaptics.h"
#include "../../Engine/Components/Input/InputMgr.h"

void TestControllerHaptics::Tick(float deltaTime)
{
    if (InputMgr::GetSelectDown(0))
    {
        InputMgr::TriggerHapticFeedback(0, 1.0f, 100000000);
    }

    if (InputMgr::GetSelectDown(1))
    {
        InputMgr::TriggerHapticFeedback(1, 0.5f, 100000000);
    }
}
