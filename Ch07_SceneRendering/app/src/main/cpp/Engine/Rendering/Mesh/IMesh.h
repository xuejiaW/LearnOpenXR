#pragma once

#include <vector>
#include <openxr/openxr.h>
#include "../Vertex.h"

class IMesh {
public:
    virtual ~IMesh() = default;
    
    virtual const std::vector<Vertex>& GetVerticesWithNormals() const = 0;
    virtual const std::vector<uint32_t>& GetIndices() const = 0;
    
    virtual uint32_t GetVertexCount() const = 0;
    virtual uint32_t GetIndexCount() const = 0;
};
