#ifndef DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_IMPL_H
#define DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_IMPL_H

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
 *
 */

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class Buffer;
class Device;
class CommandPool;
class PipelineImpl;

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
  void BindIndexBuffer(Vulkan::BufferImpl& buffer, uint32_t offset, Format format);
  void BindUniformBuffers(const std::vector<UniformBufferBinding>& bindings);
  void BindTextures(const std::vector<TextureBinding>& textureBindings);
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

  void PipelineBarrier(vk::PipelineStageFlags               srcStageMask,
                       vk::PipelineStageFlags               dstStageMask,
                       vk::DependencyFlags                  dependencyFlags,
                       std::vector<vk::MemoryBarrier>       memoryBarriers,
                       std::vector<vk::BufferMemoryBarrier> bufferBarriers,
                       std::vector<vk::ImageMemoryBarrier>  imageBarriers);

  void CopyBufferToImage(Vulkan::BufferImpl* srcBuffer, Vulkan::Image* dstImage, vk::ImageLayout dstLayout, const std::vector<vk::BufferImageCopy>& regions);

  void CopyImage(Vulkan::Image* srcImage, vk::ImageLayout srcLayout, Image* dstImage, vk::ImageLayout dstLayout, const std::vector<vk::ImageCopy>& regions);

  void SetScissor(Rect2D value);
  void SetViewport(Viewport value);

  void SetStencilTestEnable(bool stencilEnable);
  void SetStencilWriteMask(vk::StencilFaceFlags faceMask, uint32_t writeMask);
  void SetStencilCompareMask(vk::StencilFaceFlags faceMask, uint32_t compareMask);
  void SetStencilReference(vk::StencilFaceFlags faceMask, uint32_t reference);
  void SetStencilOp(vk::StencilFaceFlags faceMask, vk::StencilOp failOp, vk::StencilOp passOp, vk::StencilOp depthFailOp, vk::CompareOp compareOp);

  void SetDepthTestEnable(bool depthTestEnable);
  void SetDepthWriteEnable(bool depthWriteEnable);
  void SetDepthCompareOp(vk::CompareOp compareOp);

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

  void ExecuteCommandBuffers(std::vector<vk::CommandBuffer>& commandBuffers);

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

private:
  CommandPool*                        mOwnerCommandPool;
  Device*                             mGraphicsDevice;
  uint32_t                            mPoolAllocationIndex;
  vk::CommandBufferAllocateInfo       mAllocateInfo{};
  std::vector<DeferredTextureBinding> mDeferredTextureBindings;
  std::vector<DeferredUniformBinding> mDeferredUniformBindings;

  vk::CommandBuffer mCommandBuffer{};

  Vulkan::ProgramImpl* mCurrentProgram{nullptr}; /// To test descriptor sets, will collide with other work

  bool mRecording{false};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_IMPL_H
