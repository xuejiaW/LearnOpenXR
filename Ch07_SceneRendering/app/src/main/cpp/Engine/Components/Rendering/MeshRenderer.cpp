#include "MeshRenderer.h"
#include "Camera.h"
#include "RenderSettings.h"
#include "../Core/Transform.h"
#include "../../Core/GameObject.h"
#include "../../Core/Scene.h"
#include "../../../OpenXR/OpenXRCoreMgr.h"
#include "../../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "../../Rendering/Vertex.h"
#include <DebugOutput.h>

#include "ObjectRenderData.h"

MeshRenderer::~MeshRenderer()
{
    DestroyBuffers();
}

void MeshRenderer::SetMesh(std::shared_ptr<IMesh> mesh)
{
    m_Mesh = mesh;
    if (m_BuffersCreated)
    {
        DestroyBuffers();
    }
    CreateBuffers();
}

void MeshRenderer::SetMaterial(std::shared_ptr<Material> material)
{
    m_Material = material;
}

void MeshRenderer::Initialize()
{
    if (m_Mesh)
    {
        CreateBuffers();
    }
}

void MeshRenderer::Tick(float deltaTime)
{
    if (m_Mesh && m_Material && m_BuffersCreated)
    {
        RenderMesh();
    }
}

void MeshRenderer::Destroy()
{
    DestroyBuffers();
}

void MeshRenderer::CreateBuffers()
{
    if (!m_Mesh) return;

    const auto& verticesWithNormals = m_Mesh->GetVerticesWithNormals();
    const auto& indices = m_Mesh->GetIndices();

    GraphicsAPI::BufferCreateInfo vertexBufferInfo;
    vertexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::VERTEX;
    vertexBufferInfo.stride = sizeof(Vertex);
    vertexBufferInfo.size = verticesWithNormals.size() * sizeof(Vertex);
    vertexBufferInfo.data = const_cast<void*>(static_cast<const void*>(verticesWithNormals.data()));
    m_VertexBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(vertexBufferInfo);

    GraphicsAPI::BufferCreateInfo indexBufferInfo;
    indexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::INDEX;
    indexBufferInfo.stride = sizeof(uint32_t);
    indexBufferInfo.size = indices.size() * sizeof(uint32_t);
    indexBufferInfo.data = const_cast<void*>(static_cast<const void*>(indices.data()));
    m_IndexBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(indexBufferInfo);

    m_BuffersCreated = true;
}


void MeshRenderer::RenderMesh()
{
    Transform* transform = GetGameObject()->GetComponent<Transform>();

    if (!transform || !m_Material)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Missing transform or material");
        return;
    }

    if (!m_VertexBuffer || !m_IndexBuffer || !m_Mesh)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Invalid buffers or mesh");
        return;
    }

    Camera* activeCamera = Scene::GetActiveCamera();
    if (!activeCamera)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - No active camera found");
        return;
    }

    const RenderSettings& cameraSettings = activeCamera->GetRenderSettings();
    if (!cameraSettings.colorImage || cameraSettings.width == 0 || cameraSettings.height == 0)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Invalid camera render settings");
        return;
    }

    void* pipeline = m_Material->GetOrCreatePipeline();
    if (!pipeline)
    {
        XR_TUT_LOG_ERROR("Failed to get or create pipeline for material");
        return;
    }

    void* colorImages[] = {cameraSettings.colorImage};
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetRenderAttachments(
        colorImages, 1,
        cameraSettings.depthImage,
        cameraSettings.width, cameraSettings.height,
        pipeline
        );

    GraphicsAPI::Viewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = static_cast<float>(cameraSettings.width);
    viewport.height = static_cast<float>(cameraSettings.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetViewports(&viewport, 1);

    GraphicsAPI::Rect2D scissor = {{0, 0}, {cameraSettings.width, cameraSettings.height}};
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetScissors(&scissor, 1);

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetPipeline(pipeline);

    const XrMatrix4x4f& modelMatrix = transform->GetModelMatrix();
    const XrMatrix4x4f& viewMatrix = activeCamera->GetViewMatrix();
    const XrMatrix4x4f& projectionMatrix = activeCamera->GetProjectionMatrix();

    ObjectRenderData renderData;
    XrMatrix4x4f_Multiply(&renderData.viewProj, &projectionMatrix, &viewMatrix);
    renderData.model = modelMatrix;
    XrMatrix4x4f_Multiply(&renderData.modelViewProj, &renderData.viewProj, &renderData.model);
    renderData.color = m_Material->GetColor();

    if (!m_UniformBuffer)
    {
        GraphicsAPI::BufferCreateInfo uniformBufferInfo;
        uniformBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::UNIFORM;
        uniformBufferInfo.size = sizeof(ObjectRenderData);
        uniformBufferInfo.data = nullptr;
        m_UniformBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(uniformBufferInfo);
    }

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetBufferData(m_UniformBuffer, 0, sizeof(ObjectRenderData), &renderData);

    GraphicsAPI::DescriptorInfo objectDataDescriptor;
    objectDataDescriptor.bindingIndex = 0;
    objectDataDescriptor.resource = m_UniformBuffer;
    objectDataDescriptor.type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    objectDataDescriptor.stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    objectDataDescriptor.readWrite = false;
    objectDataDescriptor.bufferOffset = 0;
    objectDataDescriptor.bufferSize = sizeof(ObjectRenderData);
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetDescriptor(objectDataDescriptor);

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->UpdateDescriptors();

    uint32_t indexCount = static_cast<uint32_t>(m_Mesh->GetIndexCount());
    if (indexCount == 0)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Index count is 0");
        return;
    }

    void* vertexBuffer = m_VertexBuffer;
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetVertexBuffers(&vertexBuffer, 1);

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetIndexBuffer(m_IndexBuffer);

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DrawIndexed(indexCount);
}

void MeshRenderer::DestroyBuffers()
{
    if (m_VertexBuffer)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyBuffer(m_VertexBuffer);
        m_VertexBuffer = nullptr;
    }
    if (m_IndexBuffer)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyBuffer(m_IndexBuffer);
        m_IndexBuffer = nullptr;
    }
    if (m_UniformBuffer)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyBuffer(m_UniformBuffer);
        m_UniformBuffer = nullptr;
    }
    m_BuffersCreated = false;
}