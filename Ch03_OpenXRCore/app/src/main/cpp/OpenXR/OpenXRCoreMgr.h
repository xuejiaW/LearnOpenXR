#pragma once

#include <openxr/openxr.h>

class OpenXRCoreMgr
{
public:
    static void CreateInstance();
    static void DestroyInstance();

    static XrInstance m_xrInstance;
};