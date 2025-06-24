#pragma once

#include <GraphicsAPI.h>
#include "SceneRenderer.h"

class OpenXRRenderer
{
public:
    OpenXRRenderer(GraphicsAPI_Type apiType);
    ~OpenXRRenderer();

    void SetSceneRenderer(SceneRenderer* sceneRenderer);
    void RenderFrame();

private:
    void RenderLayer();
    void RenderView(int viewIndex);

    void SetupRenderState(int viewIndex, void* colorImage, void* depthImage);

    GraphicsAPI_Type m_apiType;
    SceneRenderer* m_sceneRenderer = nullptr;

    float m_nearPlane = 0.05f;
    float m_farPlane = 1000.0f;
};