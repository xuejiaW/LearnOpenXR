#pragma once

#include "../../Core/IComponent.h"
#include <openxr/openxr.h>
#include <xr_linear_algebra.h>

class Transform : public IComponent {
private:
    XrVector3f m_position = {0.0f, 0.0f, 0.0f};
    XrQuaternionf m_rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    XrVector3f m_scale = {1.0f, 1.0f, 1.0f};
    XrMatrix4x4f m_modelMatrix;
    bool m_isDirty = true;

public:
    Transform();
    Transform(const XrVector3f& position);
    Transform(const XrVector3f& position, const XrQuaternionf& rotation);
    Transform(const XrVector3f& position, const XrQuaternionf& rotation, const XrVector3f& scale);
    
    void SetPosition(const XrVector3f& position);
    void SetRotation(const XrQuaternionf& rotation);
    void SetScale(const XrVector3f& scale);
    
    const XrVector3f& GetPosition() const { return m_position; }
    const XrQuaternionf& GetRotation() const { return m_rotation; }
    const XrVector3f& GetScale() const { return m_scale; }
    const XrMatrix4x4f& GetModelMatrix();
    
    void Translate(const XrVector3f& translation);
    void Rotate(const XrQuaternionf& rotation);

private:
    void UpdateModelMatrix();
};
