#include "CubeMesh.h"

CubeMesh::CubeMesh(float size)
{
    GenerateCubeData(size);
}

void CubeMesh::GenerateCubeData(float size)
{
    float halfSize = size * 0.5f;

    XrVector4f cubeCorners[] = {
            {+halfSize, +halfSize, +halfSize, 1.0f},
            {+halfSize, +halfSize, -halfSize, 1.0f},
            {+halfSize, -halfSize, +halfSize, 1.0f},
            {+halfSize, -halfSize, -halfSize, 1.0f},
            {-halfSize, +halfSize, +halfSize, 1.0f},
            {-halfSize, +halfSize, -halfSize, 1.0f},
            {-halfSize, -halfSize, +halfSize, 1.0f},
            {-halfSize, -halfSize, -halfSize, 1.0f}
        };

    XrVector3f faceNormals[6] = {
        {1.00f, 0.00f, 0.00f},   // Right face  (+X)
        {-1.00f, 0.00f, 0.00f},  // Left face   (-X)
        {0.00f, 1.00f, 0.00f},   // Top face    (+Y)
        {0.00f, -1.00f, 0.00f},  // Bottom face (-Y)
        {0.00f, 0.00f, 1.00f},   // Front face  (+Z)
        {0.00f, 0.00f, -1.00f}   // Back face   (-Z)
    };

#define CUBE_FACE(V1, V2, V3, V4, V5, V6, NORMAL_IDX) \
    Vertex(cubeCorners[V1], faceNormals[NORMAL_IDX]), \
    Vertex(cubeCorners[V2], faceNormals[NORMAL_IDX]), \
    Vertex(cubeCorners[V3], faceNormals[NORMAL_IDX]), \
    Vertex(cubeCorners[V4], faceNormals[NORMAL_IDX]), \
    Vertex(cubeCorners[V5], faceNormals[NORMAL_IDX]), \
    Vertex(cubeCorners[V6], faceNormals[NORMAL_IDX]),

    Vertex cubeVerticesWithNormals[] = {
            CUBE_FACE(2, 1, 0, 2, 3, 1, 0)  // Right face  (+X)
            CUBE_FACE(6, 4, 5, 6, 5, 7, 1)  // Left face   (-X)
            CUBE_FACE(0, 1, 5, 0, 5, 4, 2)  // Top face    (+Y)
            CUBE_FACE(2, 6, 7, 2, 7, 3, 3)  // Bottom face (-Y)
            CUBE_FACE(0, 4, 6, 0, 6, 2, 4)  // Front face  (+Z)
            CUBE_FACE(1, 3, 7, 1, 7, 5, 5)  // Back face   (-Z)
        };

    m_verticesWithNormals.assign(std::begin(cubeVerticesWithNormals), std::end(cubeVerticesWithNormals));

    m_indices = {
            0, 1, 2, 3, 4, 5,
            6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29,
            30, 31, 32, 33, 34, 35
        };

#undef CUBE_FACE
}