#pragma once

class GameObject;

class IComponent {
protected:
    GameObject* m_GameObject = nullptr;
    bool m_Enabled = true;

public:
    virtual ~IComponent() = default;
    
    virtual void Initialize() {}
    virtual void PreTick(float deltaTime) {}
    virtual void Tick(float deltaTime) {}
    virtual void PostTick(float deltaTime) {}
    virtual void Destroy() {}
    
    GameObject* GetGameObject() const { return m_GameObject; }
    void SetGameObject(GameObject* go) { m_GameObject = go; }
    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
};
