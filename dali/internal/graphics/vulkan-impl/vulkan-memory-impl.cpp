/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/vulkan/vulkan-memory-allocation.h>

namespace Dali::Graphics::Vulkan
{
MemoryImpl::MemoryImpl(Device& device, vk::MemoryPropertyFlags memoryProperties, ::vma::Allocation* vmaAllocation)
: mDevice(device),
  deviceMemory(nullptr),
  offset(0u),
  mappedPtr(nullptr),
  mappedSize(0u),
  mMemoryProperties(memoryProperties),
  mVmaAllocation(vmaAllocation)
{
  ::vma::Allocator* vmaAllocator = mDevice.GetVulkanMemoryAllocator();
  if(vmaAllocator && mVmaAllocation)
  {
    ::vma::AllocationInfo allocationInfo;
    vmaAllocator->getAllocationInfo(*mVmaAllocation, &allocationInfo);

    deviceMemory = allocationInfo.deviceMemory;
    offset       = allocationInfo.offset;
    mappedPtr    = allocationInfo.pMappedData;
  }
}

MemoryImpl::~MemoryImpl()
{
}

void* MemoryImpl::Map()
{
  return Map(0u, 0u);
}

void* MemoryImpl::Map(uint32_t offset, uint32_t requestedMappedSize)
{
  if(!deviceMemory)
  {
    return nullptr;
  }

  if(mappedPtr)
  {
    return mappedPtr;
  }

  if(mVmaAllocation)
  {
    // When you map memory with VMA, it maps the entire allocation.
    auto vmaAllocator = mDevice.GetVulkanMemoryAllocator();
    if(vmaAllocator)
    {
      VkAssert(vmaAllocator->mapMemory(*mVmaAllocation, &mappedPtr));

      mappedPtr = static_cast<uint8_t*>(mappedPtr) + offset;
    }
  }

  mappedSize = requestedMappedSize;
  return mappedPtr;
}

void MemoryImpl::Unmap()
{
  if(deviceMemory && mappedPtr)
  {
    if(mVmaAllocation)
    {
      auto vmaAllocator = mDevice.GetVulkanMemoryAllocator();
      if(vmaAllocator)
      {
        vmaAllocator->unmapMemory(*mVmaAllocation);
      }
    }

    mappedPtr = nullptr;
  }
}

void MemoryImpl::Flush()
{
  // Don't flush if we are using host coherent memory - it's un-necessary
  if((mMemoryProperties & vk::MemoryPropertyFlagBits::eHostCoherent) != vk::MemoryPropertyFlagBits::eHostCoherent)
  {
    if(mVmaAllocation)
    {
      auto vmaAllocator = mDevice.GetVulkanMemoryAllocator();
      if(vmaAllocator)
      {
        vk::Result result = vmaAllocator->flushAllocation(*mVmaAllocation, 0u, mappedSize ? mappedSize : VK_WHOLE_SIZE);

        DALI_ASSERT_ALWAYS(result == vk::Result::eSuccess); // If it's out of memory, may as well crash.
      }
    }
  }
}

vk::DeviceMemory MemoryImpl::GetVkHandle() const
{
  return deviceMemory;
}

vk::DeviceSize MemoryImpl::GetOffset() const
{
  return offset;
}

} // namespace Dali::Graphics::Vulkan
