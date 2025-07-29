#pragma once

#include <string>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include "IComponent.h"

class GameObject {
private:
    std::string m_name;
    std::unordered_map<std::type_index, std::unique_ptr<IComponent>> m_components;
    bool m_active = true;

public:
    GameObject(const std::string& name = "GameObject") : m_name(name) {}
    
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        std::type_index typeIndex = std::type_index(typeid(T));
        
        if (m_components.find(typeIndex) != m_components.end()) {
            return static_cast<T*>(m_components[typeIndex].get());
        }
        
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* componentPtr = component.get();
        component->SetGameObject(this);
        component->Initialize();
        
        m_components[typeIndex] = std::move(component);
        return componentPtr;
    }
    
    template<typename T>
    T* GetComponent() {
        std::type_index typeIndex = std::type_index(typeid(T));
        auto it = m_components.find(typeIndex);
        if (it != m_components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }
    
    template<typename T>
    bool HasComponent() {
        std::type_index typeIndex = std::type_index(typeid(T));
        return m_components.find(typeIndex) != m_components.end();
    }
    
    template<typename T>
    void RemoveComponent() {
        std::type_index typeIndex = std::type_index(typeid(T));
        auto it = m_components.find(typeIndex);
        if (it != m_components.end()) {
            it->second->Destroy();
            m_components.erase(it);
        }
    }
    
    void PreTick(float deltaTime);
    void Tick(float deltaTime);
    void PostTick(float deltaTime);
    void Destroy();
    
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }
    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }
};
