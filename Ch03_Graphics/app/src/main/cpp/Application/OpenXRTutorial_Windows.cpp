#include "OpenXRTutorial.h"

void OpenXRTutorial_Main(GraphicsAPI_Type apiType);

#if !defined(__ANDROID__) && defined(_WIN32)
void OpenXRTutorial::PollSystemEvents() {}

#endif