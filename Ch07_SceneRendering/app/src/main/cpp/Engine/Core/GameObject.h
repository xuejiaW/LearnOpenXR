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
    
    void PreTick(float deltaTime);
    void Tick(float deltaTime);
    void PostTick(float deltaTime);
    void Destroy();
    
    const std::string& GetName() const { return m_name; }
    bool IsActive() const { return m_active; }
};
