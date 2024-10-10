#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_MEMORY_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_MEMORY_IMPL_H

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
#include <dali/internal/graphics/vulkan/vulkan-device.h>

namespace Dali::Graphics::Vulkan
{
class MemoryImpl
{
public:
  MemoryImpl(Device& device, size_t memSize, size_t memAlign, vk::MemoryPropertyFlags memoryProperties);

  ~MemoryImpl();

  vk::Result Allocate(vk::MemoryAllocateInfo, const vk::AllocationCallbacks& allocator);

  template<class T>
  T* MapTyped()
  {
    return reinterpret_cast<T*>(Map());
  }

  // No copy constructor or assignment operator
  MemoryImpl(MemoryImpl&)            = delete;
  MemoryImpl& operator=(MemoryImpl&) = delete;

  void* Map();

  void* Map(uint32_t offset, uint32_t size);

  void Unmap();

  void Flush();

  [[nodiscard]] vk::DeviceMemory GetVkHandle() const;

private:
  Device&                 mDevice;
  vk::DeviceMemory        deviceMemory;
  size_t                  size;
  size_t                  alignment;
  void*                   mappedPtr;
  size_t                  mappedSize;
  vk::MemoryPropertyFlags mMemoryProperties;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_MEMORY_IMPL_H
