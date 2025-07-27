#pragma once

#include "Core/Scene.h"
#include <GraphicsAPI.h>

class TableFloorScene {
public:
    TableFloorScene();
    ~TableFloorScene();

    void Initialize();
    void Update(float deltaTime);
    Scene* GetScene() const { return m_scene.get(); }
    
    void SetViewHeight(float heightInMeters) { m_viewHeightM = heightInMeters; }

private:
    float m_viewHeightM = 1.5f;
    std::unique_ptr<Scene> m_scene;
    
    void CreateSceneObjects();
};
