#include "TableFloorScene.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Core/Transform.h"
#include "../Engine/Components/Rendering/MeshRenderer.h"
#include "../Engine/Components/Rendering/Material.h"
#include "../Engine/Components/Rendering/Camera.h"
#include "../Engine/Components/Rendering/RenderSettings.h"
#include "../Engine/Components/Rendering/XRHmdDriver.h"
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
    auto cubeMesh = std::make_shared<CubeMesh>(1.0f);

    GameObject* cameraObject = m_scene->CreateGameObject("Camera");
    
    Transform* cameraTransform = cameraObject->AddComponent<Transform>();
    cameraTransform->SetPosition({0.0f, 0.0f, 0.0f});
    cameraTransform->SetRotation({0.0f, 0.0f, 0.0f, 1.0f});
    
    Camera* camera = cameraObject->AddComponent<Camera>();
    camera->SetProjectionParameters(0.05f, 1000.0f);
    
    RenderSettings settings;
    settings.width = 1024;
    settings.height = 1024;
    settings.colorImage = nullptr;
    settings.depthImage = nullptr;
    settings.blendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    settings.clearColor = {0.0f, 0.0f, 0.2f, 1.0f};
    camera->SetRenderSettings(settings);
    
    cameraObject->AddComponent<XRHmdDriver>();

    GameObject* floorObject = m_scene->CreateGameObject("Floor");
    floorObject->AddComponent<Transform>(
        XrVector3f{0.0f, -m_viewHeightM, -1.0f},
        XrQuaternionf{0.0f, 0.0f, 0.0f, 1.0f},
        XrVector3f{2.0f, 0.1f, 2.0f}
        );
    MeshRenderer* floorRenderer = floorObject->AddComponent<MeshRenderer>();
    floorRenderer->SetMesh(cubeMesh);
    Material* floorMaterial = floorObject->AddComponent<Material>("VertexShader.spv", "PixelShader.spv", VULKAN);
    floorMaterial->SetColor({0.4f, 0.5f, 0.5f, 1.0f});
    floorMaterial->Initialize();

    GameObject* tableObject = m_scene->CreateGameObject("Table");
    tableObject->AddComponent<Transform>(
        XrVector3f{0.0f, -m_viewHeightM + 0.9f, -0.7f},
        XrQuaternionf{0.0f, 0.0f, 0.0f, 1.0f},
        XrVector3f{1.0f, 0.2f, 1.0f}
        );
    MeshRenderer* tableRenderer = tableObject->AddComponent<MeshRenderer>();
    tableRenderer->SetMesh(cubeMesh);
    Material* tableMaterial = tableObject->AddComponent<Material>("VertexShader.spv", "PixelShader.spv", VULKAN);
    tableMaterial->SetColor({0.6f, 0.6f, 0.4f, 1.0f});
    tableMaterial->Initialize();

    XR_TUT_LOG("TableFloorScene::CreateSceneObjects() - All objects created including test cube at (0,0,-2)");
}