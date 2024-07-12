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

#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>


namespace Dali::Graphics::Vulkan
{

class Memory
{
  friend class Device;

private:

  Memory( Device* graphicsDevice, vk::DeviceMemory deviceMemory, size_t memSize, size_t memAlign, bool hostVisible );

public:

  ~Memory();

  template<class T>
  T* MapTyped()
  {
    return reinterpret_cast<T*>(Map());
  }

  Memory(Memory&) = delete;
  Memory& operator=(Memory&) = delete;

  void* Map();

  void* Map( uint32_t offset, uint32_t size );

  void Unmap();

  void Flush();

  /**
   * Releases vk::DeviceMemory object so it can be deleted
   * externally
   * @return
   */
  vk::DeviceMemory ReleaseVkObject();

  [[nodiscard]] vk::DeviceMemory GetVkHandle() const;

private:
  Device* graphicsDevice;
  vk::DeviceMemory memory;
  size_t size;
  size_t alignment;
  void* mappedPtr;
  size_t mappedSize;
  bool hostVisible;
};

}//namespaces

#endif //DALI_INTERNAL_GRAPHICS_VULKAN_MEMORY_IMPL_H
