# Copyright 2023, The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0

# XR_DOCS_TAG_BEGIN_SetProjectName3
cmake_minimum_required(VERSION 3.22.1)
set(PROJECT_NAME Ch07_SceneRendering)
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
    ../Common/XRMathUtils.cpp
    ../Common/XRPathUtils.cpp
    app/src/main/cpp/main.cpp
    app/src/main/cpp/OpenXR/OpenXRDisplayMgr.cpp
    app/src/main/cpp/OpenXR/OpenXRCoreMgr.cpp
    app/src/main/cpp/OpenXR/OpenXRSessionMgr.cpp
    app/src/main/cpp/OpenXR/OpenXRSpaceMgr.cpp
    app/src/main/cpp/OpenXR/OpenXRRenderMgr.cpp
    app/src/main/cpp/OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.cpp
    app/src/main/cpp/OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI_Vulkan.cpp
    app/src/main/cpp/Application/OpenXRTutorial_Android.cpp
    app/src/main/cpp/Application/OpenXRTutorial.cpp
    app/src/main/cpp/Application/OpenXRTutorial_Windows.cpp
    app/src/main/cpp/Engine/Core/GameObject.cpp
    app/src/main/cpp/Engine/Components/Core/Transform.cpp
    app/src/main/cpp/Engine/Core/Scene.cpp
    app/src/main/cpp/Engine/Components/Rendering/Material.cpp
    app/src/main/cpp/Engine/Components/Rendering/MeshRenderer.cpp
    app/src/main/cpp/Engine/Components/Rendering/Camera.cpp
    app/src/main/cpp/Engine/Rendering/Mesh/CubeMesh.cpp
    app/src/main/cpp/Engine/Components/OpenXR/XRHmdDriver.cpp
    app/src/main/cpp/Scenes/TableFloorScene.cpp
)
set(HEADERS
    ../Common/DebugOutput.h
    ../Common/EnumToString.h
    ../Common/GraphicsAPI.h
    ../Common/GraphicsAPI_Vulkan.h
    ../Common/HelperFunctions.h
    ../Common/OpenXRDebugUtils.h
    ../Common/OpenXRHelper.h
    ../Common/XRMathUtils.h
    ../Common/XRPathUtils.h
    app/src/main/cpp/OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h
    app/src/main/cpp/OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI_Vulkan.h
    app/src/main/cpp/OpenXR/OpenXRDisplayMgr.h
    app/src/main/cpp/OpenXR/OpenXRDisplay/RenderLayerInfo.h
    app/src/main/cpp/OpenXR/OpenXRDisplay/SwapchainConfig.h
    app/src/main/cpp/OpenXR/OpenXRDisplay/SwapchainInfo.h
    app/src/main/cpp/OpenXR/OpenXRCoreMgr.h
    app/src/main/cpp/OpenXR/OpenXRSpaceMgr.h
    app/src/main/cpp/OpenXR/OpenXRSessionMgr.h
    app/src/main/cpp/OpenXR/OpenXRRenderMgr.h
    app/src/main/cpp/Application/OpenXRTutorial.h
    app/src/main/cpp/Engine/Core/IComponent.h
    app/src/main/cpp/Engine/Core/GameObject.h
    app/src/main/cpp/Engine/Components/Core/Transform.h
    app/src/main/cpp/Engine/Core/Scene.h
    app/src/main/cpp/Engine/Components/Rendering/Material.h
    app/src/main/cpp/Engine/Components/Rendering/MeshRenderer.h
    app/src/main/cpp/Engine/Components/Rendering/Camera.h
    app/src/main/cpp/Engine/Components/Rendering/RenderSettings.h
    app/src/main/cpp/Engine/Components/OpenXR/XRHmdDriver.h

    app/src/main/cpp/Engine/Rendering/Vertex.h
    app/src/main/cpp/Engine/Rendering/Mesh/IMesh.h
    app/src/main/cpp/Engine/Rendering/Mesh/CubeMesh.h
    app/src/main/cpp/Scenes/TableFloorScene.h
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

    # Ensure the shader destination directory exists
    file(MAKE_DIRECTORY ${SHADER_DEST})

    include(../cmake/glsl_shader.cmake)
    set_source_files_properties(
        ../Shaders/VertexShader.glsl PROPERTIES ShaderType "vert"
    )
    set_source_files_properties(
        ../Shaders/PixelShader.glsl PROPERTIES ShaderType "frag"
    )

    # Create a list to hold all shader targets
    set(SHADER_TARGETS)

    foreach(FILE ${GLSL_SHADERS})
        get_filename_component(FILE_WE ${FILE} NAME_WE)
        get_source_file_property(shadertype ${FILE} ShaderType)
        set(SHADER_SPV_FILE "${SHADER_DEST}/${FILE_WE}.spv")

        glsl_spv_shader(
            INPUT
            "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}"
            OUTPUT
            "${SHADER_SPV_FILE}"
            STAGE
            ${shadertype}
            ENTRY_POINT
            main
            TARGET_ENV
            vulkan1.0
        )

        # Create a custom target for each shader
        add_custom_target(${FILE_WE}_shader_target
            DEPENDS "${SHADER_SPV_FILE}"
            COMMENT "Building shader ${FILE_WE}.spv"
        )

        list(APPEND SHADER_TARGETS ${FILE_WE}_shader_target)
    endforeach()

    # Create a single target that depends on all shaders
    add_custom_target(compile_all_shaders
        DEPENDS ${SHADER_TARGETS}
        COMMENT "Compiling all shaders"
    )

    # Make the main project depend on all shaders being compiled first
    add_dependencies(${PROJECT_NAME} compile_all_shaders)
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
