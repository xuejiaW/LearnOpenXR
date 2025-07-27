#pragma once

#include "IMesh.h"

class CubeMesh : public IMesh {
private:
    std::vector<XrVector4f> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<XrVector4f> m_normals;

public:
    CubeMesh(float size = 1.0f);
    
    const std::vector<XrVector4f>& GetVertices() const override { return m_vertices; }
    const std::vector<uint32_t>& GetIndices() const override { return m_indices; }
    const std::vector<XrVector4f>& GetNormals() const override { return m_normals; }
    uint32_t GetVertexCount() const override { return m_vertices.size(); }
    uint32_t GetIndexCount() const override { return m_indices.size(); }

private:
    void GenerateCubeData(float size);
};
