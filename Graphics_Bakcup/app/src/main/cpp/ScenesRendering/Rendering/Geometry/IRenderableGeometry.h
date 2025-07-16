#pragma once

#include <cstdint>
#include <vector>
#include <openxr/openxr.h>

class IRenderableGeometry {
public:
    virtual ~IRenderableGeometry() = default;
    
    virtual void* GetVertexBuffer() const = 0;
    virtual void* GetIndexBuffer() const = 0;
    virtual uint32_t GetIndexCount() const = 0;
    
    // Get additional geometry-specific data for rendering
    virtual const std::vector<XrVector4f>& GetNormals() const = 0;
    virtual void* GetNormalsBuffer() const = 0;
};
