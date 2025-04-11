#ifndef DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_IMPL_H
#define DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_IMPL_H

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
 *
 */

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-stored-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

// Vulkan headers for function pointer types
#include <vulkan/vulkan.h>

namespace Dali::Graphics::Vulkan
{
class Buffer;
class Device;
class CommandPool;
class PipelineImpl;

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
const uint32_t COLOR_WRITE_MASK      = 1 << 13;
const uint32_t COLOR_BLEND_ENABLE    = 1 << 14;
const uint32_t COLOR_BLEND_EQUATION  = 1 << 15;
}; // namespace DynamicStateMaskBits
using DynamicStateMask = uint32_t;

/**
 * Wraps direct access to vulkan command buffer.
 */
class CommandBufferImpl
{
  friend class CommandPool;

  friend struct CommandBufferPool;

public:
  CommandBufferImpl() = delete;

  ~CommandBufferImpl();

  void Destroy();

  /** Begin recording */
  void Begin(vk::CommandBufferUsageFlags usageFlags, vk::CommandBufferInheritanceInfo* inheritanceInfo);

  /** Finish recording */
  void End();

  /** Reset command buffer */
  void Reset();

  /** Free command buffer */
  void Free();

  /** Binds Vulkan pipeline */
  void BindPipeline(const Graphics::Pipeline* pipeline);

  /** Final validation of the pipeline */
  void ValidatePipeline();

  void BindVertexBuffers(
    uint32_t                                firstBinding,
    const std::vector<Vulkan::BufferImpl*>& buffers,
    const std::vector<uint32_t>&            offsets);

  void BindVertexBuffers(
    uint32_t                                          firstBinding,
    const IndirectPtr<VertexBufferBindingDescriptor>& bindingPtr,
    uint32_t                                          bindingCount);

  void BindIndexBuffer(Vulkan::BufferImpl& buffer, uint32_t offset, Format format);

  void BindUniformBuffers(const std::vector<UniformBufferBinding>& bindings);
  void BindUniformBuffers(const IndirectPtr<UniformBufferBindingDescriptor>& bindings, uint32_t count);
  void BindUniformBuffer(const UniformBufferBinding& binding);

  void BindTextures(const std::vector<TextureBinding>& textureBindings);
  void BindTextures(const IndirectPtr<TextureBinding>& textureBindingPtr, uint32_t count);
  bool BindTexture(const TextureBinding& textureBinding);

  void BindSamplers(const std::vector<SamplerBinding>& samplerBindings);

  /** Returns Vulkan object associated with the buffer */
  [[nodiscard]] vk::CommandBuffer GetVkHandle() const;

  /**
   * Tests if the command buffer is primary
   * @return Returns true if the command buffer is primary
   */
  [[nodiscard]] bool IsPrimary() const;
  /**
   * Allows to issue custom VkRenderPassBeginInfo structure
   * @param renderPassBeginInfo
   * @param subpassContents
   */
  void BeginRenderPass(vk::RenderPassBeginInfo renderPassBeginInfo, vk::SubpassContents subpassContents);

  /**
   * Ends current render pass
   */
  void EndRenderPass();

  /**
   * Request to read pixels.
   */
  void ReadPixels(uint8_t* buffer);

  void PipelineBarrier(vk::PipelineStageFlags                      srcStageMask,
                       vk::PipelineStageFlags                      dstStageMask,
                       vk::DependencyFlags                         dependencyFlags,
                       const std::vector<vk::MemoryBarrier>&       memoryBarriers,
                       const std::vector<vk::BufferMemoryBarrier>& bufferBarriers,
                       const std::vector<vk::ImageMemoryBarrier>&  imageBarriers);

  void CopyBufferToImage(Vulkan::BufferImpl* srcBuffer, Vulkan::Image* dstImage, vk::ImageLayout dstLayout, const std::vector<vk::BufferImageCopy>& regions);

  void CopyImage(Vulkan::Image* srcImage, vk::ImageLayout srcLayout, Image* dstImage, vk::ImageLayout dstLayout, const std::vector<vk::ImageCopy>& regions);

  void SetScissor(Rect2D value);
  void SetViewport(Viewport value);

  void SetStencilTestEnable(bool stencilEnable);
  void SetStencilWriteMask(uint32_t writeMask);
  void SetStencilCompareMask(uint32_t compareMask);
  void SetStencilReference(uint32_t reference);
  void SetStencilOp(Graphics::StencilOp failOp, Graphics::StencilOp passOp, Graphics::StencilOp depthFailOp, Graphics::CompareOp compareOp);

  void SetDepthTestEnable(bool depthTestEnable);
  void SetDepthWriteEnable(bool depthWriteEnable);
  void SetDepthCompareOp(Graphics::CompareOp compareOp);

  void SetColorMask(bool colorWriteMask);
  void SetColorBlendEnable(uint32_t attachment, bool enabled);
  void SetColorBlendEquation(uint32_t attachment, const Dali::Graphics::Vulkan::ColorBlendEquation& equation);
  void SetColorBlendAdvanced(uint32_t attachment, bool srcPremultiplied, bool dstPremultiplied, BlendOp blendOp);

  void ResolveDeferredPipelineBinding();

  void ApplyDeferredColorBlendStates();

  void PrepareForDraw();

  void Draw(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance);

  void DrawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t  vertexOffset,
    uint32_t firstInstance);

  void DrawIndexedIndirect(
    BufferImpl& buffer,
    uint32_t    offset,
    uint32_t    drawCount,
    uint32_t    stride);

private:
  /**
   * Returns allocation index
   * @return
   */
  [[nodiscard]] uint32_t GetPoolAllocationIndex() const;

  /**
   * Bind all deferred resources before drawing
   */
  void BindResources(vk::DescriptorSet set);

  /**
   * @brief Updates a descriptor set with specific resources
   * @param[in] descriptorSet The descriptor set to update
   */
  void UpdateDescriptorSet(vk::DescriptorSet descriptorSet);

private:
  // Constructor called by the CommandPool only
  CommandBufferImpl(
    CommandPool&                         commandPool,
    uint32_t                             poolIndex,
    const vk::CommandBufferAllocateInfo& allocateInfo,
    vk::CommandBuffer                    vulkanHandle);

private: // Struct for deferring texture binding
  struct DeferredTextureBinding
  {
    vk::ImageView imageView;
    vk::Sampler   sampler;
    uint32_t      binding;
  };
  struct DeferredUniformBinding
  {
    vk::Buffer buffer;
    uint32_t   offset;
    uint32_t   range;
    uint32_t   binding;
  };

  // Struct for deferring color blend state
  struct DeferredColorBlendState
  {
    uint32_t           attachment{0};
    bool               enableSet{false};
    bool               enable{false};
    bool               equationSet{false};
    ColorBlendEquation equation{};
    bool               advancedSet{false};
    bool               srcPremultiplied{false};
    bool               dstPremultiplied{false};
    BlendOp            blendOp{};
  };

private: // Struct for managing current state
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
    bool                colorWriteMask{true};
    bool                colorBlendEnable{false};
    ColorBlendEquation  colorBlendEquation{};
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
  template<typename ValueType>
  bool SetDynamicState(ValueType& oldValue, const ValueType& newValue, uint32_t bit)
  {
    if(((mDynamicStateMask & bit) != 0) || oldValue != newValue)
    {
      oldValue = newValue;
      mDynamicStateMask &= ~bit;
      return true;
    }
    return false;
  }

private:
  CommandPool*                                mOwnerCommandPool;
  Device*                                     mGraphicsDevice;
  uint32_t                                    mPoolAllocationIndex;
  vk::CommandBufferAllocateInfo               mAllocateInfo{};
  std::vector<DeferredTextureBinding>         mDeferredTextureBindings;
  std::vector<DeferredUniformBinding>         mDeferredUniformBindings;
  std::vector<DeferredColorBlendState>        mDeferredColorBlendStates;
  IndirectPtr<UniformBufferBindingDescriptor> mDeferredUniformBindingDescriptor;
  uint32_t                                    mDeferredUniformBindingDescriptorCount;

  // Deferred pipeline to bind if dynamic states not supported
  Vulkan::Pipeline* mDeferredPipelineToBind{nullptr};

  vk::Pipeline mLastBoundPipeline{}; /// The last bound pipeline

  // Dynamic depth/stencil states for deferred pipeline binding if API < 1.3
  // TODO: check API version
  vk::PipelineDepthStencilStateCreateInfo mDepthStencilState{};
  vk::StencilOpState                      mStencilTestStates[3]; /// 0 - unused, we can avoid branching
  vk::StencilOpState&                     mStencilTestFrontState{mStencilTestStates[1]};
  vk::StencilOpState&                     mStencilTestBackState{mStencilTestStates[2]};

  bool mColorWriteMask{true};

  vk::CommandBuffer mCommandBuffer{};

  Vulkan::ProgramImpl* mCurrentProgram{nullptr}; /// To test descriptor sets, will collide with other work

  bool mRecording{false};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_IMPL_H
