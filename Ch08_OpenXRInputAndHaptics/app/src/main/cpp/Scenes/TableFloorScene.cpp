#include "TableFloorScene.h"
#include <DebugOutput.h>
#include "../Engine/Components/Core/Transform.h"
#include "../Engine/Components/Rendering/Camera.h"
#include "../Engine/Components/Rendering/MeshRenderer.h"
#include "../Engine/Components/XRDevices/XRHmdDriver.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Rendering/Material.h"
#include "../Engine/Rendering/Mesh/CubeMesh.h"
#include "../Engine/Rendering/Shader.h"
#include "../Engine/Components/XRDevices/XRControllerDriver.h"
#include "../Application/Components/TestControllerHaptics.h"
#include <memory>


TableFloorScene::TableFloorScene() : Scene("TableFloorScene") {}

TableFloorScene::~TableFloorScene() {}

void TableFloorScene::Initialize() { CreateSceneObjects(); }

void TableFloorScene::CreateSceneObjects()
{
    auto cubeMesh = std::make_shared<CubeMesh>(1.0f);

    auto vertexShader = std::make_shared<Shader>("shaders/VertexShader.spv", Shader::VERTEX, VULKAN);
    auto fragmentShader = std::make_shared<Shader>("shaders/PixelShader.spv", Shader::FRAGMENT, VULKAN);

    auto floorMaterial = std::make_shared<Material>();
    floorMaterial->SetShaders(vertexShader, fragmentShader);
    floorMaterial->SetColor(0.4f, 0.5f, 0.5f, 1.0f);

    auto tableMaterial = std::make_shared<Material>();
    tableMaterial->SetShaders(vertexShader, fragmentShader);
    tableMaterial->SetColor(0.6f, 0.6f, 0.4f, 1.0f);

    auto controllerMaterial = std::make_shared<Material>();
    controllerMaterial->SetShaders(vertexShader, fragmentShader);
    controllerMaterial->SetColor(0.2f, 0.2f, 0.2f, 1.0f);

    GameObject* cameraObject = CreateGameObject("XRCamera");
    Transform* cameraTransform =
        cameraObject->AddComponent<Transform>(XrVector3f{0.0f, 0.0, 0.0f}, XrQuaternionf{0.0f, 0.0f, 0.0f, 1.0f}, XrVector3f{1.0f, 1.0f, 1.0f});

    Camera* camera = cameraObject->AddComponent<Camera>();
    camera->SetProjectionParameters(0.05f, 1000.0f);
    camera->SetClearColor(0.17f, 0.17f, 0.17f, 1.0f);
    
    cameraObject->AddComponent<XRHmdDriver>();

    GameObject* floorObject = CreateGameObject("Floor");
    floorObject->AddComponent<Transform>(XrVector3f{0.0f, -m_viewHeightM, -1.0f}, XrQuaternionf{0.0f, 0.0f, 0.0f, 1.0f},
                                         XrVector3f{2.0f, 0.1f, 2.0f});
    MeshRenderer* floorRenderer = floorObject->AddComponent<MeshRenderer>();
    floorRenderer->SetMesh(cubeMesh);
    floorRenderer->SetMaterial(floorMaterial);

    GameObject* tableObject = CreateGameObject("Table");
    tableObject->AddComponent<Transform>(XrVector3f{0.0f, -m_viewHeightM + 0.9f, -0.7f}, XrQuaternionf{0.0f, 0.0f, 0.0f, 1.0f},
                                         XrVector3f{1.0f, 0.2f, 1.0f});
    MeshRenderer* tableRenderer = tableObject->AddComponent<MeshRenderer>();
    tableRenderer->SetMesh(cubeMesh);
    tableRenderer->SetMaterial(tableMaterial);

    GameObject* testControllerHapticsObject = CreateGameObject("TestControllerHaptics");
    testControllerHapticsObject->AddComponent<TestControllerHaptics>();

    GameObject* rightControllerObject = CreateGameObject("RightController");
    Transform* rightTransform = rightControllerObject->AddComponent<Transform>();
    rightTransform->SetScale({0.05f, 0.05f, 0.05f});
    XRControllerDriver* rightControllerDriver = rightControllerObject->AddComponent<XRControllerDriver>();
    rightControllerDriver->SetHandedness(1);
    MeshRenderer* rightControllerRenderer = rightControllerObject->AddComponent<MeshRenderer>();
    rightControllerRenderer->SetMesh(cubeMesh);
    rightControllerRenderer->SetMaterial(controllerMaterial);

    GameObject* leftControllerObject = CreateGameObject("LeftController");
    Transform* leftTransform = leftControllerObject->AddComponent<Transform>();
    leftTransform->SetScale({0.05f, 0.05f, 0.05f});
    XRControllerDriver* leftControllerDriver = leftControllerObject->AddComponent<XRControllerDriver>();
    leftControllerDriver->SetHandedness(0);
    MeshRenderer* leftControllerRenderer = leftControllerObject->AddComponent<MeshRenderer>();
    leftControllerRenderer->SetMesh(cubeMesh);
    leftControllerRenderer->SetMaterial(controllerMaterial);
}