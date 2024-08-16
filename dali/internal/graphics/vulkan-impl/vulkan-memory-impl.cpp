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

MemoryImpl::MemoryImpl(Device& device, size_t memSize, size_t memAlign, bool isHostVisible)
: mDevice(device),
  deviceMemory(nullptr),
  size(memSize),
  alignment(memAlign),
  mappedPtr(nullptr),
  mappedSize(0u),
  hostVisible(isHostVisible)
{
}

vk::Result MemoryImpl::Allocate(vk::MemoryAllocateInfo allocateInfo, const vk::AllocationCallbacks& allocator)
{
  auto result = mDevice.GetLogicalDevice().allocateMemory(&allocateInfo, &allocator, &deviceMemory);
  return result;
}

MemoryImpl::~MemoryImpl()
{
  // free memory
  if(deviceMemory)
  {
    auto device    = mDevice.GetLogicalDevice();
    auto allocator = &mDevice.GetAllocator();
    device.freeMemory(deviceMemory, allocator);
    deviceMemory = nullptr;
  }
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
  mappedPtr  = mDevice.GetLogicalDevice().mapMemory(deviceMemory, offset, requestedMappedSize ? requestedMappedSize : VK_WHOLE_SIZE).value;
  mappedSize = requestedMappedSize;
  return mappedPtr;
}

void MemoryImpl::Unmap()
{
  if(deviceMemory && mappedPtr)
  {
    mDevice.GetLogicalDevice().unmapMemory(deviceMemory);
    mappedPtr = nullptr;
  }
}

vk::DeviceMemory MemoryImpl::ReleaseVkObject()
{
  auto retval  = deviceMemory;
  deviceMemory = nullptr;
  return retval;
}

void MemoryImpl::Flush()
{
  vk::Result result = mDevice.GetLogicalDevice().flushMappedMemoryRanges({vk::MappedMemoryRange{}
                                                                            .setSize(mappedSize)
                                                                            .setMemory(deviceMemory)
                                                                            .setOffset(0u)});
  DALI_ASSERT_ALWAYS(result == vk::Result::eSuccess); // If it's out of memory, may as well crash.
}

vk::DeviceMemory MemoryImpl::GetVkHandle() const
{
  return deviceMemory;
}

} // namespace Dali::Graphics::Vulkan
