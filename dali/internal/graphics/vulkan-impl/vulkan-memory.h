#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_MEMORY_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_MEMORY_H

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

#include <dali/graphics-api/graphics-memory.h>
#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-memory-impl.h>

namespace Dali::Graphics::Vulkan
{
class MemoryImpl;

/**
 *  This class is responsible for mapping and unmapping memory. It
 *  does not own the memory implementation (which belongs to a buffer/texture)
 */
class Memory : public Graphics::Memory
{
public:
  Memory(const Graphics::MapBufferInfo& mapInfo, VulkanGraphicsController& controller);
  Memory(const Graphics::MapTextureInfo& mapInfo, VulkanGraphicsController& controller);
  ~Memory() override;

  void Initialize(MemoryImpl* memoryImpl);

  /**
   * @copydoc Dali::Graphics::Memory::LockRegion()
   */
  void* LockRegion(uint32_t offset, uint32_t size) override;

  /**
   * @copydoc Dali::Graphics::Memory::Unlock()
   */
  void Unlock(bool flush) override;

  /**
   * @copydoc Dali::Graphics::Memory::Flush()
   */
  void Flush() override;

private:
  VulkanGraphicsController& mGraphicsController;
  enum class MapObjectType
  {
    NONE,
    BUFFER,
    TEXTURE
  };
  MapObjectType mMapObjectType{MapObjectType::NONE};
  union
  {
    Graphics::MapTextureInfo mMapTextureInfo{};
    Graphics::MapBufferInfo  mMapBufferInfo;
  };

  void*       mMappedPtr{nullptr};
  MemoryImpl* mMemoryImpl{nullptr};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_MEMORY_H
