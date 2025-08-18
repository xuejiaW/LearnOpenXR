#pragma once

#include <openxr/openxr.h>

struct Vertex {
    XrVector4f position;
    XrVector3f normal;
    
    Vertex() = default;
    
    Vertex(const XrVector4f& pos, const XrVector3f& norm) 
        : position(pos), normal(norm) {}
    
    Vertex(float x, float y, float z, float nx, float ny, float nz)
        : position({x, y, z, 1.0f}), normal({nx, ny, nz}) {}
};
