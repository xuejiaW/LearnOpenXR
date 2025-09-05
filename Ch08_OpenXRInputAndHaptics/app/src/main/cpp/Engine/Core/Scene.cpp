#include "Scene.h"
#include "GameObject.h"
#include "../Components/Rendering/Camera.h"
#include <algorithm>

Camera* Scene::s_ActiveCamera = nullptr;

Scene::Scene(const std::basic_string<char>& name) : m_SceneName(name) {}

Scene::~Scene()
{
    Clear();
}

GameObject* Scene::CreateGameObject(const std::string& name)
{
    auto gameObject = std::make_unique<GameObject>(name);
    GameObject* ptr = gameObject.get();
    m_GameObjectsLists.push_back(std::move(gameObject));
    return ptr;
}

void Scene::DestroyGameObject(GameObject* gameObject)
{
    auto it = std::find_if(m_GameObjectsLists.begin(), m_GameObjectsLists.end(), [gameObject](const std::unique_ptr<GameObject>& obj)
    {
        return obj.get() == gameObject;
    });

    if (it != m_GameObjectsLists.end())
    {
        (*it)->Destroy();
        m_GameObjectsLists.erase(it);
    }
}

void Scene::Clear()
{
    for (auto& gameObject : m_GameObjectsLists)
    {
        gameObject->Destroy();
    }
    m_GameObjectsLists.clear();
}

void Scene::Update(float deltaTime)
{
    for (auto& gameObject : m_GameObjectsLists)
    {
        if (gameObject->IsActive())
        {
            gameObject->PreTick(deltaTime);
        }
    }

    for (auto& gameObject : m_GameObjectsLists)
    {
        if (gameObject->IsActive())
        {
            gameObject->Tick(deltaTime);
        }
    }

    for (auto& gameObject : m_GameObjectsLists)
    {
        if (gameObject->IsActive())
        {
            gameObject->PostTick(deltaTime);
        }
    }
}

void Scene::SetActiveCamera(Camera* camera)
{
    s_ActiveCamera = camera;
}

Camera* Scene::GetActiveCamera()
{
    return s_ActiveCamera;
}