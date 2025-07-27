#include "Transform.h"
#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>

Transform::Transform() {
    UpdateModelMatrix();
}

Transform::Transform(const XrVector3f& position) : m_position(position) {
    UpdateModelMatrix();
}

Transform::Transform(const XrVector3f& position, const XrQuaternionf& rotation) 
    : m_position(position), m_rotation(rotation) {
    UpdateModelMatrix();
}

Transform::Transform(const XrVector3f& position, const XrQuaternionf& rotation, const XrVector3f& scale)
    : m_position(position), m_rotation(rotation), m_scale(scale) {
    UpdateModelMatrix();
}

void Transform::SetPosition(const XrVector3f& position) {
    m_position = position;
    m_isDirty = true;
}

void Transform::SetRotation(const XrQuaternionf& rotation) {
    m_rotation = rotation;
    m_isDirty = true;
}

void Transform::SetScale(const XrVector3f& scale) {
    m_scale = scale;
    m_isDirty = true;
}

const XrMatrix4x4f& Transform::GetModelMatrix() {
    if (m_isDirty) {
        UpdateModelMatrix();
        m_isDirty = false;
    }
    return m_modelMatrix;
}

void Transform::Translate(const XrVector3f& translation) {
    m_position.x += translation.x;
    m_position.y += translation.y;
    m_position.z += translation.z;
    m_isDirty = true;
}

void Transform::Rotate(const XrQuaternionf& rotation) {
    // Multiply current rotation by new rotation
    XrQuaternionf result;
    result.w = m_rotation.w * rotation.w - m_rotation.x * rotation.x - m_rotation.y * rotation.y - m_rotation.z * rotation.z;
    result.x = m_rotation.w * rotation.x + m_rotation.x * rotation.w + m_rotation.y * rotation.z - m_rotation.z * rotation.y;
    result.y = m_rotation.w * rotation.y - m_rotation.x * rotation.z + m_rotation.y * rotation.w + m_rotation.z * rotation.x;
    result.z = m_rotation.w * rotation.z + m_rotation.x * rotation.y - m_rotation.y * rotation.x + m_rotation.z * rotation.w;
    m_rotation = result;
    m_isDirty = true;
}

void Transform::UpdateModelMatrix() {
    // Create scale matrix
    XrMatrix4x4f scaleMatrix;
    XrMatrix4x4f_CreateScale(&scaleMatrix, m_scale.x, m_scale.y, m_scale.z);
    
    // Create rotation matrix
    XrMatrix4x4f rotationMatrix;
    XrMatrix4x4f_CreateFromQuaternion(&rotationMatrix, &m_rotation);
    
    // Create translation matrix
    XrMatrix4x4f translationMatrix;
    XrMatrix4x4f_CreateTranslation(&translationMatrix, m_position.x, m_position.y, m_position.z);
    
    // Combine: Translation * Rotation * Scale
    XrMatrix4x4f temp;
    XrMatrix4x4f_Multiply(&temp, &rotationMatrix, &scaleMatrix);
    XrMatrix4x4f_Multiply(&m_modelMatrix, &translationMatrix, &temp);
}
