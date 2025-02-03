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

#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-memory-allocation.h>

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

  bufferImpl->Initialize(memoryProperties);

  return bufferImpl;
}

BufferImpl::BufferImpl(Device& device, const vk::BufferCreateInfo& createInfo)
: mDevice(device),
  mMemory(nullptr),
  mInfo(createInfo)
{
}

BufferImpl::~BufferImpl()
{
  Destroy();
}

void BufferImpl::Initialize(vk::MemoryPropertyFlags memoryProperties)
{
  auto vmaAllocator = mDevice.GetVulkanMemoryAllocator();
  if(vmaAllocator)
  {
    // Create buffer with memory allocated and bound
    auto vmaAllocInfo = ::vma::AllocationCreateInfo{}
                          .setPreferredFlags(memoryProperties)
                          .setFlags(::vma::AllocationCreateFlagBits::eHostAccessSequentialWrite)
                          .setUsage(::vma::MemoryUsage::eAuto);

    mVmaAllocation = std::make_unique<::vma::Allocation>();

    // This creates the buffer, allocates appropriate memory for it, and binds the buffer with the memory.
    ::vma::AllocationInfo allocationInfo;
    VkAssert(vmaAllocator->createBuffer(&mInfo, &vmaAllocInfo, &mBuffer, mVmaAllocation.get(), &allocationInfo));

    // Wrap the allocated memory so that we can map and unmap it.
    mMemory = std::make_unique<MemoryImpl>(mDevice, memoryProperties, mVmaAllocation.get());
  }
}

void BufferImpl::Destroy()
{
  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Destroying buffer: %p\n", static_cast<VkBuffer>(mBuffer));

  auto vmaAllocator = mDevice.GetVulkanMemoryAllocator();
  if(vmaAllocator && mVmaAllocation)
  {
    vmaAllocator->destroyBuffer(mBuffer, *mVmaAllocation);
    mVmaAllocation.reset(nullptr);
  }

  mBuffer = nullptr;
  mMemory.reset();
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
