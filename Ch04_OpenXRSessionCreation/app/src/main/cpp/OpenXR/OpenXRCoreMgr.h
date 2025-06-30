#pragma once

#include <openxr/openxr.h>

class OpenXRCoreMgr
{
public:
    static void CreateInstance();
    static void DestroyInstance();

    static void GetSystemID();
    static XrSystemId systemID;

    static XrInstance m_xrInstance;
};