#pragma once

#include "IRenderableGeometry.h"
#include "GraphicsAPI.h"

class CubeGeometry : public IRenderableGeometry {
public:
    CubeGeometry();
    ~CubeGeometry();
    
    void* GetVertexBuffer() const override { return m_vertexBuffer; }
    void* GetIndexBuffer() const override { return m_indexBuffer; }
    uint32_t GetIndexCount() const override { return 36; }
    
    const std::vector<XrVector4f>& GetNormals() const override { return m_normals; }
    void* GetNormalsBuffer() const override { return m_normalsBuffer; }
    
private:
    void* m_vertexBuffer = nullptr;
    void* m_indexBuffer = nullptr;
    void* m_normalsBuffer = nullptr;
    
    std::vector<XrVector4f> m_normals;
};
