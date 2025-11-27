#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_CONTROLLER_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_CONTROLLER_IMPL_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dali/graphics-api/graphics-controller.h>

#include <dali/devel-api/threading/thread-pool.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-target.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-resource-transfer-request.h>

namespace Dali
{
namespace Graphics
{
class VulkanGraphics;

namespace Vulkan
{
class Device;
class Surface;
class Buffer;
class Sampler;
class Texture;
class TextureArray;
class SamplerImpl;

/**
 * Class to manage the vulkan graphics backend. This is the main object that clients interact
 * with to get work done on the GPU.
 */
class VulkanGraphicsController : public Graphics::Controller, public Integration::GraphicsConfig
{
public:
  VulkanGraphicsController();
  ~VulkanGraphicsController() override;

  void Initialize(Dali::Graphics::VulkanGraphics& graphicsImplementation,
                  Vulkan::Device&                 graphicsDevice);

  /**
   * Get graphics configuration for info about the graphics subsystem.
   */
  Integration::GraphicsConfig& GetGraphicsConfig() override;

  Vulkan::Device& GetGraphicsDevice();

  void SetResourceBindingHints(const std::vector<SceneResourceBinding>& resourceBindings) override;

  /**
   * @brief Submits array of command buffers
   *
   * Submits command buffers to the graphics pipeline. Submitted commands
   * may be executed instantly or postponed.
   *
   * @param[in] submitInfo Valid SubmitInfo structure
   */
  void SubmitCommandBuffers(const SubmitInfo& submitInfo) override;

  /**
   * @brief Presents render target
   * @param renderTarget render target to present
   */
  void PresentRenderTarget(Graphics::RenderTarget* renderTarget) override;

  /**
   * @brief Waits until the GPU is idle
   */
  void WaitIdle() override;

  /**
   * @brief Lifecycle pause event
   */
  void Pause() override;

  /**
   * @brief Lifecycle resume event
   */
  void Resume() override;

  /**
   * @brief Lifecycle shutdown event
   */
  void Shutdown() override;

  /**
   * @brief Lifecycle destroy event
   */
  void Destroy() override;

  /**
   * @brief Executes batch update of textures
   *
   * This function may perform full or partial update of many textures.
   * The data source may come from:
   * - CPU memory (client side)
   * - GPU memory (another Texture or Buffer)
   *
   * UpdateTextures() is the only way to update unmappable Texture objects.
   * It is recommended to batch updates as it may help with optimizing
   * memory transfers based on dependencies.
   *
   */
  void UpdateTextures(const std::vector<TextureUpdateInfo>&       updateInfoList,
                      const std::vector<TextureUpdateSourceInfo>& sourceList) override;

  /**
   * Auto generates mipmaps for the texture
   * @param[in] texture The texture
   */
  void GenerateTextureMipmaps(const Graphics::Texture& texture) override;

  /**
   * @brief Enables depth/stencil buffer
   *
   * @param[in] renderTarget The surface for which to enable depth/stencil buffer.
   * @param[in] enableDepth True to enable depth
   * @param[in] enableStencil True to enable stencil
   * @return True on success
   */
  bool EnableDepthStencilBuffer(const Graphics::RenderTarget& renderTarget, bool enableDepth, bool enableStencil) override;

  /**
   * @brief Runs garbage collector (if supported)
   *
   * @param[in] numberOfDiscardedRenderers number of discarded renderers
   */
  void RunGarbageCollector(size_t numberOfDiscardedRenderers) override;

  /**
   * @brief Discards unused resources
   */
  void DiscardUnusedResources() override;

  /**
   * @brief Tests whether discard queue is empty
   *
   * @return True if empty
   */
  bool IsDiscardQueueEmpty() override;

  /**
   * @brief Test if the graphics subsystem has resumed & should force a draw
   *
   * @return true if the graphics subsystem requires a re-draw
   */
  bool IsDrawOnResumeRequired() override;

  /**
   * @brief Creates new Buffer object
   *
   * The Buffer object is created with underlying memory. The Buffer
   * specification is immutable. Based on the BufferCreateInfo::usage,
   * the memory may be client-side mappable or not.
   *
   * The old buffer may be passed as BufferCreateInfo::oldbuffer, however,
   * it's up to the implementation whether the object will be reused or
   * discarded and replaced by the new one.
   *
   * @param[in] bufferCreateInfo The valid BufferCreateInfo structure
   * @param[in] oldBuffer The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Buffer object
   */
  UniquePtr<Graphics::Buffer> CreateBuffer(const Graphics::BufferCreateInfo& bufferCreateInfo, UniquePtr<Graphics::Buffer>&& oldBuffer) override;

  /**
   * @brief Creates new CommandBufferImpl object
   *
   * @param[in] bufferCreateInfo The valid BufferCreateInfo structure
   * @param[in] oldCommandBuffer The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the CommandBufferImpl object
   */
  UniquePtr<Graphics::CommandBuffer> CreateCommandBuffer(const Graphics::CommandBufferCreateInfo& commandBufferCreateInfo, UniquePtr<Graphics::CommandBuffer>&& oldCommandBuffer) override;

  /**
   * @brief Creates new RenderPass object
   *
   * @param[in] renderPassCreateInfo The valid RenderPassCreateInfo structure
   * @param[in] oldRenderPass The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the RenderPass object
   */
  UniquePtr<Graphics::RenderPass> CreateRenderPass(const Graphics::RenderPassCreateInfo& renderPassCreateInfo, UniquePtr<Graphics::RenderPass>&& oldRenderPass) override;

  /**
   * @brief Creates new Texture object
   *
   * @param[in] textureCreateInfo The valid TextureCreateInfo structure
   * @param[in] oldTexture The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the TextureCreateInfo object
   */
  UniquePtr<Graphics::Texture> CreateTexture(const Graphics::TextureCreateInfo& textureCreateInfo, UniquePtr<Graphics::Texture>&& oldTexture) override;

  /**
   * @brief Creates new FramebufferImpl object
   *
   * @param[in] framebufferCreateInfo The valid FramebufferCreateInfo structure
   * @param[in] oldFramebuffer The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the FramebufferImpl object
   */
  UniquePtr<Graphics::Framebuffer> CreateFramebuffer(const Graphics::FramebufferCreateInfo& framebufferCreateInfo, UniquePtr<Graphics::Framebuffer>&& oldFramebuffer) override;

  /**
   * @brief Creates new Pipeline object
   *
   * @param[in] pipelineCreateInfo The valid PipelineCreateInfo structure
   * @param[in] oldPipeline The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Pipeline object
   */
  UniquePtr<Graphics::Pipeline> CreatePipeline(const Graphics::PipelineCreateInfo& pipelineCreateInfo, UniquePtr<Graphics::Pipeline>&& oldPipeline) override;

  /**
   * @brief Creates new Program object
   *
   * @param[in] ProgramCreateInfo The valid ProgramCreateInfo structure
   * @param[in] oldProgram The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Program object
   */
  UniquePtr<Graphics::Program> CreateProgram(const Graphics::ProgramCreateInfo& programCreateInfo, UniquePtr<Graphics::Program>&& oldProgram) override;

  /**
   * @brief Creates new Shader object
   *
   * @param[in] shaderCreateInfo The valid ShaderCreateInfo structure
   * @param[in] oldShader The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Shader object
   */
  UniquePtr<Graphics::Shader> CreateShader(const Graphics::ShaderCreateInfo& shaderCreateInfo, UniquePtr<Graphics::Shader>&& oldShader) override;

  /**
   * @brief Creates new Sampler object
   *
   * @param[in] samplerCreateInfo The valid SamplerCreateInfo structure
   * @param[in] oldSampler The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Sampler object
   */
  UniquePtr<Graphics::Sampler> CreateSampler(const Graphics::SamplerCreateInfo& samplerCreateInfo, UniquePtr<Graphics::Sampler>&& oldSampler) override;

  /**
   * @brief Creates new RenderTarget object
   *
   * @param[in] renderTargetCreateInfo The valid RenderTargetCreateInfo structure
   * @param[in] oldRenderTarget The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the RenderTarget object
   */
  UniquePtr<Graphics::RenderTarget> CreateRenderTarget(const Graphics::RenderTargetCreateInfo& renderTargetCreateInfo, UniquePtr<Graphics::RenderTarget>&& oldRenderTarget) override;

  /**
   * Create a synchronisation object.
   *
   * @return A pointer to an opaque sync object
   * @param[in] syncObjectCreateInfo The valid SyncObjectCreateInfo structure
   * @param[in] oldSyncObject The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   */
  UniquePtr<Graphics::SyncObject> CreateSyncObject(const Graphics::SyncObjectCreateInfo& syncObjectCreateInfo,
                                                   UniquePtr<Graphics::SyncObject>&&     oldSyncObject) override;

  /**
   * Add the graphics resource to a discard queue for later destruction
   * @param[in] resource The graphics resource to discard.
   */
  void DiscardResource(Vulkan::ResourceBase* resource);

  /**
   * @brief Maps memory associated with Buffer object
   *
   * @param[in] mapInfo Filled details of mapped resource
   * @return Returns pointer to Memory object or nullptr on error
   */
  UniquePtr<Graphics::Memory> MapBufferRange(const Graphics::MapBufferInfo& mapInfo) override;

  /**
   * @brief Maps memory associated with the texture.
   *
   * Only Texture objects that are backed with linear memory (staging memory) can be mapped.
   * Example:
   * 1) GLES implementation may create PBO object as staging memory and couple it
   * with the texture. Texture can be mapped and the memory can be read/write on demand.
   *
   * 2) Vulkan implementation may allocate DeviceMemory and use linear layout.
   *
   * @param[in] mapInfo Filled details of mapped resource
   *
   * @return Valid Memory object or nullptr on error
   */
  UniquePtr<Graphics::Memory> MapTextureRange(const Graphics::MapTextureInfo& mapInfo) override;

  /**
   * @brief Unmaps memory and discards Memory object
   *
   * This function automatically removes lock if Memory has been
   * previously locked.
   *
   * @param[in] memory Valid and previously mapped Memory object
   */
  void UnmapMemory(UniquePtr<Graphics::Memory> memory) override;

  /**
   * @brief Returns memory requirements of the Buffer object.
   *
   * Call this function whenever it's necessary to know how much memory
   * is needed to store all the buffer data and what memory alignment
   * the data should follow.
   *
   * @return Returns memory requirements of Buffer
   */
  Graphics::MemoryRequirements GetBufferMemoryRequirements(Graphics::Buffer& buffer) const override;

  /**
   * @brief Returns memory requirements of the Texture object.
   *
   * Call this function whenever it's necessary to know how much memory
   * is needed to store all the texture data and what memory alignment
   * the data should follow.
   *
   * @return Returns memory requirements of Texture
   */
  Graphics::MemoryRequirements GetTextureMemoryRequirements(Graphics::Texture& texture) const override;

  /**
   * @brief Returns specification of the Texture object
   *
   * Function obtains specification of the Texture object. It may retrieve
   * implementation dependent details like ie. whether the texture is
   * emulated (for example, RGB emulated on RGBA), compressed etc.
   *
   * @return Returns the TextureProperties object
   */
  Graphics::TextureProperties GetTextureProperties(const Graphics::Texture& texture) override;

  /**
   * @brief Returns the reflection of the given program
   *
   * @param[in] program The program
   * @return The reflection of the program
   */
  const Graphics::Reflection& GetProgramReflection(const Graphics::Program& program) override;

  /**
   * @brief Tests whether two Pipelines are the same.
   *
   * On the higher level, this function may help wit creating pipeline cache.
   *
   * @return true if pipeline objects match
   */
  [[nodiscard]] bool PipelineEquals(const Graphics::Pipeline& pipeline0, const Graphics::Pipeline& pipeline1) const override;

  /**
   * @brief Retrieves program parameters
   *
   * This function can be used to retrieve data from internal implementation
   *
   * @param[in] program Valid program object
   * @param[in] parameterId Integer parameter id
   * @param[out] outData Pointer to output memory
   * @return True on success
   */
  bool GetProgramParameter(Graphics::Program& program, uint32_t parameterId, void* outData) override;

  uint32_t GetDeviceLimitation(Graphics::DeviceCapability capability) override;

  bool IsCompatible(const Graphics::RenderTarget& gfxRenderTargetA, const Graphics::RenderTarget& gfxRenderTargetB, const Graphics::RenderPass& gfxRenderPassA, const Graphics::RenderPass& gfxRenderPassB) override;

  void UpdateRenderTarget(Graphics::RenderTarget& renderTarget, const Graphics::RenderTargetCreateInfo& renderTargetCreateInfo) override;

public: // Integration::GraphicsConfig
  bool        IsBlendEquationSupported(DevelBlendEquation::Type blendEquation) override;
  uint32_t    GetShaderLanguageVersion() override;
  std::string GetShaderVersionPrefix() override;

  std::string GetVertexShaderPrefix() override;

  std::string GetFragmentShaderPrefix() override;

public: // ResourceId relative API.
  /**
   * @brief Create Graphics::Texture as resourceId.
   * The ownership of Graphics::Texture will be hold on this controller.
   * @note If some Graphics::Texture already created before, assert.
   * @post DiscardTextureFromResourceId() or ReleaseTextureFromResourceId() should be called when we don't use resourceId texture anymore.
   *
   * @param[in] resourceId The unique id of resouces.
   * @return Pointer of Graphics::Texture, or nullptr if we fail to create.
   */
  Graphics::Texture* CreateTextureByResourceId(uint32_t resourceId, const Graphics::TextureCreateInfo& createInfo) override;

  /**
   * @brief Discard Graphics::Texture as resourceId.
   *
   * @param[in] resourceId The unique id of resouces.
   */
  void DiscardTextureFromResourceId(uint32_t resourceId) override;

  /**
   * @brief Get the Graphics::Texture as resourceId.
   *
   * @param[in] resourceId The unique id of resouces.
   * @return Pointer of Graphics::Texture, or nullptr if there is no valid objects.
   */
  Graphics::Texture* GetTextureFromResourceId(uint32_t resourceId) override;

  /**
   * @brief Get the ownership of Graphics::Texture as resourceId.
   *
   * @param[in] resourceId The unique id of resouces.
   * @return Pointer of Graphics::Texture.
   */
  UniquePtr<Graphics::Texture> ReleaseTextureFromResourceId(uint32_t resourceId) override;

  /**
   * @return true if there is a clip space transform matrix
   */
  bool HasClipMatrix() const override;

  /**
   * @return the clip space transform matrix
   */
  const Matrix& GetClipMatrix() const override;

public: // Other API
  /**
   * Adds the render-target's fbo attachments into the dependency graph.
   */
  void AddTextureDependencies(RenderTarget* renderTarget);

  /**
   * Check if any of the textures are fbo attachments and update dependency graph
   */
  void CheckTextureDependencies(const std::vector<Graphics::TextureBinding>& textureBindings,
                                RenderTarget*                                renderTarget);

  /**
   * Remove the render target from dependency graph
   */
  void RemoveRenderTarget(RenderTarget* renderTarget);

  /**
   * Inform controller when a frame is about to start. This indicates start of Render, before msg processing
   */
  void FrameStart();

  /**
   * Inform controller when rendering is about to start. This is called between Core::PreRender and Core::RenderScene.
   */
  void RenderStart();

  /**
   * @brief Reset the DidPresent flag to false at the start of each frame.
   * It will be set to true when the render target is presented.
   * More detail, at VulkanGraphicsController::PresentRenderTarget(), swapchain present successfully.
   */
  void ResetDidPresent();

  /**
   * @brief Check if the render target was presented in the last frame.
   * This is used to determine if the frame was successfully presented to the display.
   *
   * @return true if the render target was presented, false otherwise.
   */
  bool DidPresent() const;

  SamplerImpl* GetDefaultSampler();

  /**
   * Remove the texture array and destroy its resources. This should only be called after
   * the last ref has been removed.
   *
   * @param textureArray The texture to remove
   */
  void RemoveTextureArray(TextureArray* textureArray);

  /**
   * Check whether advanced blending is supported
   * @return Whether advanced blending is supported
   */
  bool IsAdvancedBlendEquationSupported();

  /**
   * Set the cached advanced blending support status
   * @param[in] isSupported Whether advanced blending is supported
   */
  void SetIsAdvancedBlendEquationSupported(bool isSupported);

  /**
   * Get the maximum texture size
   * @return Maximum texture size
   */
  uint32_t GetMaxTextureSize();

  /**
   * Get the maximum number of combined texture units
   * @return Maximum number of combined texture units
   */
  uint32_t GetMaxCombinedTextureUnits();

  /**
   * Notify that the logical device has been created
   * This should be called when Vulkan::Device::CreateDevice() completes
   */
  void NotifyLogicalDeviceCreated();

public: // For debug
  std::size_t GetCapacity() const;

private:
  /**
   * Flush all outstanding queues.
   */
  void Flush();

  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali
#endif // DALI_INTERNAL_GRAPHICS_VULKAN_CONTROLLER_IMPL_H
