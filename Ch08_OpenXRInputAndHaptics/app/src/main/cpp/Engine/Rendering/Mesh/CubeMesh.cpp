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

#define CUBE_FACE(V1, V2, V3, V4, V5, V6) cubeCorners[V1], cubeCorners[V2], cubeCorners[V3], cubeCorners[V4], cubeCorners[V5], cubeCorners[V6],

    XrVector4f cubeVerticesArray[] = {
            CUBE_FACE(2, 1, 0, 2, 3, 1)
            CUBE_FACE(6, 4, 5, 6, 5, 7)
            CUBE_FACE(0, 1, 5, 0, 5, 4)
            CUBE_FACE(2, 6, 7, 2, 7, 3)
            CUBE_FACE(0, 4, 6, 0, 6, 2)
            CUBE_FACE(1, 3, 7, 1, 7, 5)
        };

    m_vertices.assign(std::begin(cubeVerticesArray), std::end(cubeVerticesArray));

    m_indices = {
            0, 1, 2, 3, 4, 5,
            6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29,
            30, 31, 32, 33, 34, 35
        };

    m_normals = {
            {1.00f, 0.00f, 0.00f, 0},
            {-1.00f, 0.00f, 0.00f, 0},
            {0.00f, 1.00f, 0.00f, 0},
            {0.00f, -1.00f, 0.00f, 0},
            {0.00f, 0.00f, 1.00f, 0},
            {0.00f, 0.0f, -1.00f, 0}
        };
}