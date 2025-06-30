#pragma once
#include "GraphicsAPI.h"

class OpenXRGraphicsAPI
{
public:
    OpenXRGraphicsAPI() = default;
    static const char* GetGraphicsAPIInstanceExtensionString(GraphicsAPI_Type type);
};