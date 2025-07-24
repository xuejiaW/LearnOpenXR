#pragma once

#include "Scenes/IScene.h"
#include <GraphicsAPI.h>

class TableFloorScene : public IScene
{
public:
    TableFloorScene();
    ~TableFloorScene() override;

    void Initialize() override;
    void Update(float deltaTime) override;
    const std::vector<SceneObject>& GetObjects() const override { return m_objects; }
    
    void SetViewHeight(float heightInMeters) { m_viewHeightM = heightInMeters; }

private:
    float m_viewHeightM = 1.5f;
    std::vector<SceneObject> m_objects;
    
    void CreateSceneObjects();
};
