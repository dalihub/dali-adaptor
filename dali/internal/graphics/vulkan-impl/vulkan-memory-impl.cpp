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

#include <dali/internal/graphics/vulkan-impl/vulkan-memory-impl.h>

namespace Dali::Graphics::Vulkan
{

Memory::Memory( Device* _graphicsDevice, vk::DeviceMemory deviceMemory, size_t memSize, size_t memAlign, bool isHostVisible )
: graphicsDevice( _graphicsDevice ),
  memory( deviceMemory ),
  size( memSize ),
  alignment( memAlign ),
  mappedPtr( nullptr ),
  mappedSize( 0u ),
  hostVisible( isHostVisible )
{
}

Memory::~Memory()
{
  // free memory
  if( memory )
  {
    auto device = graphicsDevice->GetLogicalDevice();
    auto allocator = &graphicsDevice->GetAllocator();
    auto deviceMemory = memory;

    // Discard unused descriptor set layouts
    graphicsDevice->DiscardResource( [ device, deviceMemory, allocator ]() {
      // free memory
      device.freeMemory( deviceMemory, allocator );
    } );
  }
}

void* Memory::Map( uint32_t offset, uint32_t requestedMappedSize )
{
  if( !memory )
  {
    return nullptr;
  }

  if( mappedPtr )
  {
    return mappedPtr;
  }
  mappedPtr = graphicsDevice->GetLogicalDevice().mapMemory( memory, offset, requestedMappedSize ? requestedMappedSize : VK_WHOLE_SIZE ).value;
  mappedSize = requestedMappedSize;
  return mappedPtr;
}

void* Memory::Map()
{
  return Map( 0u, 0u );
}

void Memory::Unmap()
{
  if( memory && mappedPtr )
  {
    graphicsDevice->GetLogicalDevice().unmapMemory( memory );
    mappedPtr = nullptr;
  }
}

vk::DeviceMemory Memory::ReleaseVkObject()
{
  auto retval = memory;
  memory = nullptr;
  return retval;
}

void Memory::Flush()
{
  vk::Result result = graphicsDevice->GetLogicalDevice().flushMappedMemoryRanges( { vk::MappedMemoryRange{}
    .setSize( mappedSize )
    .setMemory( memory )
    .setOffset( 0u )
  } );
  DALI_ASSERT_ALWAYS(result == vk::Result::eSuccess); // If it's out of memory, may as well crash.
}

vk::DeviceMemory Memory::GetVkHandle() const
{
  return memory;
}

} //namespace Dali::Graphics::Vulkan
