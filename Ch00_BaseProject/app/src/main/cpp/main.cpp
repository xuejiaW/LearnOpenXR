#include <android/log.h>
#include <android_native_app_glue.h>

#define LOG_TAG "NDK_Template"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void android_main(struct android_app* app) {
  LOGI("========================================");
  LOGI("NDK C++ Template Started!");
  LOGI("========================================");

  int events;
  struct android_poll_source* source;

  while (true) {
    while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
      if (source != nullptr) {
        source->process(app, source);
      }

      if (app->destroyRequested != 0) {
        LOGI("App destroy requested, exiting...");
        return;
      }
    }
  }
}