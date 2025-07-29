#include "TableFloorScene.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Core/Transform.h"
#include "../Engine/Components/Rendering/MeshRenderer.h"
#include "../Engine/Components/Rendering/Material.h"
#include "../Engine/Components/Rendering/Camera.h"
#include "../Engine/Rendering/Mesh/CubeMesh.h"
#include <DebugOutput.h>
#include <memory>

TableFloorScene::TableFloorScene() : m_scene(std::make_unique<Scene>("TableFloorScene")) {}

TableFloorScene::~TableFloorScene() {}

void TableFloorScene::Initialize()
{
    CreateSceneObjects();
}

void TableFloorScene::Update(float deltaTime)
{
    m_scene->Update(deltaTime);
}

void TableFloorScene::CreateSceneObjects()
{
    // Create shared resources first
    auto cubeMesh = std::make_shared<CubeMesh>(1.0f);

    // Create camera first - it will automatically set itself as active
    GameObject* cameraObject = m_scene->CreateGameObject("Camera");
    Camera* camera = cameraObject->AddComponent<Camera>();

    // Configure camera render settings - NO LONGER NEED TO SET PIPELINE
    Camera::RenderSettings settings;
    settings.width = 1024;  // Default resolution
    settings.height = 1024;
    settings.colorImage = nullptr;  // Will be set by OpenXR runtime
    settings.depthImage = nullptr;  // Will be set by OpenXR runtime
    settings.blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    settings.clearColor = {0.0f, 0.0f, 0.2f, 1.0f};
    // settings.pipeline is no longer needed - Materials will create their own compatible pipelines
    camera->SetRenderSettings(settings);

    XR_TUT_LOG("TableFloorScene::CreateSceneObjects() - Camera created and set as active");

    // Create floor - make it more visible and properly positioned
    GameObject* floorObject = m_scene->CreateGameObject("Floor");
    Transform* floorTransform = floorObject->AddComponent<Transform>(
        XrVector3f{0.0f, -m_viewHeightM, -1.0f},                          // position: further away and lower
        XrQuaternionf{0.0f, 0.0f, 0.0f, 1.0f},                   // rotation: no rotation
        XrVector3f{2.0f, 0.1f, 2.0f}                             // scale: make it thicker so it's clearly a cube
        );
    MeshRenderer* floorRenderer = floorObject->AddComponent<MeshRenderer>();
    floorRenderer->SetMesh(cubeMesh);
    Material* floorMaterial = floorObject->AddComponent<Material>("VertexShader.spv", "PixelShader.spv", VULKAN);
    floorMaterial->SetColor({0.4f, 0.5f, 0.5f, 1.0f});
    floorMaterial->Initialize(); // Material will get camera settings automatically

    // Create table
    GameObject* tableObject = m_scene->CreateGameObject("Table");
    Transform* tableTransform = tableObject->AddComponent<Transform>(
        XrVector3f{0.0f, -m_viewHeightM + 0.9f, -0.7f},          // position
        XrQuaternionf{0.0f, 0.0f, 0.0f, 1.0f},                   // rotation
        XrVector3f{1.0f, 0.2f, 1.0f}                             // scale
        );
    MeshRenderer* tableRenderer = tableObject->AddComponent<MeshRenderer>();
    tableRenderer->SetMesh(cubeMesh);
    Material* tableMaterial = tableObject->AddComponent<Material>("VertexShader.spv", "PixelShader.spv", VULKAN);
    tableMaterial->SetColor({0.6f, 0.6f, 0.4f, 1.0f});
    tableMaterial->Initialize(); // Material will get camera settings automatically

    XR_TUT_LOG("TableFloorScene::CreateSceneObjects() - All objects created including test cube at (0,0,-2)");
}