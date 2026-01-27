#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_BUFFER_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_BUFFER_H

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

#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>

#include <dali/graphics-api/graphics-buffer-create-info.h>
#include <dali/graphics-api/graphics-buffer.h>

namespace Dali::Graphics::Vulkan
{
class BufferImpl;

using BufferResource = Resource<Graphics::Buffer, Graphics::BufferCreateInfo>;

class Buffer : public BufferResource
{
public:
  Buffer(const Graphics::BufferCreateInfo& createInfo, VulkanGraphicsController& controller);

  ~Buffer() override = default;

  /**
   * @copydoc Graphics::Vulkan::Resource::DestroyResource();
   */
  void DestroyResource() override;

  /**
   * @copydoc Graphics::Vulkan::Resource::InitializeResource();
   */
  InitializationResult InitializeResource() override;

  /**
   * @return false - Vulkan should always allocate GPU buffers
   */
  bool                IsCPUAllocated();
  [[nodiscard]] void* GetCPUAllocatedAddress() const;

  /**
   * @copydoc Graphics::Vulkan::Resource::DiscardResource();
   */
  void DiscardResource() override;

  bool TryRecycle(const Graphics::BufferCreateInfo& createInfo, VulkanGraphicsController& controller) override;

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
    this->~Buffer();
  }

  void Bind(Graphics::BufferUsage bindingTarget) const;

  BufferImpl* GetImpl()
  {
    if(!mCpuAllocated)
    {
      return mBufferImpl;
    }
    return nullptr;
  }

private:
  void InitializeCPUBuffer();
  void InitializeGPUBuffer();

private:
  union
  {
    BufferImpl* mBufferImpl{nullptr};
    uint8_t*    mBufferPtr;
  };
  bool mCpuAllocated{false};
  bool mTransient{false};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_BUFFER_H
