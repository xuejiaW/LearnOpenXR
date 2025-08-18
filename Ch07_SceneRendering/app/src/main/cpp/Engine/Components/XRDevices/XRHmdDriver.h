#pragma once

#include "../../Core/IComponent.h"

class XRHmdDriver : public IComponent {
public:
    void PreTick(float deltaTime) override;
    
private:
    void SyncFromOpenXR();
    void UpdateTransformFromOpenXR(int viewIndex);
    void UpdateCameraFOVFromOpenXR(int viewIndex);
};
