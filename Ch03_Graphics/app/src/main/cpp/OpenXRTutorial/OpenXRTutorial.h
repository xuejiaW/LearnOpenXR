#pragma once

#include <GraphicsAPI.h>
#include <xr_linear_algebra.h>
#include <openxr/openxr.h>
#include "../OpenXR/OpenXRDisplay/RenderLayerInfo.h"

#if defined(__ANDROID__)
#include <android_native_app_glue.h>
#endif

class OpenXRTutorial
{
public:
    OpenXRTutorial(GraphicsAPI_Type apiType);
    ~OpenXRTutorial();

    void Run();

    static GraphicsAPI_Type m_apiType;

#if defined(__ANDROID__)
  public:
    static android_app *androidApp;

    struct AndroidAppState
    {
        ANativeWindow *nativeWindow = nullptr;
        bool resumed = false;
    };
    static AndroidAppState androidAppState;

    static void AndroidAppHandleCmd(struct android_app *app, int32_t cmd);
#endif

private:

    // Data used to render a cuboid
    struct CuboidConstants
    {
        XrMatrix4x4f viewProj;
        XrMatrix4x4f modelViewProj;
        XrMatrix4x4f model;
        XrVector4f color;
        XrVector4f pad1;
        XrVector4f pad2;
        XrVector4f pad3;
    };

    // Reused for every draw
    CuboidConstants cuboidsConstants;

    XrVector4f normals[6] = {
        {1.00f, 0.00f, 0.00f, 0},
        {-1.00f, 0.00f, 0.00f, 0},
        {0.00f, 1.00f, 0.00f, 0},
        {0.00f, -1.00f, 0.00f, 0},
        {0.00f, 0.00f, 1.00f, 0},
        {0.00f, 0.0f, -1.00f, 0}};

    void PollSystemEvents();

    void CreateReferenceSpaces();
    void DestroyReferenceSpace();

    void RenderFrame();
    bool RenderLayer(RenderLayerInfo& renderLayerInfo);

    void CreateResources();
    void RenderCuboid(XrPosef pose, XrVector3f scale, XrVector3f color);
    void DestroyResources();

    float m_viewHeightM = 1.5f;

    // Vertex and index buffers: geometry for our cuboids.
    void* m_vertexBuffer = nullptr;
    void* m_indexBuffer = nullptr;
    // Camera values constant buffer for the shaders.
    void* m_uniformBuffer_Camera = nullptr;
    // The normals are stored in a uniform buffer to simplify our vertex geometry.
    void* m_uniformBuffer_Normals = nullptr;

    // We use only two shaders in this app.
    void *m_vertexShader = nullptr, *m_fragmentShader = nullptr;

    // The pipeline is a graphics-API specific state object.
    void* m_pipeline = nullptr;

    XrSpace m_ActiveSpaces = XR_NULL_HANDLE;

};

