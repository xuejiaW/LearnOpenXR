#include "MeshRenderer.h"
#include "../Core/Transform.h"
#include "../../Core/GameObject.h"
#include "../../Core/Scene.h"
#include "../../../OpenXR/OpenXRCoreMgr.h"
#include "../../../OpenXR/OpenXRGraphicsAPI/OpenXRGraphicsAPI.h"
#include "Material.h"
#include "Camera.h"
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

    const auto& vertices = m_mesh->GetVertices();
    const auto& indices = m_mesh->GetIndices();

    // Create vertex buffer
    GraphicsAPI::BufferCreateInfo vertexBufferInfo;
    vertexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::VERTEX;
    vertexBufferInfo.stride = sizeof(float) * 4;
    vertexBufferInfo.size = vertices.size() * sizeof(XrVector4f);
    vertexBufferInfo.data = const_cast<void*>(static_cast<const void*>(vertices.data()));
    m_vertexBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(vertexBufferInfo);

    // Create index buffer
    GraphicsAPI::BufferCreateInfo indexBufferInfo;
    indexBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::INDEX;
    indexBufferInfo.stride = sizeof(uint32_t);
    indexBufferInfo.size = indices.size() * sizeof(uint32_t);
    indexBufferInfo.data = const_cast<void*>(static_cast<const void*>(indices.data()));
    m_indexBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(indexBufferInfo);

    // Create face normals buffer (6 faces, exactly like CubeGeometry)
    // Don't use m_mesh->GetNormals() as it might have per-vertex normals
    XrVector4f faceNormals[6] = {
            {1.0f, 0.0f, 0.0f, 0.0f},   // +X face
            {-1.0f, 0.0f, 0.0f, 0.0f},  // -X face  
            {0.0f, 1.0f, 0.0f, 0.0f},   // +Y face
            {0.0f, -1.0f, 0.0f, 0.0f},  // -Y face
            {0.0f, 0.0f, 1.0f, 0.0f},   // +Z face
            {0.0f, 0.0f, -1.0f, 0.0f}   // -Z face
        };

    GraphicsAPI::BufferCreateInfo faceNormalsBufferInfo;
    faceNormalsBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::UNIFORM;
    faceNormalsBufferInfo.stride = 0; // Uniform buffer, no stride
    faceNormalsBufferInfo.size = sizeof(faceNormals);
    faceNormalsBufferInfo.data = faceNormals;
    m_faceNormalsBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(faceNormalsBufferInfo);

    m_buffersCreated = true;
}

void MeshRenderer::UpdateBuffers()
{
    // Static geometry data doesn't need updating after creation
    // Only uniform buffers (like ObjectRenderData) need per-frame updates
    // This method is kept for potential future dynamic mesh support
}void MeshRenderer::RenderMesh()
{
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Starting render");

    Transform* transform = GetGameObject()->GetComponent<Transform>();
    Material* material = GetGameObject()->GetComponent<Material>();

    if (!transform || !material)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Missing transform or material");
        return;
    }

    XR_TUT_LOG("MeshRenderer::RenderMesh() - Got transform and material");

    // Check if buffers are valid
    if (!m_vertexBuffer || !m_indexBuffer || !m_mesh)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Invalid buffers or mesh");
        return;
    }

    XR_TUT_LOG("MeshRenderer::RenderMesh() - Buffers are valid");

    // Get active camera for render settings
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

    // Get or create the rendering pipeline
    void* pipeline = material->GetOrCreatePipeline();
    if (!pipeline)
    {
        XR_TUT_LOG_ERROR("Failed to get or create pipeline for material");
        return;
    }

    XR_TUT_LOG("MeshRenderer::RenderMesh() - Got pipeline successfully");

    // Set render attachments with the material's pipeline
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Setting render attachments");
    void* colorImages[] = {cameraSettings.colorImage};
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetRenderAttachments(
        colorImages, 1,
        cameraSettings.depthImage,
        cameraSettings.width, cameraSettings.height,
        pipeline
        );

    // Set viewport
    GraphicsAPI::Viewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = cameraSettings.width;
    viewport.height = cameraSettings.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    XR_TUT_LOG("MeshRenderer::RenderMesh() - Setting viewport");
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetViewports(&viewport, 1);

    // Set scissor
    GraphicsAPI::Rect2D scissor = {{0, 0}, {cameraSettings.width, cameraSettings.height}};
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Setting scissor");
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetScissors(&scissor, 1);

    // Set the pipeline
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Setting pipeline");
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetPipeline(pipeline);
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Pipeline set successfully");

    // CRITICAL: Calculate ObjectRenderData exactly like SceneRenderer
    const XrMatrix4x4f& modelMatrix = transform->GetModelMatrix();
    const XrMatrix4x4f& viewMatrix = activeCamera->GetViewMatrix();
    const XrMatrix4x4f& projectionMatrix = activeCamera->GetProjectionMatrix();

    // Debug: Print matrix validity
    bool viewValid = (viewMatrix.m[0] != 0.0f || viewMatrix.m[5] != 0.0f || viewMatrix.m[10] != 0.0f);
    bool projValid = (projectionMatrix.m[0] != 0.0f || projectionMatrix.m[5] != 0.0f || projectionMatrix.m[10] != 0.0f);
    XR_TUT_LOG("MeshRenderer::RenderMesh() - View matrix valid: " << viewValid << ", Proj matrix valid: " << projValid);

    // Calculate matrices exactly like SceneRenderer
    struct ObjectRenderData
    {
        XrMatrix4x4f viewProj;
        XrMatrix4x4f modelViewProj;
        XrMatrix4x4f model;
        XrVector4f color;
        XrVector4f pad1, pad2, pad3;
    };

    ObjectRenderData renderData;
    // Calculate viewProj matrix CORRECTLY like SceneRenderer
    XrMatrix4x4f_Multiply(&renderData.viewProj, &projectionMatrix, &viewMatrix);
    renderData.model = modelMatrix;
    XrMatrix4x4f_Multiply(&renderData.modelViewProj, &renderData.viewProj, &renderData.model);
    renderData.color = material->GetColor(); // Read color from material instead of hardcoding

    // Debug info
    XrVector3f position = transform->GetPosition();
    XrVector3f scale = transform->GetScale();
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Object position: (" << position.x << ", " << position.y << ", " << position.z << ")");
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Object scale: (" << scale.x << ", " << scale.y << ", " << scale.z << ")");

    // Print actual matrix values for debugging
    XR_TUT_LOG(
        "MeshRenderer::RenderMesh() - Model matrix[0]: " << modelMatrix.m[0] << ", [5]: " << modelMatrix.m[5] << ", [10]: " << modelMatrix.m[10]);
    XR_TUT_LOG("MeshRenderer::RenderMesh() - View matrix[0]: " << viewMatrix.m[0] << ", [5]: " << viewMatrix.m[5] << ", [10]: " << viewMatrix.m[10]);
    XR_TUT_LOG(
        "MeshRenderer::RenderMesh() - Proj matrix[0]: " << projectionMatrix.m[0] << ", [5]: " << projectionMatrix.m[5] << ", [10]: " <<
        projectionMatrix.m[10]);

    // Create uniform buffer for ObjectRenderData (exactly like SceneRenderer)
    if (!m_uniformBuffer)
    {
        GraphicsAPI::BufferCreateInfo uniformBufferInfo;
        uniformBufferInfo.type = GraphicsAPI::BufferCreateInfo::Type::UNIFORM;
        uniformBufferInfo.size = sizeof(ObjectRenderData);
        uniformBufferInfo.data = nullptr;
        m_uniformBuffer = OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->CreateBuffer(uniformBufferInfo);
        XR_TUT_LOG("MeshRenderer::RenderMesh() - Created uniform buffer");
    }

    // Update uniform buffer with ObjectRenderData (exactly like SceneRenderer)
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetBufferData(m_uniformBuffer, 0, sizeof(ObjectRenderData), &renderData);
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Updated uniform buffer with ObjectRenderData");

    // Set descriptor for binding 0 (ObjectRenderData)
    GraphicsAPI::DescriptorInfo objectDataDescriptor;
    objectDataDescriptor.bindingIndex = 0;
    objectDataDescriptor.resource = m_uniformBuffer;
    objectDataDescriptor.type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    objectDataDescriptor.stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    objectDataDescriptor.readWrite = false;
    objectDataDescriptor.bufferOffset = 0;
    objectDataDescriptor.bufferSize = sizeof(ObjectRenderData);
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetDescriptor(objectDataDescriptor);
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Set ObjectRenderData descriptor");

    // Set descriptor for binding 1 (Face Normals) - exactly like SceneRenderer
    GraphicsAPI::DescriptorInfo normalsDescriptor;
    normalsDescriptor.bindingIndex = 1;
    normalsDescriptor.resource = m_faceNormalsBuffer;  // Use the pre-created face normals buffer
    normalsDescriptor.type = GraphicsAPI::DescriptorInfo::Type::BUFFER;
    normalsDescriptor.stage = GraphicsAPI::DescriptorInfo::Stage::VERTEX;
    normalsDescriptor.readWrite = false;
    normalsDescriptor.bufferOffset = 0;
    normalsDescriptor.bufferSize = 6 * sizeof(XrVector4f);  // Only 6 face normals
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetDescriptor(normalsDescriptor);
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Set face normals descriptor");

    // Don't set binding=2 - SceneRenderer doesn't set it either
    // Update descriptors - only bindings 0 and 1, exactly like SceneRenderer
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->UpdateDescriptors();
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Updated descriptors (0,1 only - matching SceneRenderer)");

    // Check index count
    uint32_t indexCount = m_mesh->GetIndexCount();
    if (indexCount == 0)
    {
        XR_TUT_LOG_ERROR("MeshRenderer::RenderMesh() - Index count is 0");
        return;
    }
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Index count: " << indexCount);

    // Set vertex buffer (only 1 buffer like SceneRenderer)
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Setting vertex buffer");
    void* vertexBuffer = m_vertexBuffer;
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetVertexBuffers(&vertexBuffer, 1);
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Vertex buffer set successfully");

    // Set index buffer
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Setting index buffer");
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->SetIndexBuffer(m_indexBuffer);
    XR_TUT_LOG("MeshRenderer::RenderMesh() - Index buffer set successfully");

    // Draw
    XR_TUT_LOG("MeshRenderer::RenderMesh() - About to call DrawIndexed");
    OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DrawIndexed(indexCount);
    XR_TUT_LOG("MeshRenderer::RenderMesh() - DrawIndexed completed successfully");
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
    if (m_colorBuffer)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyBuffer(m_colorBuffer);
        m_colorBuffer = nullptr;
    }
    if (m_faceNormalsBuffer)
    {
        OpenXRCoreMgr::openxrGraphicsAPI->graphicsAPI->DestroyBuffer(m_faceNormalsBuffer);
        m_faceNormalsBuffer = nullptr;
    }
    m_buffersCreated = false;
}