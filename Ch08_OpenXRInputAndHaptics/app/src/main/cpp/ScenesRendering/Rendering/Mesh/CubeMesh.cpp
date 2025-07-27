#include "CubeMesh.h"

CubeMesh::CubeMesh(float size) {
    GenerateCubeData(size);
}

void CubeMesh::GenerateCubeData(float size) {
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

    // Create vertices manually for better control (6 faces * 6 vertices per face = 36 vertices)
    std::vector<XrVector4f> cubeVertices;
    
    // Right face (+X) - all vertices have X = +halfSize
    // Triangle 1: (0,2,3), Triangle 2: (0,3,1) - proper quad triangulation
    cubeVertices.insert(cubeVertices.end(), {
        cubeCorners[0], cubeCorners[2], cubeCorners[3],  // Triangle 1: top-front, bottom-front, bottom-back
        cubeCorners[0], cubeCorners[3], cubeCorners[1]   // Triangle 2: top-front, bottom-back, top-back
    });
    
    // Left face (-X) - all vertices have X = -halfSize  
    // Triangle 1: (4,5,7), Triangle 2: (4,7,6) - proper quad triangulation
    cubeVertices.insert(cubeVertices.end(), {
        cubeCorners[4], cubeCorners[5], cubeCorners[7],  // Triangle 1: top-front, top-back, bottom-back
        cubeCorners[4], cubeCorners[7], cubeCorners[6]   // Triangle 2: top-front, bottom-back, bottom-front
    });
    
    // Top face (+Y) - all vertices have Y = +halfSize
    // Triangle 1: (0,1,5), Triangle 2: (0,5,4) - proper quad triangulation
    cubeVertices.insert(cubeVertices.end(), {
        cubeCorners[0], cubeCorners[1], cubeCorners[5],  // Triangle 1: front-right, back-right, back-left
        cubeCorners[0], cubeCorners[5], cubeCorners[4]   // Triangle 2: front-right, back-left, front-left
    });
    
    // Bottom face (-Y) - all vertices have Y = -halfSize
    // Triangle 1: (2,6,7), Triangle 2: (2,7,3) - proper quad triangulation
    cubeVertices.insert(cubeVertices.end(), {
        cubeCorners[2], cubeCorners[6], cubeCorners[7],  // Triangle 1: front-right, front-left, back-left
        cubeCorners[2], cubeCorners[7], cubeCorners[3]   // Triangle 2: front-right, back-left, back-right
    });
    
    // Front face (+Z) - all vertices have Z = +halfSize
    // Triangle 1: (0,4,6), Triangle 2: (0,6,2) - proper quad triangulation
    cubeVertices.insert(cubeVertices.end(), {
        cubeCorners[0], cubeCorners[4], cubeCorners[6],  // Triangle 1: top-right, top-left, bottom-left
        cubeCorners[0], cubeCorners[6], cubeCorners[2]   // Triangle 2: top-right, bottom-left, bottom-right
    });
    
    // Back face (-Z) - all vertices have Z = -halfSize
    // Triangle 1: (1,3,7), Triangle 2: (1,7,5) - proper quad triangulation
    cubeVertices.insert(cubeVertices.end(), {
        cubeCorners[1], cubeCorners[3], cubeCorners[7],  // Triangle 1: top-right, bottom-right, bottom-left
        cubeCorners[1], cubeCorners[7], cubeCorners[5]   // Triangle 2: top-right, bottom-left, top-left
    });

    // Assign to member variable
    m_vertices = cubeVertices;

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
