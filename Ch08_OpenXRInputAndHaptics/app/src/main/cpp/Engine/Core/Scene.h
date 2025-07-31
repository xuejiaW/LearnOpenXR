#pragma once

#include <vector>
#include <memory>
#include <string>

class GameObject;
class Camera;

class Scene {
private:
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    std::string m_sceneName;
    
    // Static active camera for easy access across the scene
    static Camera* s_activeCamera;

public:
    Scene(const std::string& name = "Scene");
    ~Scene();
    
    GameObject* CreateGameObject(const std::string& name = "GameObject");
    void DestroyGameObject(GameObject* gameObject);
    void Clear();
    
    void Update(float deltaTime);
    
    const std::string& GetName() const { return m_sceneName; }
    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_gameObjects; }
    
    static void SetActiveCamera(Camera* camera);
    static Camera* GetActiveCamera();
};
