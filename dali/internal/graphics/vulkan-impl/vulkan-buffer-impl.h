#pragma once
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

#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-memory-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <cstdint>

namespace Dali::Graphics::Vulkan
{
class BufferImpl
{
public:
  static BufferImpl* New(Vulkan::Device& device, size_t size, vk::BufferUsageFlags usageFlags);

  static BufferImpl* New(Vulkan::Device& device, size_t size, vk::SharingMode sharingMode, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryProperties);

  /** Destructor */
  ~BufferImpl();

  /**
   * Returns buffer usage flags
   * @return
   */
  vk::BufferUsageFlags GetUsage() const
  {
    return mInfo.usage;
  }

  /**
   * Returns buffer size in bytes
   * @return
   */
  uint32_t GetSize() const
  {
    return static_cast<uint32_t>(mInfo.size);
  }

  /**
   * Returns Vulkan VkBuffer object associated with this buffer
   * @return
   */
  vk::Buffer GetVkHandle() const
  {
    return mBuffer;
  }

  /**
   * Returns handle to the allocated memory
   *
   * @todo replace with LockRegion
   *
   * @return
   */
  MemoryImpl* GetMemory() const
  {
    return mMemory.get();
  }

  /**
   * Destroys underlying Vulkan resources on the caller thread.
   *
   * @note Calling this function is unsafe and makes any further use of
   * buffer invalid.
   */
  void Destroy();

  BufferImpl(const Buffer&)            = delete;
  BufferImpl& operator=(const Buffer&) = delete;

  /**
   * Get the memory requirements for this buffer.
   */
  Graphics::MemoryRequirements GetMemoryRequirements();

private:
  /**
   * Creates new VkBuffer with given specification, it doesn't
   * bind the memory.
   * @param graphics
   * @param createInfo
   */
  BufferImpl(Device& graphicsDevice, const vk::BufferCreateInfo& createInfo);

  /**
   * Initializes buffer resources
   */
  void Initialize(vk::MemoryPropertyFlags memoryProperties);

  /**
   * Destroys used Vulkan resource objects
   * @param device Vulkan device
   * @param buffer Vulkan buffer
   * @param memory Vulkan device memory
   * @param allocator Pointer to the Vulkan allocator callbacks
   */
  static void DestroyVulkanResources(vk::Device device, vk::Buffer buffer, vk::DeviceMemory memory, const vk::AllocationCallbacks* allocator);

private:
  Device&                     mDevice;
  std::unique_ptr<MemoryImpl> mMemory;
  vk::BufferCreateInfo        mInfo;
  vk::Buffer                  mBuffer;
};

} // namespace Dali::Graphics::Vulkan
