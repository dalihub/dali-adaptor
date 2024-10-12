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

#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>

#include <dali/integration-api/debug.h>

#include <cstdint>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
BufferImpl* BufferImpl::New(Device& device, size_t size, vk::BufferUsageFlags usageFlags)
{
  return New(device, size, vk::SharingMode(vk::SharingMode::eExclusive), usageFlags, vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
}

BufferImpl* BufferImpl::New(Device& device, size_t size, vk::SharingMode sharingMode, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryProperties)
{
  auto info = vk::BufferCreateInfo{};
  info.setSharingMode(sharingMode);
  info.setSize(size);
  info.setUsage(usageFlags);

  auto bufferImpl = new BufferImpl(device, info);

  VkAssert(device.GetLogicalDevice().createBuffer(&info, &device.GetAllocator(), &bufferImpl->mBuffer));

  bufferImpl->Initialize(memoryProperties);

  return bufferImpl;
}

BufferImpl::BufferImpl(Device& device, const vk::BufferCreateInfo& createInfo)
: mDevice(device),
  mMemory(nullptr),
  mInfo(createInfo)
{
}

void BufferImpl::Initialize(vk::MemoryPropertyFlags memoryProperties)
{
  // Allocate
  auto requirements    = mDevice.GetLogicalDevice().getBufferMemoryRequirements(mBuffer);
  auto memoryTypeIndex = Device::GetMemoryIndex(mDevice.GetMemoryProperties(),
                                                requirements.memoryTypeBits,
                                                memoryProperties);

  mMemory = std::make_unique<MemoryImpl>(mDevice, size_t(requirements.size), size_t(requirements.alignment), memoryProperties);

  auto allocateInfo = vk::MemoryAllocateInfo{}
                        .setMemoryTypeIndex(memoryTypeIndex)
                        .setAllocationSize(requirements.size);

  auto result = mMemory->Allocate(allocateInfo, mDevice.GetAllocator("DEVICEMEMORY"));

  if(result != vk::Result::eSuccess)
  {
    DALI_LOG_INFO(gVulkanFilter, Debug::General, "Unable to allocate memory for the buffer of size %d!", int(requirements.size));

    mMemory = nullptr;
  }

  // Bind
  if(mMemory)
  {
    VkAssert(mDevice.GetLogicalDevice().bindBufferMemory(mBuffer, mMemory->GetVkHandle(), 0));
  }
}

void BufferImpl::DestroyNow()
{
  DestroyVulkanResources(mDevice.GetLogicalDevice(), mBuffer, mMemory->ReleaseVkObject(), &mDevice.GetAllocator());
  mBuffer = nullptr;
  mMemory = nullptr;
}

void BufferImpl::DestroyVulkanResources(vk::Device device, vk::Buffer buffer, vk::DeviceMemory memory, const vk::AllocationCallbacks* allocator)
{
  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Invoking deleter function: buffer->%p\n", static_cast<VkBuffer>(buffer));

  device.destroyBuffer(buffer, allocator);
  device.freeMemory(memory, allocator);
}

Graphics::MemoryRequirements BufferImpl::GetMemoryRequirements()
{
  auto                         requirements = mDevice.GetLogicalDevice().getBufferMemoryRequirements(mBuffer);
  Graphics::MemoryRequirements memoryRequirements;
  memoryRequirements.size      = requirements.size;
  memoryRequirements.alignment = requirements.alignment;
  return memoryRequirements;
}

} // namespace Dali::Graphics::Vulkan
