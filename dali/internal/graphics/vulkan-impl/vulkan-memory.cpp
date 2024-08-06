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

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-memory.h>

namespace Dali::Graphics::Vulkan
{
Memory::Memory(const Graphics::MapBufferInfo& mapInfo, VulkanGraphicsController& controller)
: mGraphicsController(controller)
{
  mMapBufferInfo = mapInfo;
  mMapObjectType = MapObjectType::BUFFER;
}

Memory::Memory(const Graphics::MapTextureInfo& mapInfo, VulkanGraphicsController& controller)
: mGraphicsController(controller)
{
  mMapTextureInfo = mapInfo;
  mMapObjectType  = MapObjectType::TEXTURE;
}

Memory::~Memory()
{
  Unlock(true);
  // this handle doesn't own memory associated with a buffer,
  // it's the buffer's responsibility to free it.
}

void Memory::Initialize(MemoryImpl* impl)
{
  mMemoryImpl = impl;
}

void* Memory::LockRegion(uint32_t offset, uint32_t size)
{
  if(mMapObjectType == MapObjectType::BUFFER)
  {
    auto buffer = static_cast<Vulkan::Buffer*>(mMapBufferInfo.buffer);
    if(buffer->IsCPUAllocated())
    {
      using Ptr  = uint8_t*;
      mMappedPtr = Ptr(buffer->GetCPUAllocatedAddress()) + offset;
    }
    else
    {
      mMappedPtr = mMemoryImpl->Map(offset, size);
    }
  }
  return mMappedPtr;
}

void Memory::Unlock(bool flush)
{
  if(mMapObjectType == MapObjectType::BUFFER)
  {
    auto buffer = static_cast<Vulkan::Buffer*>(mMapBufferInfo.buffer);
    if(!buffer->IsCPUAllocated() && mMemoryImpl)
    {
      mMemoryImpl->Unmap();
      if(flush)
      {
        Flush();
      }
    }
  }
  mMappedPtr = nullptr;
}

void Memory::Flush()
{
  if(mMemoryImpl)
  {
    mMemoryImpl->Flush();
  }
}

} // namespace Dali::Graphics::Vulkan
