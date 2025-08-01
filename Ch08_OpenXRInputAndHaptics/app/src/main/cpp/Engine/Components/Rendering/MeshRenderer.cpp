﻿#include "MeshRenderer.h"
#include "../Core/Transform.h"
#include "../../Core/GameObject.h"
#include "../../Core/Scene.h"
#include "../../../OpenXR/OpenXRCoreMgr.h"
#include "../../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "Material.h"
#include "Camera.h"
#include "../../Rendering/Vertex.h"
#include <DebugOutput.h>

MeshRenderer::~MeshRenderer()
{
    DestroyBuffers();
}

void MeshRenderer::SetMesh(std::shared_ptr<IMesh> mesh)
{
    m_mesh = mesh;
    if (m_buffersCreated)
    {
        DestroyBuffers();
    }
    CreateBuffers();
}

void MeshRenderer::Initialize()
{
    if (m_mesh)
    {
        CreateBuffers();
    }
}

void MeshRenderer::Tick(float deltaTime)
{
    if (m_mesh && m_buffersCreated)
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
    if (!m_mesh) return;

    const auto& verticesWithNormals = m_mesh->GetVerticesWithNormals();
    const auto& indices = m_mesh->GetIndices();

    GraphicsAPI::BufferCreateInfo vertexBufferInfo;
    vertexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::VERTEX;
    vertexBufferInfo.stride = sizeof(Vertex);
    vertexBufferInfo.size = verticesWithNormals.size() * sizeof(Vertex);
    vertexBufferInfo.data = const_cast<void*>(static_cast<const void*>(verticesWithNormals.data()));
    m_vertexBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(vertexBufferInfo);

    GraphicsAPI::BufferCreateInfo indexBufferInfo;
    indexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::INDEX;
    indexBufferInfo.stride = sizeof(uint32_t);
    indexBufferInfo.size = indices.size() * sizeof(uint32_t);
    indexBufferInfo.data = const_cast<void*>(static_cast<const void*>(indices.data()));
    m_indexBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(indexBufferInfo);

    m_buffersCreated = true;
}


void MeshRenderer::RenderMesh()
{
    Transform* transform = GetGameObject()->GetComponent<Transform>();
    Material* material = GetGameObject()->GetComponent<Material>();

    if (!transform || !material)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Missing transform or material");
        return;
    }

    if (!m_vertexBuffer || !m_indexBuffer || !m_mesh)
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

    const Camera::RenderSettings& cameraSettings = activeCamera->GetRenderSettings();
    if (!cameraSettings.colorImage || cameraSettings.width == 0 || cameraSettings.height == 0)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Invalid camera render settings");
        return;
    }

    void* pipeline = material->GetOrCreatePipeline();
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
    viewport.width = cameraSettings.width;
    viewport.height = cameraSettings.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetViewports(&viewport, 1);

    GraphicsAPI::Rect2D scissor = {{0, 0}, {cameraSettings.width, cameraSettings.height}};
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetScissors(&scissor, 1);

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetPipeline(pipeline);

    const XrMatrix4x4f& modelMatrix = transform->GetModelMatrix();
    const XrMatrix4x4f& viewMatrix = activeCamera->GetViewMatrix();
    const XrMatrix4x4f& projectionMatrix = activeCamera->GetProjectionMatrix();

    struct ObjectRenderData
    {
        XrMatrix4x4f viewProj;
        XrMatrix4x4f modelViewProj;
        XrMatrix4x4f model;
        XrVector4f color;
        XrVector4f pad1, pad2, pad3;
    };

    ObjectRenderData renderData;
    XrMatrix4x4f_Multiply(&renderData.viewProj, &projectionMatrix, &viewMatrix);
    renderData.model = modelMatrix;
    XrMatrix4x4f_Multiply(&renderData.modelViewProj, &renderData.viewProj, &renderData.model);
    renderData.color = material->GetColor();

    if (!m_uniformBuffer)
    {
        GraphicsAPI::BufferCreateInfo uniformBufferInfo;
        uniformBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::UNIFORM;
        uniformBufferInfo.size = sizeof(ObjectRenderData);
        uniformBufferInfo.data = nullptr;
        m_uniformBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(uniformBufferInfo);
    }

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetBufferData(m_uniformBuffer, 0, sizeof(ObjectRenderData), &renderData);

    GraphicsAPI::DescriptorInfo objectDataDescriptor;
    objectDataDescriptor.bindingIndex = 0;
    objectDataDescriptor.resource = m_uniformBuffer;
    objectDataDescriptor.type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    objectDataDescriptor.stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    objectDataDescriptor.readWrite = false;
    objectDataDescriptor.bufferOffset = 0;
    objectDataDescriptor.bufferSize = sizeof(ObjectRenderData);
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetDescriptor(objectDataDescriptor);

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->UpdateDescriptors();

    uint32_t indexCount = m_mesh->GetIndexCount();
    if (indexCount == 0)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Index count is 0");
        return;
    }

    void* vertexBuffer = m_vertexBuffer;
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetVertexBuffers(&vertexBuffer, 1);

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetIndexBuffer(m_indexBuffer);

    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DrawIndexed(indexCount);
}

void MeshRenderer::DestroyBuffers()
{
    if (m_vertexBuffer)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyBuffer(m_vertexBuffer);
        m_vertexBuffer = nullptr;
    }
    if (m_indexBuffer)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyBuffer(m_indexBuffer);
        m_indexBuffer = nullptr;
    }
    if (m_uniformBuffer)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyBuffer(m_uniformBuffer);
        m_uniformBuffer = nullptr;
    }
    m_buffersCreated = false;
}