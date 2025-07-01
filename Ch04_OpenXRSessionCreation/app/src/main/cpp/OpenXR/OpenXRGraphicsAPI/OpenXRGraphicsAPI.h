#pragma once
#include "GraphicsAPI.h"

class OpenXRGraphicsAPI
{
public:
    virtual ~OpenXRGraphicsAPI() = default;
    OpenXRGraphicsAPI() = default;

    virtual void* GetGraphicsBinding() = 0;
    
    static const char* GetGraphicsAPIInstanceExtensionString(GraphicsAPI_Type type);
};