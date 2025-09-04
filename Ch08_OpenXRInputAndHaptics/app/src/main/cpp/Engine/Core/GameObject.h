// ReSharper disable CppClangTidyCppcoreguidelinesMissingStdForward
#pragma once

#include <string>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include "IComponent.h"

class GameObject
{
public:
    GameObject(const std::string& name = "GameObject") : m_Name(name) {}

    template <typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_ComponentsLists.find(typeIndex) != m_ComponentsLists.end())
        {
            return static_cast<T*>(m_ComponentsLists[typeIndex].get());
        }

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* componentPtr = component.get();
        component->SetGameObject(this);
        component->Initialize();

        m_ComponentsLists[typeIndex] = std::move(component);
        return componentPtr;
    }

    template <typename T>
    T* GetComponent()
    {
        std::type_index typeIndex = std::type_index(typeid(T));
        auto it = m_ComponentsLists.find(typeIndex);
        if (it != m_ComponentsLists.end())
        {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    void PreTick(float deltaTime);
    void Tick(float deltaTime);
    void PostTick(float deltaTime);
    void Destroy();

    const std::string& GetName() const { return m_Name; }
    bool IsActive() const { return m_Active; }

private:
    std::string m_Name;
    std::unordered_map<std::type_index, std::unique_ptr<IComponent>> m_ComponentsLists;
    bool m_Active = true;
};