#ifndef DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_H
#define DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_H

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

#include <dali/graphics-api/graphics-command-buffer-create-info.h>
#include <dali/graphics-api/graphics-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>

namespace Dali::Graphics::Vulkan
{
class CommandBufferImpl;
class Swapchain;
class RenderTarget;

using CommandBufferResource = Resource<Graphics::CommandBuffer, Graphics::CommandBufferCreateInfo>;

namespace DynamicStateMaskBits
{
const uint32_t SCISSOR               = 1 << 0;
const uint32_t VIEWPORT              = 1 << 1;
const uint32_t STENCIL_TEST          = 1 << 2;
const uint32_t STENCIL_WRITE_MASK    = 1 << 3;
const uint32_t STENCIL_COMP_MASK     = 1 << 4;
const uint32_t STENCIL_REF           = 1 << 5;
const uint32_t STENCIL_OP_FAIL       = 1 << 6;
const uint32_t STENCIL_OP_DEPTH_FAIL = 1 << 7;
const uint32_t STENCIL_OP_PASS       = 1 << 8;
const uint32_t STENCIL_OP_COMP       = 1 << 9;
const uint32_t DEPTH_TEST            = 1 << 10;
const uint32_t DEPTH_WRITE           = 1 << 11;
const uint32_t DEPTH_OP_COMP         = 1 << 12;
}; // namespace DynamicStateMaskBits
using DynamicStateMask = uint32_t;

class CommandBuffer : public CommandBufferResource
{
public:
  CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, VulkanGraphicsController& controller);
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
  bool InitializeResource() override;

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

  [[nodiscard]] CommandBufferImpl* GetImpl() const;

private:
  static const DynamicStateMask INITIAL_DYNAMIC_MASK_VALUE{0xFFFFFFFF};

  /** Struct that defines the current state */
  struct DynamicState
  {
    Rect2D              scissor;
    Viewport            viewport;
    uint32_t            stencilWriteMask;
    uint32_t            stencilReference;
    uint32_t            stencilCompareMask;
    Graphics::CompareOp stencilCompareOp;
    Graphics::CompareOp depthCompareOp;
    Graphics::StencilOp stencilFailOp;
    Graphics::StencilOp stencilPassOp;
    Graphics::StencilOp stencilDepthFailOp;
    bool                stencilTest;
    bool                depthTest;
    bool                depthWrite;
  } mDynamicState;
  DynamicStateMask mDynamicStateMask{INITIAL_DYNAMIC_MASK_VALUE}; // If a bit is 1, next cmd will write, else check & write if different.

  template<typename ValueType>
  bool SetDynamicState(ValueType& oldValue, ValueType& newValue, uint32_t bit)
  {
    if(((mDynamicStateMask & bit) != 0) || oldValue != newValue)
    {
      oldValue = newValue;
      mDynamicStateMask &= ~bit;
      return true;
    }
    return false;
  }

  std::vector<CommandBufferImpl*> mCommandBufferImpl; ///< There are as many elements as there are swapchain images
  RenderTarget*                   mRenderTarget{nullptr};
  Swapchain*                      mLastSwapchain{nullptr};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_H
