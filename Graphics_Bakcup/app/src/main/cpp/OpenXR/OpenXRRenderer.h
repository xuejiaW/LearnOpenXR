#pragma once

#include <GraphicsAPI.h>
#include <memory>

class SceneRenderer;
class IScene;
struct RenderSettings; // 前向声明

class OpenXRRenderer
{
public:
    OpenXRRenderer(GraphicsAPI_Type apiType);
    ~OpenXRRenderer();

    void SetScene(std::shared_ptr<IScene> scene);
    void Initialize();
    void RenderFrame();
    void Cleanup();

private:
    void RenderView(int viewIndex);

    GraphicsAPI_Type m_apiType;
    std::unique_ptr<SceneRenderer> m_sceneRenderer;
    std::shared_ptr<IScene> m_scene;

    float m_nearPlane = 0.05f;
    float m_farPlane = 1000.0f;
};
