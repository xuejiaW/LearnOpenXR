#pragma once
#include <openxr/openxr.h>

class OpenXRSpaceMgr
{
public:
    static void CreateReferenceSpace();
    static void DestroyReferenceSpace();
    static XrSpace activeSpaces;
};