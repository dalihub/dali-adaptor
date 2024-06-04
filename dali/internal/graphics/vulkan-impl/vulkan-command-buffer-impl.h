#ifndef DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_IMPL_H
#define DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_IMPL_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/graphics-api/graphics-types.h>

namespace Dali::Graphics::Vulkan
{
class Device;

class CommandBufferImpl : public VkManaged
{
  friend class CommandPool;

  friend struct CommandBufferPool;

public:
  CommandBufferImpl() = delete;

  ~CommandBufferImpl() override;

  /** Begin recording */
  void Begin( vk::CommandBufferUsageFlags usageFlags, vk::CommandBufferInheritanceInfo* inheritanceInfo );

  /** Finish recording */
  void End();

  /** Reset command buffer */
  void Reset();

  /** Free command buffer */
  void Free();

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
  void BeginRenderPass( vk::RenderPassBeginInfo renderPassBeginInfo, vk::SubpassContents subpassContents );

  /**
   * Ends current render pass
   */
  void EndRenderPass();

  /**
   * Implements VkManaged::OnDestroy
   * @return
   */
  bool OnDestroy() override;

private:

  /**
   * Returns allocation index
   * @return
   */
  [[nodiscard]] uint32_t GetPoolAllocationIndex() const;

private:

  // Constructor called by the CommandPool only
  CommandBufferImpl( CommandPool& commandPool,
                 uint32_t poolIndex,
                 const vk::CommandBufferAllocateInfo& allocateInfo,
                 vk::CommandBuffer vulkanHandle );

private:

  CommandPool* mOwnerCommandPool;
  Device* mGraphicsDevice;
  uint32_t mPoolAllocationIndex;
  vk::CommandBufferAllocateInfo mAllocateInfo{};

  vk::CommandBuffer mCommandBuffer{};

  bool mRecording{ false };
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_IMPL_H
