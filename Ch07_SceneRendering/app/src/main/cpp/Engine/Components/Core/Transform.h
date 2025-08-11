#pragma once

#include "../../Core/IComponent.h"
#include <openxr/openxr.h>
#include <xr_linear_algebra.h>

class Transform : public IComponent {
public:
    Transform();
    Transform(const XrVector3f& position);
    Transform(const XrVector3f& position, const XrQuaternionf& rotation);
    Transform(const XrVector3f& position, const XrQuaternionf& rotation, const XrVector3f& scale);
    
    void SetPosition(const XrVector3f& position);
    void SetRotation(const XrQuaternionf& rotation);
    void SetScale(const XrVector3f& scale);
    
    const XrVector3f& GetPosition() const { return m_Position; }
    const XrQuaternionf& GetRotation() const { return m_Rotation; }
    const XrVector3f& GetScale() const { return m_Scale; }
    const XrMatrix4x4f& GetModelMatrix();
    const XrMatrix4x4f& GetViewMatrix();
    
    void Translate(const XrVector3f& translation);
    void Rotate(const XrQuaternionf& rotation);

private:
    void UpdateModelMatrix();
    void UpdateViewMatrix();
    void MarkMatricesDirty();
    
    XrVector3f m_Position = {0.0f, 0.0f, 0.0f};
    XrQuaternionf m_Rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    XrVector3f m_Scale = {1.0f, 1.0f, 1.0f};
    XrMatrix4x4f m_ModelMatrix;
    XrMatrix4x4f m_ViewMatrix;
    bool m_ModelMatrixDirty = true;
    bool m_ViewMatrixDirty = true;
};
