# Copyright 2023, The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0

# XR_DOCS_TAG_BEGIN_SetProjectName3
cmake_minimum_required(VERSION 3.22.1)
set(PROJECT_NAME Ch03_OpenXRCore)
project("${PROJECT_NAME}")
# XR_DOCS_TAG_END_SetProjectName3

# XR_DOCS_TAG_BEGIN_CMakeModulePath
# Additional Directories for find_package() to search within.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/glsl_shader.cmake")

# XR_DOCS_TAG_BEGIN_FetchContent
# For FetchContent_Declare() and FetchContent_MakeAvailable()
include(FetchContent)

# openxr_loader - From github.com/KhronosGroup
set(BUILD_TESTS
    OFF
    CACHE INTERNAL "Build tests"
)
set(BUILD_API_LAYERS
    ON
    CACHE INTERNAL "Use OpenXR layers"
)
FetchContent_Declare(
    OpenXR
    URL_HASH MD5=924a94a2da0b5ef8e82154c623d88644
    URL  "${CMAKE_CURRENT_SOURCE_DIR}/../external/release-1.0.34.zip"
        SOURCE_DIR
        openxr
)
FetchContent_MakeAvailable(OpenXR)
# XR_DOCS_TAG_END_FetchContent

# Files
set(SOURCES
    ../Common/GraphicsAPI.cpp
    ../Common/GraphicsAPI_Vulkan.cpp
    ../Common/OpenXRDebugUtils.cpp
    app/src/main/cpp/main.cpp
    app/src/main/cpp/Application/OpenXRTutorial_Android.cpp
    app/src/main/cpp/Application/OpenXRTutorial.cpp
    app/src/main/cpp/Application/OpenXRTutorial_Windows.cpp
)
set(HEADERS
    ../Common/DebugOutput.h
    ../Common/GraphicsAPI.h
    ../Common/GraphicsAPI_Vulkan.h
    ../Common/HelperFunctions.h
    ../Common/OpenXRDebugUtils.h
    ../Common/OpenXRHelper.h
    app/src/main/cpp/Application/OpenXRTutorial.h
)

# XR_DOCS_TAG_BEGIN_GLSLShaders
set(GLSL_SHADERS "../Shaders/VertexShader.glsl" "../Shaders/PixelShader.glsl")
# XR_DOCS_TAG_END_GLSLShaders

if(ANDROID) # Android
    # XR_DOCS_TAG_BEGIN_Android
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
            # In this repo
            ../Common/
            # From OpenXR repo
            "${openxr_SOURCE_DIR}/src/common"
            "${openxr_SOURCE_DIR}/external/include"
    )

    # export ANativeActivity_onCreate for java to call.
    set_property(
        TARGET ${PROJECT_NAME}
        APPEND_STRING
        PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate"
    )

    # native_app_glue
    include(AndroidNdkModules)
    android_ndk_import_module_native_app_glue()

    target_link_libraries(${PROJECT_NAME} android native_app_glue openxr_loader)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-cast-calling-convention)
    # XR_DOCS_TAG_END_Android
    
    # Force Vulkan Graphics API for Android
    target_compile_definitions(${PROJECT_NAME} PUBLIC XR_TUTORIAL_USE_VULKAN)

    # XR_DOCS_TAG_BEGIN_VulkanNDK
    # VulkanNDK
    find_library(vulkan-lib vulkan)
    if(vulkan-lib)
        target_include_directories(
            ${PROJECT_NAME}
            PUBLIC ${ANDROID_NDK}/sources/third_party/vulkan/src/include
        )
        target_link_libraries(${PROJECT_NAME} ${vulkan-lib})
        target_compile_definitions(
            ${PROJECT_NAME} PUBLIC XR_TUTORIAL_USE_VULKAN
        )
    endif()
    # XR_DOCS_TAG_END_VulkanNDK

    # XR_DOCS_TAG_BEGIN_CompileAndroidGLSLShaders
    # Vulkan GLSL
    set(SHADER_DEST "${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/assets/shaders")
    include(../cmake/glsl_shader.cmake)
    set_source_files_properties(
        ../Shaders/VertexShader.glsl PROPERTIES ShaderType "vert"
    )
    set_source_files_properties(
        ../Shaders/PixelShader.glsl PROPERTIES ShaderType "frag"
    )

    foreach(FILE ${GLSL_SHADERS})
        get_filename_component(FILE_WE ${FILE} NAME_WE)
        get_source_file_property(shadertype ${FILE} ShaderType)
        glsl_spv_shader(
            INPUT
            "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}"
            OUTPUT
            "${SHADER_DEST}/${FILE_WE}.spv"
            STAGE
            ${shadertype}
            ENTRY_POINT
            main
            TARGET_ENV
            vulkan1.0
        )
        # Make our project depend on these files
        target_sources(${PROJECT_NAME} PRIVATE "${SHADER_DEST}/${FILE_WE}.spv")
    endforeach()
    # XR_DOCS_TAG_END_CompileAndroidGLSLShaders

else() # Windows / Linux
    # XR_DOCS_TAG_BEGIN_WindowsLinux
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

    if(NOT "${XR_RUNTIME_JSON}" STREQUAL "")
        set_target_properties(
            ${PROJECT_NAME} PROPERTIES VS_DEBUGGER_ENVIRONMENT
                                       "XR_RUNTIME_JSON=${XR_RUNTIME_JSON}"
        )
    endif()

    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
            # In this repo
            ../Common/
            # From OpenXR repo
            "${openxr_SOURCE_DIR}/src/common"
            "${openxr_SOURCE_DIR}/external/include"
    )
    target_link_libraries(${PROJECT_NAME} openxr_loader)
    # XR_DOCS_TAG_END_WindowsLinux
    
    # Force Vulkan Graphics API for Windows/Linux
    target_compile_definitions(${PROJECT_NAME} PUBLIC XR_TUTORIAL_USE_VULKAN)

    if(WIN32) # Windows
    else() # Linux
        # XR_DOCS_TAG_BEGIN_Linux
        target_compile_definitions(
            ${PROJECT_NAME} PUBLIC XR_TUTORIAL_USE_LINUX_XLIB
        )
        # XR_DOCS_TAG_END_Linux
    endif()

    # XR_DOCS_TAG_BEGIN_VulkanSDK
    # VulkanSDK
    find_package(Vulkan)
    if(Vulkan_FOUND)
        target_include_directories(
            ${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS}
        )
        target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
        target_compile_definitions(
            ${PROJECT_NAME} PUBLIC XR_TUTORIAL_USE_VULKAN
        )
    endif()
    # XR_DOCS_TAG_END_VulkanSDK

    # XR_DOCS_TAG_BEGIN_BuildShadersVulkanWindowsLinux
    # Vulkan GLSL
    set(SHADER_DEST "${CMAKE_CURRENT_BINARY_DIR}")
    if(Vulkan_FOUND)
        include(../cmake/glsl_shader.cmake)
        set_source_files_properties(
            ../Shaders/VertexShader.glsl PROPERTIES ShaderType "vert"
        )
        set_source_files_properties(
            ../Shaders/PixelShader.glsl PROPERTIES ShaderType "frag"
        )

        foreach(FILE ${GLSL_SHADERS})
            get_filename_component(FILE_WE ${FILE} NAME_WE)
            get_source_file_property(shadertype ${FILE} ShaderType)
            glsl_spv_shader(
                INPUT
                "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}"
                OUTPUT
                "${SHADER_DEST}/${FILE_WE}.spv"
                STAGE
                ${shadertype}
                ENTRY_POINT
                main
                TARGET_ENV
                vulkan1.0
            )
            # Make our project depend on these files
            target_sources(
                ${PROJECT_NAME} PRIVATE "${SHADER_DEST}/${FILE_WE}.spv"
            )
        endforeach()
    endif()
    # XR_DOCS_TAG_END_BuildShadersVulkanWindowsLinux
endif() # EOF




