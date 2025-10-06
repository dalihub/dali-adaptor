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

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>

#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-common.h>

// EXTERNAL INCLUDES

namespace Dali::Graphics::Vulkan
{
Buffer::Buffer(const Graphics::BufferCreateInfo& createInfo, VulkanGraphicsController& controller)
: BufferResource(createInfo, controller)
{
  // Check if buffer is CPU allocated
  if(((0 | BufferUsage::UNIFORM_BUFFER) & mCreateInfo.usage) &&
     (0 | BufferPropertiesFlagBit::CPU_ALLOCATED) & mCreateInfo.propertiesFlags)
  {
    // cpu allocated buffer
    DALI_ASSERT_ALWAYS(false && "Vulkan backend does not allow CPU allocated buffers");
    mCpuAllocated = true; //@todo Remove for vulkan? (it won't be used, so...)
  }

  // Check if buffer is transient
  if((0 | BufferPropertiesFlagBit::TRANSIENT_MEMORY) & mCreateInfo.propertiesFlags)
  {
    // memory is transient, may be lazily allocated when
    // mapped, together with cpu allocated it may create emulated uniform
    // buffer in this implementation
    mTransient = true;
  }
}

ResourceBase::InitializationResult Buffer::InitializeResource()
{
  if(mCpuAllocated && !mTransient)
  {
    InitializeCPUBuffer(); // @todo remove for vulkan?
  }
  else if(!mCpuAllocated)
  {
    InitializeGPUBuffer();
  }

  return InitializationResult::INITIALIZED;
}

void Buffer::InitializeCPUBuffer()
{
  // Just allocate memory
  const auto allocators = GetCreateInfo().allocationCallbacks;

  if(allocators)
  {
    mBufferPtr = static_cast<uint8_t*>(allocators->allocCallback(mCreateInfo.size, 0, allocators->userData));
  }
  else
  {
    mBufferPtr = static_cast<uint8_t*>(malloc(mCreateInfo.size));
    if(DALI_UNLIKELY(mBufferPtr == nullptr))
    {
      DALI_LOG_ERROR("malloc is failed. request malloc size : %u\n", mCreateInfo.size);
    }
  }
}

void Buffer::InitializeGPUBuffer()
{
  if(!mBufferImpl)
  {
    auto vkUsageFlags = static_cast<vk::BufferUsageFlags>(mCreateInfo.usage);
    mBufferImpl       = BufferImpl::New(mController.GetGraphicsDevice(), mCreateInfo.size, vkUsageFlags);
  }
}

bool Buffer::IsCPUAllocated()
{
  DALI_ASSERT_ALWAYS(!mCpuAllocated && "Vulkan backend does not use CPU allocated buffers");
  return mCpuAllocated;
}

void* Buffer::GetCPUAllocatedAddress() const
{
  return mBufferPtr;
}

void Buffer::DestroyResource()
{
  // Destroy CPU allocated buffer
  if(mCpuAllocated && mBufferPtr)
  {
    const auto allocators = GetCreateInfo().allocationCallbacks;
    if(allocators)
    {
      allocators->freeCallback(mBufferPtr, allocators->userData);
    }
    else
    {
      free(mBufferPtr);
    }
    mBufferPtr = nullptr;
  }
  else // Destroy GPU allocation
  {
    delete(mBufferImpl);
    mBufferImpl = nullptr;
  }
}

void Buffer::DiscardResource()
{
  mController.DiscardResource(this);
}

void Buffer::Bind(Graphics::BufferUsage bindingTarget) const
{
  if(!mCpuAllocated && mBufferImpl)
  {
    switch(bindingTarget)
    {
      case Graphics::BufferUsage::VERTEX_BUFFER:
      {
        break;
      }
      case Graphics::BufferUsage::INDEX_BUFFER:
      {
        break;
      }
      default:
      {
        // Nothing to do
      }
    }
  }
}

} // namespace Dali::Graphics::Vulkan
