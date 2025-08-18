#pragma once

#include "../../Core/IComponent.h"

class XRControllerDriver : public IComponent
{
public:
    void SetHandedness(int handedness);
    void PreTick(float deltaTime) override;

private:
    int m_Handedness = 0;
};