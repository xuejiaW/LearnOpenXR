#include "TableFloorScene.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Components/Core/Transform.h"
#include "../Engine/Components/Rendering/MeshRenderer.h"
#include "../Engine/Components/Rendering/Material.h"
#include "../Engine/Components/Rendering/Camera.h"
#include "../Engine/Components/Rendering/RenderSettings.h"
#include "../Engine/Components/XRDevices/XRHmdDriver.h"
#include "../Engine/Rendering/Mesh/CubeMesh.h"
#include "../Application/Components/TestControllerHaptics.h"
#include <DebugOutput.h>
#include <memory>

#include "../Engine/Components/XRDevices/XRControllerDriver.h"

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

    GameObject* testControllerHapticsObject = m_scene->CreateGameObject("TestControllerHaptics");
    testControllerHapticsObject->AddComponent<TestControllerHaptics>();

    GameObject* rightControllerObject = m_scene->CreateGameObject("RightController");
    Transform* rightTransform = rightControllerObject->AddComponent<Transform>();
    rightTransform->SetScale({0.05f, 0.05f, 0.05f});
    XRControllerDriver* rightControllerDriver = rightControllerObject->AddComponent<XRControllerDriver>();
    rightControllerDriver->SetHandedness(1);
    MeshRenderer* rightControllerRenderer = rightControllerObject->AddComponent<MeshRenderer>();
    rightControllerRenderer->SetMesh(cubeMesh);
    Material* rightControllerMaterial = rightControllerObject->AddComponent<Material>("VertexShader.spv", "PixelShader.spv", VULKAN);
    rightControllerMaterial->SetColor({0.2f, 0.2f, 0.2f, 1.0f});

    GameObject* leftControllerObject = m_scene->CreateGameObject("RightController");
    Transform* leftTransform = leftControllerObject->AddComponent<Transform>();
    leftTransform->SetScale({0.05f, 0.05f, 0.05f});
    XRControllerDriver* leftControllerDriver = leftControllerObject->AddComponent<XRControllerDriver>();
    leftControllerDriver->SetHandedness(0);
    MeshRenderer* leftControllerRenderer = leftControllerObject->AddComponent<MeshRenderer>();
    leftControllerRenderer->SetMesh(cubeMesh);
    Material* leftControllerMaterial = leftControllerObject->AddComponent<Material>("VertexShader.spv", "PixelShader.spv", VULKAN);
    leftControllerMaterial->SetColor({0.2f, 0.2f, 0.2f, 1.0f});

    XR_TUT_LOG("TableFloorScene::CreateSceneObjects() - All objects created including test cube at (0,0,-2)");
}