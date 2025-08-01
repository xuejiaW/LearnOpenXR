#pragma once

#include "IMesh.h"

class CubeMesh : public IMesh {
private:
    std::vector<Vertex> m_verticesWithNormals;
    std::vector<uint32_t> m_indices;

public:
    CubeMesh(float size = 1.0f);
    
    const std::vector<Vertex>& GetVerticesWithNormals() const override { return m_verticesWithNormals; }
    const std::vector<uint32_t>& GetIndices() const override { return m_indices; }
    
    uint32_t GetVertexCount() const override { return m_verticesWithNormals.size(); }
    uint32_t GetIndexCount() const override { return m_indices.size(); }

private:
    void GenerateCubeData(float size);
};
