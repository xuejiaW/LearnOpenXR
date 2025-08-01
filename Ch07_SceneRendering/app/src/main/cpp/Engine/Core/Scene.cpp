#include "Scene.h"
#include "GameObject.h"
#include "../Components/Rendering/Camera.h"
#include <algorithm>

// Define static member variable
Camera* Scene::s_activeCamera = nullptr;

Scene::Scene(const std::string& name) : m_sceneName(name) {
}

Scene::~Scene() {
    Clear();
}

GameObject* Scene::CreateGameObject(const std::string& name) {
    auto gameObject = std::make_unique<GameObject>(name);
    GameObject* ptr = gameObject.get();
    m_gameObjects.push_back(std::move(gameObject));
    return ptr;
}

void Scene::DestroyGameObject(GameObject* gameObject) {
    auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
        [gameObject](const std::unique_ptr<GameObject>& obj) {
            return obj.get() == gameObject;
        });
    
    if (it != m_gameObjects.end()) {
        (*it)->Destroy();
        m_gameObjects.erase(it);
    }
}

void Scene::Clear() {
    for (auto& gameObject : m_gameObjects) {
        gameObject->Destroy();
    }
    m_gameObjects.clear();
}

void Scene::Update(float deltaTime) {
    for (auto& gameObject : m_gameObjects) {
        if (gameObject->IsActive()) {
            gameObject->PreTick(deltaTime);
        }
    }
    
    for (auto& gameObject : m_gameObjects) {
        if (gameObject->IsActive()) {
            gameObject->Tick(deltaTime);
        }
    }
    
    for (auto& gameObject : m_gameObjects) {
        if (gameObject->IsActive()) {
            gameObject->PostTick(deltaTime);
        }
    }
}

void Scene::SetActiveCamera(Camera* camera) {
    s_activeCamera = camera;
}

Camera* Scene::GetActiveCamera() {
    return s_activeCamera;
}
