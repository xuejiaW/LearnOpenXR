// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

// OpenXR Tutorial for Khronos Group

#pragma once
#include <GraphicsAPI.h>

#if defined(XR_USE_GRAPHICS_API_VULKAN)

// Structure to pass Vulkan initialization data (without OpenXR dependencies)
struct VulkanInitInfo {
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> deviceExtensions;
    VkInstance instance = VK_NULL_HANDLE;           // Pre-created instance (optional)
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // Pre-selected physical device (optional)
};

class GraphicsAPI_Vulkan : public GraphicsAPI {
public:
    GraphicsAPI_Vulkan(const VulkanInitInfo& initInfo);                // New pure Vulkan constructor
    ~GraphicsAPI_Vulkan();

    virtual void* CreateDesktopSwapchain(const SwapchainCreateInfo& swapchainCI) override;
    virtual void DestroyDesktopSwapchain(void*& swapchain) override;
    virtual void* GetDesktopSwapchainImage(void* swapchain, uint32_t index) override;
    virtual void AcquireDesktopSwapchanImage(void* swapchain, uint32_t& index) override;
    virtual void PresentDesktopSwapchainImage(void* swapchain, uint32_t index) override;

    // XR_DOCS_TAG_BEGIN_GetDepthFormat_Vulkan
    virtual int64_t GetDepthFormat() override { return (int64_t)VK_FORMAT_D32_SFLOAT; }
    // XR_DOCS_TAG_END_GetDepthFormat_Vulkan

    virtual void* CreateImage(const ImageCreateInfo& imageCI) override;
    virtual void DestroyImage(void*& image) override;

    virtual void* CreateImageView(const ImageViewCreateInfo& imageViewCI) override;
    virtual void DestroyImageView(void*& imageView) override;

    virtual void* CreateSampler(const SamplerCreateInfo& samplerCI) override;
    virtual void DestroySampler(void*& sampler) override;

    virtual void* CreateBuffer(const BufferCreateInfo& bufferCI) override;
    virtual void DestroyBuffer(void*& buffer) override;

    virtual void* CreateShader(const ShaderCreateInfo& shaderCI) override;
    virtual void DestroyShader(void*& shader) override;

    virtual void* CreatePipeline(const PipelineCreateInfo& pipelineCI) override;
    virtual void DestroyPipeline(void*& pipeline) override;

    virtual void BeginRendering() override;
    virtual void EndRendering() override;

    virtual void SetBufferData(void* buffer, size_t offset, size_t size, void* data) override;

    virtual void ClearColor(void* imageView, float r, float g, float b, float a) override;
    virtual void ClearDepth(void* imageView, float d) override;

    virtual void SetRenderAttachments(void** colorViews, size_t colorViewCount, void* depthStencilView, uint32_t width, uint32_t height, void* pipeline) override;
    virtual void SetViewports(Viewport* viewports, size_t count) override;
    virtual void SetScissors(Rect2D* scissors, size_t count) override;

    virtual void SetPipeline(void* pipeline) override;
    virtual void SetDescriptor(const DescriptorInfo& descriptorInfo) override;
    virtual void UpdateDescriptors() override;
    virtual void SetVertexBuffers(void** vertexBuffers, size_t count) override;
    virtual void SetIndexBuffer(void* indexBuffer) override;    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;

    // Getter methods for OpenXR integration
    VkInstance GetInstance() const { return instance; }
    VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
    VkDevice GetDevice() const { return device; }
    VkQueue GetQueue() const { return queue; }
    uint32_t GetQueueFamilyIndex() const { return queueFamilyIndex; }
    uint32_t GetQueueIndex() const { return queueIndex; }

private:
    virtual const std::vector<int64_t> GetSupportedColorSwapchainFormats() override;
    virtual const std::vector<int64_t> GetSupportedDepthSwapchainFormats() override;

private:
    VkInstance instance{};
    VkPhysicalDevice physicalDevice{};
    VkDevice device{};
    uint32_t queueFamilyIndex = 0xFFFFFFFF;
    uint32_t queueIndex = 0xFFFFFFFF;
    VkQueue queue{};
    VkFence fence{};

    VkCommandPool cmdPool{};
    VkCommandBuffer cmdBuffer{};
    VkDescriptorPool descriptorPool;

    std::vector<const char*> activeInstanceLayers{};
    std::vector<const char*> activeInstanceExtensions{};
    std::vector<const char*> activeDeviceLayer{};
    std::vector<const char*> activeDeviceExtensions{};

    VkImage currentDesktopSwapchainImage = VK_NULL_HANDLE;

    std::unordered_map<VkSwapchainKHR, VkSurfaceKHR> surfaces;
    VkSemaphore acquireSemaphore{};
    VkSemaphore submitSemaphore{};

    std::unordered_map<VkImage, VkImageLayout> imageStates;
    std::unordered_map<VkImage, std::pair<VkDeviceMemory, ImageCreateInfo>> imageResources;
    std::unordered_map<VkImageView, ImageViewCreateInfo> imageViewResources;

    std::unordered_map<VkBuffer, std::pair<VkDeviceMemory, BufferCreateInfo>> bufferResources;

    std::unordered_map<VkShaderModule, ShaderCreateInfo> shaderResources;
    std::unordered_map<VkPipeline, std::tuple<VkPipelineLayout, VkDescriptorSetLayout, VkRenderPass, PipelineCreateInfo>> pipelineResources;

    std::unordered_map<VkCommandBuffer, std::vector<VkFramebuffer>> cmdBufferFramebuffers;
    bool inRenderPass = false;

    VkPipeline setPipeline = VK_NULL_HANDLE;
    std::unordered_map<VkCommandBuffer, std::vector<VkDescriptorSet>> cmdBufferDescriptorSets;
    std::vector<std::tuple<VkWriteDescriptorSet, VkDescriptorBufferInfo, VkDescriptorImageInfo>> writeDescSets;

};
#endif
