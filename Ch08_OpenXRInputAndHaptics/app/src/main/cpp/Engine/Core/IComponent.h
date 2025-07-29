#pragma once

class GameObject;

class IComponent {
protected:
    GameObject* m_gameObject = nullptr;
    bool m_enabled = true;

public:
    virtual ~IComponent() = default;
    
    virtual void Initialize() {}
    virtual void PreTick(float deltaTime) {}
    virtual void Tick(float deltaTime) {}
    virtual void PostTick(float deltaTime) {}
    virtual void Destroy() {}
    
    GameObject* GetGameObject() const { return m_gameObject; }
    void SetGameObject(GameObject* go) { m_gameObject = go; }
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }
};
