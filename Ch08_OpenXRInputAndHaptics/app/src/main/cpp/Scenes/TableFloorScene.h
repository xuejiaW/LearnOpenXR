#pragma once

#include "../Engine/Core/Scene.h"
#include <GraphicsAPI.h>

class TableFloorScene : public Scene {
public:
    TableFloorScene();
    ~TableFloorScene();

    void Initialize(); // Builds objects
    void SetViewHeight(float heightInMeters) { m_viewHeightM = heightInMeters; }

private:
    float m_viewHeightM = 1.5f;
    void CreateSceneObjects();
};

