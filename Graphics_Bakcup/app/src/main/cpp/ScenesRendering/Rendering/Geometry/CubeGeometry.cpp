#include "CubeGeometry.h"
#include "../../../OpenXR/OpenXRCoreMgr.h"
#include <openxr/openxr.h>
#include <GraphicsAPI.h>

CubeGeometry::CubeGeometry() {
    // Define the 8 vertices of a unit cube
    constexpr XrVector4f cubeCorners[] = {
        {+0.5f, +0.5f, +0.5f, 1.0f},  // 0: front-top-right
        {+0.5f, +0.5f, -0.5f, 1.0f},  // 1: back-top-right
        {+0.5f, -0.5f, +0.5f, 1.0f},  // 2: front-bottom-right
        {+0.5f, -0.5f, -0.5f, 1.0f},  // 3: back-bottom-right
        {-0.5f, +0.5f, +0.5f, 1.0f},  // 4: front-top-left
        {-0.5f, +0.5f, -0.5f, 1.0f},  // 5: back-top-left
        {-0.5f, -0.5f, +0.5f, 1.0f},  // 6: front-bottom-left
        {-0.5f, -0.5f, -0.5f, 1.0f}   // 7: back-bottom-left
    };

#define CUBE_FACE(V1, V2, V3, V4, V5, V6) cubeCorners[V1], cubeCorners[V2], cubeCorners[V3], cubeCorners[V4], cubeCorners[V5], cubeCorners[V6],

    // Create vertex buffer with 6 faces * 6 vertices per face = 36 vertices
    XrVector4f cubeVertices[] = {
        CUBE_FACE(2, 1, 0, 2, 3, 1)  // Right face  (+X)
        CUBE_FACE(6, 4, 5, 6, 5, 7)  // Left face   (-X)
        CUBE_FACE(0, 1, 5, 0, 5, 4)  // Top face    (+Y)
        CUBE_FACE(2, 6, 7, 2, 7, 3)  // Bottom face (-Y)
        CUBE_FACE(0, 4, 6, 0, 6, 2)  // Front face  (+Z)
        CUBE_FACE(1, 3, 7, 1, 7, 5)  // Back face   (-Z)
    };

    // Sequential indices for the 36 vertices
    uint32_t triangleIndices[36] = {
        0, 1, 2, 3, 4, 5,        // Right face triangles
        6, 7, 8, 9, 10, 11,      // Left face triangles
        12, 13, 14, 15, 16, 17,  // Top face triangles
        18, 19, 20, 21, 22, 23,  // Bottom face triangles
        24, 25, 26, 27, 28, 29,  // Front face triangles
        30, 31, 32, 33, 34, 35   // Back face triangles
    };

    GraphicsAPI::BufferCreateInfo vertexBufferInfo;
    vertexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::VERTEX;
    vertexBufferInfo.stride = sizeof(float) * 4;
    vertexBufferInfo.size = sizeof(cubeVertices);
    vertexBufferInfo.data = &cubeVertices;
    m_vertexBuffer = OpenXRCoreMgr::GetGraphicsAPI()->CreateBuffer(vertexBufferInfo);

    GraphicsAPI::BufferCreateInfo indexBufferInfo;
    indexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::INDEX;
    indexBufferInfo.stride = sizeof(uint32_t);
    indexBufferInfo.size = sizeof(triangleIndices);
    indexBufferInfo.data = &triangleIndices;
    m_indexBuffer = OpenXRCoreMgr::GetGraphicsAPI()->CreateBuffer(indexBufferInfo);
    
    // Initialize face normals for lighting calculations
    m_normals = {
        {1.00f, 0.00f, 0.00f, 0},   // Right face  (+X)
        {-1.00f, 0.00f, 0.00f, 0},  // Left face   (-X)
        {0.00f, 1.00f, 0.00f, 0},   // Top face    (+Y)
        {0.00f, -1.00f, 0.00f, 0},  // Bottom face (-Y)
        {0.00f, 0.00f, 1.00f, 0},   // Front face  (+Z)
        {0.00f, 0.0f, -1.00f, 0}    // Back face   (-Z)
    };
    
    // Create normals buffer (consistent with vertex/index buffer creation)
    GraphicsAPI::BufferCreateInfo faceNormalsBufferInfo;
    faceNormalsBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::UNIFORM;
    faceNormalsBufferInfo.stride = 0;
    faceNormalsBufferInfo.size = sizeof(XrVector4f) * m_normals.size();
    faceNormalsBufferInfo.data = m_normals.data();
    m_normalsBuffer = OpenXRCoreMgr::GetGraphicsAPI()->CreateBuffer(faceNormalsBufferInfo);
}

CubeGeometry::~CubeGeometry() {
    if (m_vertexBuffer) {
        OpenXRCoreMgr::GetGraphicsAPI()->DestroyBuffer(m_vertexBuffer);
    }
    if (m_indexBuffer) {
        OpenXRCoreMgr::GetGraphicsAPI()->DestroyBuffer(m_indexBuffer);
    }
    if (m_normalsBuffer) {
        OpenXRCoreMgr::GetGraphicsAPI()->DestroyBuffer(m_normalsBuffer);
    }
}
