# Simple NDK C++ Template
cmake_minimum_required(VERSION 3.22.1)
project(NativeActivity LANGUAGES C CXX)

# Import native_app_glue for NativeActivity support
include(AndroidNdkModules)
android_ndk_import_module_native_app_glue()

# Files
set(SOURCES
    app/src/main/cpp/main.cpp
)

# Create shared library
add_library(native-activity SHARED ${SOURCES})

# Set C++ standard
target_compile_features(native-activity PUBLIC cxx_std_17)

# Link libraries
target_link_libraries(native-activity
    android
    native_app_glue
    log
)

# Export ANativeActivity_onCreate symbol
set_property(
    TARGET native-activity
    APPEND_STRING
    PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate"
)
