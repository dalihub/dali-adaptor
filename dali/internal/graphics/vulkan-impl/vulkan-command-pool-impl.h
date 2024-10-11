#ifndef DALI_GRAPHICS_VULKAN_COMMAND_POOL_IMPL_H
#define DALI_GRAPHICS_VULKAN_COMMAND_POOL_IMPL_H

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

#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class Device;
class CommandBufferImpl;

class CommandPool : public VkManaged
{
public: // Construction, destruction
  /**
   *
   * @param graphics
   * @param createInfo
   * @return
   */
  static CommandPool* New(Device& graphics, const vk::CommandPoolCreateInfo& createInfo);

  /**
   *
   * @param graphics
   * @return
   */
  static CommandPool* New(Device& graphics);

  CommandPool() = delete;

  CommandPool(Device& graphicsDevice, const vk::CommandPoolCreateInfo& createInfo);

  ~CommandPool() override;

  vk::CommandPool GetVkHandle() const;

  Device& GetGraphicsDevice() const;

  bool Initialize();

  bool OnDestroy() override; // TODO: Queue deleter for destruction

  /**
   * Resets command pool
   */
  void Reset(bool releaseResources);

public: // API
  /**
   *
   * @param allocateInfo
   * @return
   */
  CommandBufferImpl* NewCommandBuffer(const vk::CommandBufferAllocateInfo& allocateInfo);

  /**
   *
   * @param isPrimary
   * @return
   */
  CommandBufferImpl* NewCommandBuffer(bool isPrimary = true);

  /**
   * Releases command buffer
   * @param buffer
   * @return
   */
  bool ReleaseCommandBuffer(CommandBufferImpl& buffer);

  /**
   * Returns current pool capacity ( 0 if nothing allocated )
   * @return
   */
  uint32_t GetCapacity() const;

  /**
   * Returns number of allocated command buffers
   * @return
   */
  uint32_t GetAllocationCount() const;

  /**
   * Returns number of allocated command buffers by level
   * @param level
   * @return
   */
  uint32_t GetAllocationCount(vk::CommandBufferLevel level) const;

  Device* GetGraphicsDevice()
  {
    return mGraphicsDevice;
  }

private: // Internal structs
  /**
   * CommandBufferPool contains preallocated command buffers that are
   * reusable.
   */
  struct InternalPool
  {
    static constexpr uint32_t INVALID_NODE_INDEX{0xffffffffu};

    struct Node
    {
      Node(uint32_t _nextFreeIndex, CommandBufferImpl* _commandBuffer);

      uint32_t           nextFreeIndex;
      CommandBufferImpl* commandBuffer;
    };

    InternalPool(CommandPool& owner, Device* graphics, uint32_t initialCapacity, bool isPrimary);

    ~InternalPool();

    /**
     * Creates new batch of command buffers
     * @param allocateInfo
     * @return
     */
    std::vector<vk::CommandBuffer> AllocateVkCommandBuffers(vk::CommandBufferAllocateInfo allocateInfo);

    /**
     * Resizes command pool to the new capacity. Pool may only grow
     * @param newCapacity
     */
    void Resize(uint32_t newCapacity);

    /**
     * Allocates new command buffer
     * @return
     */
    CommandBufferImpl* AllocateCommandBuffer(bool reset);

    /**
     * Releases command buffer back to the pool
     * @param reset if true, Resets command buffer
     * @param ref
     */
    void ReleaseCommandBuffer(CommandBufferImpl& buffer, bool reset = false);

    uint32_t GetCapacity() const;

    uint32_t GetAllocationCount() const;

    CommandPool&      mOwner;
    Device*           mGraphicsDevice;
    std::vector<Node> mPoolData;
    uint32_t          mFirstFree;
    uint32_t          mCapacity;
    uint32_t          mAllocationCount;
    bool              mIsPrimary;
  };

private: // Data members
  Device*                   mGraphicsDevice;
  vk::CommandPoolCreateInfo mCreateInfo;
  vk::CommandPool           mCommandPool;

  // Pools are lazily allocated, depends on the requested command buffers
  std::unique_ptr<InternalPool> mInternalPoolPrimary;
  std::unique_ptr<InternalPool> mInternalPoolSecondary;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_COMMAND_POOL_IMPL_H
