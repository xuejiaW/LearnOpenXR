#pragma once

#include <openxr/openxr.h>

#include <HelperFunctions.h>

inline void OpenXRDebugBreak() {
    std::cerr << "Breakpoint here to debug." << std::endl;
    DEBUG_BREAK;
}

inline const char* GetXRErrorString(XrResult result) {
    static char string[XR_MAX_RESULT_STRING_SIZE];
    // xrResultToString(OpenXRCoreMgr::m_xrInstance, result, string);
    return string;
}

#define OPENXR_CHECK(x, y)                                                                                                                                  \
    do {                                                                                                                                                    \
        XrResult result = (x);                                                                                                                              \
        if (!XR_SUCCEEDED(result)) {                                                                                                                        \
            std::cout << "ERROR: OPENXR: " << int(result) << "(" << (GetXRErrorString(result)) << ") " << y << std::endl; \
            OpenXRDebugBreak();                                                                                                                             \
        }                                                                                                                                                   \
    } while (0)
