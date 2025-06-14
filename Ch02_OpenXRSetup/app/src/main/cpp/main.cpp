#define XR_DOCS_CHAPTER_VERSION XR_DOCS_CHAPTER_1_4

#include <DebugOutput.h>
#include "OpenXRTutorial/OpenXRTutorial.h"

void OpenXRTutorial_Main(GraphicsAPI_Type apiType)
{
    DebugOutput debugOutput;
    XR_TUT_LOG("OpenXR Tutorial Ch02_OpenXRSetup");

    OpenXRTutorial app(apiType);
    app.Run();
}
