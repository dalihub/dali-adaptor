#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_CONTROLLER_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_CONTROLLER_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

namespace Dali::Graphics
{
class VulkanGraphics;

namespace Vulkan
{
class Device;
}

class VulkanGraphicsController : public Dali::Graphics::Controller, Integration::GraphicsConfig
{
public:
  VulkanGraphicsController();
  ~VulkanGraphicsController() override;

  void Initialize(Dali::Graphics::VulkanGraphics& graphicsImplementation);

  [[nodiscard]] Vulkan::Device& GetGraphicsDevice() const;

  /**
   * Get graphics configuration for info about the graphics subsystem.
   */
  Integration::GraphicsConfig& GetGraphicsConfig() override;

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
  void PresentRenderTarget(RenderTarget* renderTarget) override;

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
  void GenerateTextureMipmaps(const Texture& texture) override;

  /**
   * @brief Enables depth/stencil buffer
   *
   * @param[in] enableDepth True to enable depth
   * @param[in] enableStencil True to enable stencil
   * @return True on success
   */
  bool EnableDepthStencilBuffer(bool enableDepth, bool enableStencil) override;

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
  UniquePtr<Buffer> CreateBuffer(const BufferCreateInfo& bufferCreateInfo, UniquePtr<Buffer>&& oldBuffer) override;

  /**
   * @brief Creates new CommandBuffer object
   *
   * @param[in] bufferCreateInfo The valid BufferCreateInfo structure
   * @param[in] oldCommandBuffer The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the CommandBuffer object
   */
  UniquePtr<CommandBuffer> CreateCommandBuffer(const CommandBufferCreateInfo& commandBufferCreateInfo, UniquePtr<CommandBuffer>&& oldCommandBuffer) override;

  /**
   * @brief Creates new RenderPass object
   *
   * @param[in] renderPassCreateInfo The valid RenderPassCreateInfo structure
   * @param[in] oldRenderPass The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the RenderPass object
   */
  UniquePtr<RenderPass> CreateRenderPass(const RenderPassCreateInfo& renderPassCreateInfo, UniquePtr<RenderPass>&& oldRenderPass) override;

  /**
   * @brief Creates new Texture object
   *
   * @param[in] textureCreateInfo The valid TextureCreateInfo structure
   * @param[in] oldTexture The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the TextureCreateInfo object
   */
  UniquePtr<Texture> CreateTexture(const TextureCreateInfo& textureCreateInfo, UniquePtr<Texture>&& oldTexture) override;

  /**
   * @brief Creates new Framebuffer object
   *
   * @param[in] framebufferCreateInfo The valid FramebufferCreateInfo structure
   * @param[in] oldFramebuffer The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Framebuffer object
   */
  UniquePtr<Framebuffer> CreateFramebuffer(const FramebufferCreateInfo& framebufferCreateInfo, UniquePtr<Framebuffer>&& oldFramebuffer) override;

  /**
   * @brief Creates new Pipeline object
   *
   * @param[in] pipelineCreateInfo The valid PipelineCreateInfo structure
   * @param[in] oldPipeline The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Pipeline object
   */
  UniquePtr<Pipeline> CreatePipeline(const PipelineCreateInfo& pipelineCreateInfo, UniquePtr<Pipeline>&& oldPipeline) override;

  /**
   * @brief Creates new Program object
   *
   * @param[in] ProgramCreateInfo The valid ProgramCreateInfo structure
   * @param[in] oldProgram The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Program object
   */
  UniquePtr<Program> CreateProgram(const ProgramCreateInfo& programCreateInfo, UniquePtr<Program>&& oldProgram) override;

  /**
   * @brief Creates new Shader object
   *
   * @param[in] shaderCreateInfo The valid ShaderCreateInfo structure
   * @param[in] oldShader The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Shader object
   */
  UniquePtr<Shader> CreateShader(const ShaderCreateInfo& shaderCreateInfo, UniquePtr<Shader>&& oldShader) override;

  /**
   * @brief Creates new Sampler object
   *
   * @param[in] samplerCreateInfo The valid SamplerCreateInfo structure
   * @param[in] oldSampler The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the Sampler object
   */
  UniquePtr<Sampler> CreateSampler(const SamplerCreateInfo& samplerCreateInfo, UniquePtr<Sampler>&& oldSampler) override;

  /**
   * @brief Creates new RenderTarget object
   *
   * @param[in] renderTargetCreateInfo The valid RenderTargetCreateInfo structure
   * @param[in] oldRenderTarget The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   * @return pointer to the RenderTarget object
   */
  UniquePtr<RenderTarget> CreateRenderTarget(const RenderTargetCreateInfo& renderTargetCreateInfo, UniquePtr<RenderTarget>&& oldRenderTarget) override;

  /**
   * Create a synchronisation object.
   *
   * @return A pointer to an opaque sync object
   * @param[in] syncObjectCreateInfo The valid SyncObjectCreateInfo structure
   * @param[in] oldSyncObject The valid pointer to the old object or nullptr. The object will be reused or destroyed.
   */
  UniquePtr<SyncObject> CreateSyncObject(const SyncObjectCreateInfo& syncObjectCreateInfo,
                                                 UniquePtr<SyncObject>&&     oldSyncObject) override;

  /**
   * @brief Maps memory associated with Buffer object
   *
   * @param[in] mapInfo Filled details of mapped resource
   * @return Returns pointer to Memory object or nullptr on error
   */
  UniquePtr<Memory> MapBufferRange(const MapBufferInfo& mapInfo) override;

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
  UniquePtr<Memory> MapTextureRange(const MapTextureInfo& mapInfo) override;

  /**
   * @brief Unmaps memory and discards Memory object
   *
   * This function automatically removes lock if Memory has been
   * previously locked.
   *
   * @param[in] memory Valid and previously mapped Memory object
   */
  void UnmapMemory(UniquePtr<Memory> memory) override;

  /**
   * @brief Returns memory requirements of the Texture object.
   *
   * Call this function whenever it's necessary to know how much memory
   * is needed to store all the texture data and what memory alignment
   * the data should follow.
   *
   * @return Returns memory requirements of Texture
   */
  MemoryRequirements GetTextureMemoryRequirements(Texture& texture) const override;

  /**
   * @brief Returns memory requirements of the Buffer object.
   *
   * Call this function whenever it's necessary to know how much memory
   * is needed to store all the buffer data and what memory alignment
   * the data should follow.
   *
   * @return Returns memory requirements of Buffer
   */
  MemoryRequirements GetBufferMemoryRequirements(Buffer& buffer) const override;

  /**
   * @brief Returns specification of the Texture object
   *
   * Function obtains specification of the Texture object. It may retrieve
   * implementation dependent details like ie. whether the texture is
   * emulated (for example, RGB emulated on RGBA), compressed etc.
   *
   * @return Returns the TextureProperties object
   */
  TextureProperties GetTextureProperties(const Texture& texture) override;

  /**
   * @brief Returns the reflection of the given program
   *
   * @param[in] program The program
   * @return The reflection of the program
   */
  const Reflection& GetProgramReflection(const Program& program) override;

  /**
   * @brief Tests whether two Pipelines are the same.
   *
   * On the higher level, this function may help wit creating pipeline cache.
   *
   * @return true if pipeline objects match
   */
  bool PipelineEquals(const Pipeline& pipeline0, const Pipeline& pipeline1) const override;

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

public: // Integration::GraphicsConfig

  //bool IsBlendEquationSupported(DevelBlendEquation::Type blendEquation) override;

  uint32_t GetShaderLanguageVersion() override;

  std::string GetShaderVersionPrefix() override;

  std::string GetVertexShaderPrefix() override;

  std::string GetFragmentShaderPrefix() override;

private:
  struct Impl;
  std::unique_ptr< Impl > mImpl;
};

}

#endif //DALI_INTERNAL_GRAPHICS_VULKAN_CONTROLLER_H
