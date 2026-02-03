#ifndef DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_H
#define DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <dali/graphics-api/graphics-command-buffer-create-info.h>
#include <dali/graphics-api/graphics-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class CommandBufferImpl;
class Swapchain;
class RenderTarget;
class StoredCommandPool;
class StoredCommandBuffer;

// Constants for blend state support
constexpr uint32_t MAX_COLOR_ATTACHMENTS = 4;

using CommandBufferResource = Resource<Graphics::CommandBuffer, Graphics::CommandBufferCreateInfo>;

/**
 * Implements Graphics::CommandBuffer, and is stored as a discardable resource.
 * Internally, this holds one or more vk:CommandBuffer wrappers (CommandBufferImpl);
 * allocated from a thread specific pool. The command buffer it's writing to is determined by
 * the Vulkan device's current buffer index.
 *
 * It may also hold a StoredCommandBuffer, in which case, all commands first get written
 * to the StoredCommandBuffer, and are transferred to the current vulkan command buffer during
 * submission by using the Process() call. This gives the backend the chance to modify
 * the command stream prior to submission (E.g. to insert barriers, etc).
 * A separate CommandBufferExecutor class is used to do the transfer.
 *
 * If Immediate mode is used, then there is no StoredCommandBuffer, and all commands
 * are written directly to the CommandBufferImpl and into the current vulkan command buffer.
 * (This is intended for use within the backend, for example, during resource transfer).
 *
 * Dynamic state is now handled in the CommandBufferImpl class.
 */
class CommandBuffer : public CommandBufferResource
{
public:
  enum class Storage
  {
    IMMEDIATE,
    STORED
  };

public:
  CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, VulkanGraphicsController& controller);
  CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, VulkanGraphicsController& controller, Storage storage, bool doubleBuffered);

  ~CommandBuffer() override;

  void Begin(const Graphics::CommandBufferBeginInfo& info) override;

  void End() override;

  void BindVertexBuffers(uint32_t                                    firstBinding,
                         const std::vector<const Graphics::Buffer*>& buffers,
                         const std::vector<uint32_t>&                offsets) override;

  /**
   * @brief Binds uniform buffers
   *
   * @param[in] bindings List of uniform buffer bindings
   */
  void BindUniformBuffers(const std::vector<UniformBufferBinding>& bindings) override;

  /**
   * @brief Binds pipeline
   *
   * @param[in] pipeline valid pipeline
   */
  void BindPipeline(const Graphics::Pipeline& pipeline) override;

  /**
   * @brief Binds textures
   *
   * @param[in] textureBindings List of texture bindings
   */
  void BindTextures(const std::vector<TextureBinding>& textureBindings) override;

  /**
   * @brief Binds samplers
   *
   * @param[in] samplerBindings List of sampler bindings
   */
  void BindSamplers(const std::vector<SamplerBinding>& samplerBindings) override;

  /**
   * @brief Binds buffer containing push constants
   *
   * @param[in] data pointer to the buffer
   * @param[in] size size of data in bytes
   * @param[in] binding push constants binding index
   */
  void BindPushConstants(void*    data,
                         uint32_t size,
                         uint32_t binding) override;

  /**
   * @brief Binds index buffer
   *
   * Most commonly used formats:
   * R32_UINT,
   * R16_UINT
   *
   * @param[in] buffer Valid buffer
   * @param[in] offset offset within buffer
   * @param[in] format Format of index buffer
   */
  void BindIndexBuffer(const Graphics::Buffer& buffer,
                       uint32_t                offset,
                       Format                  format) override;
  /**
   * @brief Begins render pass
   *
   * The function initialises rendering for specified RenderPass object
   * onto renderTarget. renderArea defines the scissor rect. Depends on the
   * renderPass spec, the clearValues may be used.
   *
   * Calling EndRenderPass() is necessary to finalize the render pass.
   *
   * @param[in] renderPass valid render pass object
   * @param[in] renderTarget valid render target, must not be used when framebuffer set
   * @param[in] renderArea area to draw (clear operation is affected)
   * @param[in] clearValues clear values (compatible with renderpass spec)
   */
  void BeginRenderPass(
    Graphics::RenderPass*          renderPass,
    Graphics::RenderTarget*        renderTarget,
    Rect2D                         renderArea,
    const std::vector<ClearValue>& clearValues) override;

  /**
   * @brief Ends current render pass
   *
   * This command must be issued in order to finalize the render pass.
   * It's up to the implementation whether anything has to be done but
   * the Controller may use end RP marker in order to resolve resource
   * dependencies (for example, to know when target texture is ready
   * before passing it to another render pass).
   *
   * The caller may query the sync object to determine when this render
   * pass has actually finished on the GPU.
   *
   * @param[in] syncObject If non-null, this object will ensure an
   * appropriate fence sync object is created after the render pass is
   * executed.
   */
  void EndRenderPass(Graphics::SyncObject* syncObject) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::ReadPixels
   */
  void ReadPixels(uint8_t* buffer) override;

  /**
   * @brief Executes a list of secondary command buffers
   *
   * The secondary command buffers will be executed as a part of a primary
   * command buffer that calls this function.
   *
   * @param[in] commandBuffers List of buffers to execute
   */
  void ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& commandBuffers) override;

  /**
   * @brief Draw primitives
   *
   * @param[in] vertexCount number of vertices
   * @param[in] instanceCount number of instances
   * @param[in] firstVertex index of first vertex
   * @param[in] firstInstance index of first instance
   */
  void Draw(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) override;

  /**
   * @brief Draws indexed primitives
   *
   * @param[in] indexCount Number of indices
   * @param[in] instanceCount Number of instances
   * @param[in] firstIndex first index
   * @param[in] vertexOffset offset of first vertex
   * @param[in] firstInstance first instance
   */
  void DrawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t  vertexOffset,
    uint32_t firstInstance) override;

  /**
   * @brief Draws indexed primitives indirectly
   *
   * Indirect draw uses additional buffer that holds render data.
   *
   * Indirect draw support depends on the hardware (most of modern hardware
   * supports this drawing technique).
   *
   * @param[in] buffer Buffer containing draw parameters
   * @param[in] offset Offset in bytes where parameters begin
   * @param[in] drawCount number of draws to execute
   * @param[in] stride stride between draw parameters
   */
  void DrawIndexedIndirect(
    Graphics::Buffer& buffer,
    uint32_t          offset,
    uint32_t          drawCount,
    uint32_t          stride) override;

  /**
   * @brief Draws using native API (via callback)
   *
   * DrawNative should be use in order to acquire direct access to the
   * graphics API like GL. Upon command execution, the backend will
   * invoke given callback and pass API-specific arguments (for example,
   * the GL callback will receive EGL context used for rendering).
   *
   * The client side must make sure the callback is valid for the
   * time of execution.
   *
   * @param[in] drawInfo NativeDrawInfo structure
   */
  void DrawNative(const DrawNativeInfo* drawInfo) override;

  /**
   * @brief Resets CommandBufferImpl
   *
   * This function resets the command buffer and discards all previously
   * recorded commands.
   *
   * Since the allocation may use internal memory pool of the CommandBufferImpl,
   * resetting doesn't have to discard all the resources (for example, it doesn't
   * need to destroy command but only move the pointer to the beginning of
   * the command buffer).
   *
   * It is useful if the command buffer has to be re-recorded frequently, for example,
   * every frame.
   */
  void Reset() override;

  /**
   * @brief Changes scissor rect
   *
   * @param[in] value 2D scissor rectangle
   */
  void SetScissor(Rect2D value) override;

  /**
   * @brief Enables/disables scissor test
   *
   * @param[in] value State of scissor test
   */
  void SetScissorTestEnable(bool value) override;

  /**
   * @brief Sets viewport
   *
   * @param[in] value 2D viewport area
   */
  void SetViewport(Viewport value) override;

  /**
   * @brief Sets whether the viewport should be changed
   * @param[in] value state of viewport
   */
  void SetViewportEnable(bool value) override;

  /**
   * @brief Sets the color mask for all channels.
   */
  void SetColorMask(bool enabled) override;

  /**
   * @brief Clears the stencil buffer (outside of BeginRenderPass) to the current stencil mask
   */
  void ClearStencilBuffer() override;

  /**
   * @brief Clears the depth buffer (outside of BeginRenderPass) to the current depth mask
   */
  void ClearDepthBuffer() override;

  /**
   * @brief Enable or disable the stencil test
   *
   * @param[in] stencilEnable whether stencil test should be enabled
   */
  void SetStencilTestEnable(bool stencilEnable) override;

  /**
   * @brief The mask used for writing to the stencil buffer.
   *
   * It should be as wide as necessary for the stencil texture format.
   * @param[in] writeMask The mask for wriing to / clearing the stencil buffer
   */
  void SetStencilWriteMask(uint32_t writeMask) override;

  /**
   * @brief Setup the stencil function and how subsequent draws will affect the stencil buffer.
   *
   * @param[in] failOp What happens to stencil buffer if drawing a pixel fails the stencil test
   * @param[in] passOp What happens to stencil buffer if drawing a pixel passes stencil & depth test
   * @param[in] depthFailOp What happens to stencil buffer if drawing a pixel passes stencil but fails depth test.
   * @param[in] compareOp How the stencil buffer, reference and compareMask are combined to determine whether to draw a pixel or not.
   * @param[in] reference A reference value that is ANDed with the mask in the compare op.
   * @param[in] compareMask The bitplanes from the stencil buffer that are active.
   */
  void SetStencilState(Graphics::CompareOp compareOp,
                       uint32_t            reference,
                       uint32_t            compareMask,
                       Graphics::StencilOp failOp,
                       Graphics::StencilOp passOp,
                       Graphics::StencilOp depthFailOp) override;

  /**
   * @brief Defines the comparison operator for passing the depth test.
   *
   * @param[in] compareOp The comparison operator
   */
  void SetDepthCompareOp(Graphics::CompareOp compareOp) override;

  /**
   * @brief Enables depth testing
   *
   * @param[in] depthTestEnable True if depth testing will be enabled.
   */
  void SetDepthTestEnable(bool depthTestEnable) override;

  /**
   * @brief Enables depth writing / clearing
   *
   * @param[in] depthWriteEnabled True if the depth buffer can be updated or cleared.
   */
  void SetDepthWriteEnable(bool depthWriteEnable) override;

  /**
   * @brief Enables or disables color blending for the specified attachment
   *
   * @param[in] attachment The color attachment index
   * @param[in] enabled True to enable blending, false to disable
   */
  void SetColorBlendEnable(uint32_t attachment, bool enabled) override;

  /**
   * @brief Sets the color blend equation for the specified attachment
   *
   * @param[in] attachment The color attachment index
   * @param[in] srcColorBlendFactor Source color blend factor
   * @param[in] dstColorBlendFactor Destination color blend factor
   * @param[in] colorBlendOp Color blend operation
   * @param[in] srcAlphaBlendFactor Source alpha blend factor
   * @param[in] dstAlphaBlendFactor Destination alpha blend factor
   * @param[in] alphaBlendOp Alpha blend operation
   */
  void SetColorBlendEquation(uint32_t              attachment,
                             Graphics::BlendFactor srcColorBlendFactor,
                             Graphics::BlendFactor dstColorBlendFactor,
                             Graphics::BlendOp     colorBlendOp,
                             Graphics::BlendFactor srcAlphaBlendFactor,
                             Graphics::BlendFactor dstAlphaBlendFactor,
                             Graphics::BlendOp     alphaBlendOp) override;

  /**
   * @brief Sets the advanced color blend equation for the specified attachment
   *
   * @param[in] attachment The color attachment index
   * @param[in] srcPremultiplied Source premultiplied
   * @param[in] dstPremultiplied Destination premultiplied
   * @param[in] blendOverlap Blend overlap
   * @param[in] blendOp Blend operation
   */
  void SetColorBlendAdvanced(uint32_t          attachment,
                             bool              srcPremultiplied,
                             bool              dstPremultiplied,
                             Graphics::BlendOp blendOp) override;

  /**
   * @brief Process the locally recorded command buffer into an actual vulkan command buffer
   * ready for submission.
   */
  void Process() const;

public: // VulkanResource API
  /**
   * @brief Called when GL resources are destroyed
   */
  void DestroyResource() override;

  /**
   * @brief Called when initializing the resource
   *
   * @return True on success
   */
  InitializationResult InitializeResource() override;

  /**
   * @brief Called when UniquePtr<> on client-side dies
   */
  void DiscardResource() override;

  /**
   * @copydoc Graphics::Vulkan::Resource::GetAllocationCallbacks()
   */
  [[nodiscard]] const Graphics::AllocationCallbacks* GetAllocationCallbacks() const override
  {
    return mCreateInfo.allocationCallbacks;
  }

  /**
   * @copydoc Graphics::Vulkan::Resource::InvokeDeleter()
   * Only intended for use by discard queue.
   */
  void InvokeDeleter() override
  {
    this->~CommandBuffer();
  }

public: // API
  /**
   * Get the last target referenced by a BeginRenderPass command in this command buffer.
   * Core now splits up command buffers so that they contain 1 render target each.
   */
  RenderTarget* GetRenderTarget() const;

  /**
   * Get the implementation according to the current buffer index.
   */
  [[nodiscard]] CommandBufferImpl* GetImpl() const;

  const StoredCommandBuffer* GetStoredCommandBuffer() const
  {
    return mStoredCommandBuffer.get();
  }

private:
  /**
   * Ensure that there are enough command buffers allocated.
   */
  void AllocateCommandBuffers(bool doubleBuffered);

  Storage                              mStorageType;
  std::unique_ptr<StoredCommandBuffer> mStoredCommandBuffer; ///< Copy of all cmds

  std::vector<CommandBufferImpl*> mCommandBufferImpl; ///< There are as many elements as there are swapchain images
  RenderTarget*                   mRenderTarget{nullptr};

  bool mDoubleBuffered{true};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_H
