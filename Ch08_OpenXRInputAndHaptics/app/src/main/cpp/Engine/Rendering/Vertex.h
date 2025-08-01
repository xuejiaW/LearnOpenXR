#pragma once

#include <openxr/openxr.h>

struct Vertex {
    XrVector4f position;  // 位置 (x, y, z, w)
    XrVector3f normal;    // 法线 (nx, ny, nz)
    
    Vertex() = default;
    
    Vertex(const XrVector4f& pos, const XrVector3f& norm) 
        : position(pos), normal(norm) {}
    
    Vertex(float x, float y, float z, float nx, float ny, float nz)
        : position({x, y, z, 1.0f}), normal({nx, ny, nz}) {}
};
