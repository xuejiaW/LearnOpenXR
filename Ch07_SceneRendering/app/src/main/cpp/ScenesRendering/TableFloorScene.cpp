#include "TableFloorScene.h"
#include "Rendering/Geometry/CubeGeometry.h"
#include "Rendering/Material.h"
#include <memory>

TableFloorScene::TableFloorScene() {}

TableFloorScene::~TableFloorScene() {}

void TableFloorScene::Initialize()
{
    CreateSceneObjects();
}

void TableFloorScene::Update(float deltaTime)
{
}

void TableFloorScene::CreateSceneObjects()
{
    auto cubeGeometry = std::make_shared<CubeGeometry>();
    auto material = std::make_shared<Material>("VertexShader.spv", "PixelShader.spv", VULKAN);
    
    SceneObject floor;
    floor.pose = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM, 0.0f}};
    floor.scale = {2.0f, 0.1f, 2.0f};
    floor.color = {0.4f, 0.5f, 0.5f};
    floor.geometry = cubeGeometry;
    floor.material = material;
    
    SceneObject table;
    table.pose = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, -m_viewHeightM + 0.9f, -0.7f}};
    table.scale = {1.0f, 0.2f, 1.0f};
    table.color = {0.6f, 0.6f, 0.4f};
    table.geometry = cubeGeometry;
    table.material = material;
    
    m_objects.push_back(floor);
    m_objects.push_back(table);
}
