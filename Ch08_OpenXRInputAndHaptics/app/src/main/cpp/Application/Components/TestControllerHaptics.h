#pragma once
#include "../../Engine/Core/IComponent.h"

class TestControllerHaptics : public IComponent
{
public:
   void Tick(float deltaTime) override; 
};