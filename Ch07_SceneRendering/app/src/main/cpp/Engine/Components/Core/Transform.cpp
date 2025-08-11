#include "Transform.h"
#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>

Transform::Transform() {
    UpdateModelMatrix();
    UpdateViewMatrix();
}

Transform::Transform(const XrVector3f& position) : m_Position(position) {
    UpdateModelMatrix();
    UpdateViewMatrix();
}

Transform::Transform(const XrVector3f& position, const XrQuaternionf& rotation) 
    : m_Position(position), m_Rotation(rotation) {
    UpdateModelMatrix();
    UpdateViewMatrix();
}

Transform::Transform(const XrVector3f& position, const XrQuaternionf& rotation, const XrVector3f& scale)
    : m_Position(position), m_Rotation(rotation), m_Scale(scale) {
    UpdateModelMatrix();
    UpdateViewMatrix();
}

void Transform::SetPosition(const XrVector3f& position) {
    m_Position = position;
    MarkMatricesDirty();
}

void Transform::SetRotation(const XrQuaternionf& rotation) {
    m_Rotation = rotation;
    MarkMatricesDirty();
}

void Transform::SetScale(const XrVector3f& scale) {
    m_Scale = scale;
    m_ModelMatrixDirty = true;
}

const XrMatrix4x4f& Transform::GetModelMatrix() {
    if (m_ModelMatrixDirty) {
        UpdateModelMatrix();
        m_ModelMatrixDirty = false;
    }
    return m_ModelMatrix;
}

const XrMatrix4x4f& Transform::GetViewMatrix() {
    if (m_ViewMatrixDirty) {
        UpdateViewMatrix();
        m_ViewMatrixDirty = false;
    }
    return m_ViewMatrix;
}

void Transform::Translate(const XrVector3f& translation) {
    m_Position.x += translation.x;
    m_Position.y += translation.y;
    m_Position.z += translation.z;
    MarkMatricesDirty();
}

void Transform::Rotate(const XrQuaternionf& rotation) {
    XrQuaternionf result;
    result.w = m_Rotation.w * rotation.w - m_Rotation.x * rotation.x - m_Rotation.y * rotation.y - m_Rotation.z * rotation.z;
    result.x = m_Rotation.w * rotation.x + m_Rotation.x * rotation.w + m_Rotation.y * rotation.z - m_Rotation.z * rotation.y;
    result.y = m_Rotation.w * rotation.y - m_Rotation.x * rotation.z + m_Rotation.y * rotation.w + m_Rotation.z * rotation.x;
    result.z = m_Rotation.w * rotation.z + m_Rotation.x * rotation.y - m_Rotation.y * rotation.x + m_Rotation.z * rotation.w;
    m_Rotation = result;
    MarkMatricesDirty();
}

void Transform::UpdateModelMatrix() {
    XrMatrix4x4f scaleMatrix;
    XrMatrix4x4f_CreateScale(&scaleMatrix, m_Scale.x, m_Scale.y, m_Scale.z);
    
    XrMatrix4x4f rotationMatrix;
    XrMatrix4x4f_CreateFromQuaternion(&rotationMatrix, &m_Rotation);
    
    XrMatrix4x4f translationMatrix;
    XrMatrix4x4f_CreateTranslation(&translationMatrix, m_Position.x, m_Position.y, m_Position.z);
    
    XrMatrix4x4f temp;
    XrMatrix4x4f_Multiply(&temp, &rotationMatrix, &scaleMatrix);
    XrMatrix4x4f_Multiply(&m_ModelMatrix, &translationMatrix, &temp);
}

void Transform::UpdateViewMatrix() {
    XrMatrix4x4f rotationMatrix, translationMatrix;
    
    XrMatrix4x4f_CreateFromQuaternion(&rotationMatrix, &m_Rotation);
    
    XrMatrix4x4f_CreateTranslation(&translationMatrix, 
        -m_Position.x, -m_Position.y, -m_Position.z);
    
    XrMatrix4x4f rotationInverse;
    XrMatrix4x4f_Transpose(&rotationInverse, &rotationMatrix);
    
    XrMatrix4x4f_Multiply(&m_ViewMatrix, &rotationInverse, &translationMatrix);
}

void Transform::MarkMatricesDirty() {
    m_ModelMatrixDirty = true;
    m_ViewMatrixDirty = true;
}
