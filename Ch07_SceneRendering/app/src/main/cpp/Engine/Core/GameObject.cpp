#include "GameObject.h"

void GameObject::PreTick(float deltaTime) {
    if (!m_active) return;
    
    for (auto& componentPair : m_components) {
        if (componentPair.second->IsEnabled()) {
            componentPair.second->PreTick(deltaTime);
        }
    }
}

void GameObject::Tick(float deltaTime) {
    if (!m_active) return;
    
    for (auto& componentPair : m_components) {
        if (componentPair.second->IsEnabled()) {
            componentPair.second->Tick(deltaTime);
        }
    }
}

void GameObject::PostTick(float deltaTime) {
    if (!m_active) return;
    
    for (auto& componentPair : m_components) {
        if (componentPair.second->IsEnabled()) {
            componentPair.second->PostTick(deltaTime);
        }
    }
}

void GameObject::Destroy() {
    for (auto& componentPair : m_components) {
        componentPair.second->Destroy();
    }
    m_components.clear();
}
