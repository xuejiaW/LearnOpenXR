#pragma once

#include <vector>
#include <memory>
#include <string>

class GameObject;
class Camera;

class Scene {
public:
    Scene(const std::string& name = "Scene");
    ~Scene();
    
    GameObject* CreateGameObject(const std::string& name = "GameObject");
    void DestroyGameObject(GameObject* gameObject);
    void Clear();
    
    void Update(float deltaTime);
    
    const std::string& GetName() const { return m_SceneName; }
    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_GameObjectsLists; }
    
    static void SetActiveCamera(Camera* camera);
    static Camera* GetActiveCamera();
private:
    std::vector<std::unique_ptr<GameObject>> m_GameObjectsLists;
    std::string m_SceneName;
    
    static Camera* s_ActiveCamera;
};
