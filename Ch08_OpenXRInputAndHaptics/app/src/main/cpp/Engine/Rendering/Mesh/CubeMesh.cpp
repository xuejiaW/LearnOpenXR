#include "CubeMesh.h"

CubeMesh::CubeMesh(float size)
{
    GenerateCubeData(size);
}

void CubeMesh::GenerateCubeData(float size)
{
    float halfSize = size * 0.5f;

    // Define the 8 vertices of a cube (same approach as CubeGeometry)
    XrVector4f cubeCorners[] = {
            {+halfSize, +halfSize, +halfSize, 1.0f},  // 0: front-top-right
            {+halfSize, +halfSize, -halfSize, 1.0f},  // 1: back-top-right
            {+halfSize, -halfSize, +halfSize, 1.0f},  // 2: front-bottom-right
            {+halfSize, -halfSize, -halfSize, 1.0f},  // 3: back-bottom-right
            {-halfSize, +halfSize, +halfSize, 1.0f},  // 4: front-top-left
            {-halfSize, +halfSize, -halfSize, 1.0f},  // 5: back-top-left
            {-halfSize, -halfSize, +halfSize, 1.0f},  // 6: front-bottom-left
            {-halfSize, -halfSize, -halfSize, 1.0f}   // 7: back-bottom-left
        };

#define CUBE_FACE(V1, V2, V3, V4, V5, V6) cubeCorners[V1], cubeCorners[V2], cubeCorners[V3], cubeCorners[V4], cubeCorners[V5], cubeCorners[V6],

    XrVector4f cubeVerticesArray[] = {
            CUBE_FACE(2, 1, 0, 2, 3, 1)  // Right face  (+X) - exactly like CubeGeometry  
            CUBE_FACE(6, 4, 5, 6, 5, 7)  // Left face   (-X) - exactly like CubeGeometry
            CUBE_FACE(0, 1, 5, 0, 5, 4)  // Top face    (+Y) - exactly like CubeGeometry
            CUBE_FACE(2, 6, 7, 2, 7, 3)  // Bottom face (-Y) - exactly like CubeGeometry
            CUBE_FACE(0, 4, 6, 0, 6, 2)  // Front face  (+Z) - exactly like CubeGeometry
            CUBE_FACE(1, 3, 7, 1, 7, 5)  // Back face   (-Z) - exactly like CubeGeometry
        };

    m_vertices.assign(std::begin(cubeVerticesArray), std::end(cubeVerticesArray));

    // Sequential indices for the 36 vertices
    m_indices = {
            0, 1, 2, 3, 4, 5,        // Right face triangles
            6, 7, 8, 9, 10, 11,      // Left face triangles
            12, 13, 14, 15, 16, 17,  // Top face triangles
            18, 19, 20, 21, 22, 23,  // Bottom face triangles
            24, 25, 26, 27, 28, 29,  // Front face triangles
            30, 31, 32, 33, 34, 35   // Back face triangles
        };

    // Face normals for lighting calculations (matching CubeGeometry approach)
    m_normals = {
            {1.00f, 0.00f, 0.00f, 0},   // Right face  (+X)
            {-1.00f, 0.00f, 0.00f, 0},  // Left face   (-X)
            {0.00f, 1.00f, 0.00f, 0},   // Top face    (+Y)
            {0.00f, -1.00f, 0.00f, 0},  // Bottom face (-Y)
            {0.00f, 0.00f, 1.00f, 0},   // Front face  (+Z)
            {0.00f, 0.0f, -1.00f, 0}    // Back face   (-Z)
        };
}