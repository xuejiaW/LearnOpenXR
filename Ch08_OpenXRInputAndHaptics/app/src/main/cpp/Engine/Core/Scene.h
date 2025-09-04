#pragma once

#include <memory>
#include <string>
#include <vector>


class GameObject;
class Camera;

class Scene
{
  public:
    Scene(const std::string& name = "Scene");
    ~Scene();

    GameObject* CreateGameObject(const std::string& name = "GameObject");
    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_GameObjectsLists; }
    void DestroyGameObject(GameObject* gameObject);
    void Clear();

    void Update(float deltaTime);

    const std::string& GetName() const { return m_SceneName; }

    static void SetActiveCamera(Camera* camera);
    static Camera* GetActiveCamera();

  private:
    std::vector<std::unique_ptr<GameObject>> m_GameObjectsLists;
    std::string m_SceneName;

    static Camera* s_ActiveCamera;
};
