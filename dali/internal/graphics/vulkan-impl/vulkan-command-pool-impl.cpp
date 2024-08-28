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
 *
 */

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
/**
 *
 * Struct: InternalPool
 */
CommandPool::InternalPool::Node::Node(uint32_t _nextFreeIndex, CommandBufferImpl* _commandBuffer)
: nextFreeIndex(_nextFreeIndex),
  commandBuffer(_commandBuffer)
{
}

CommandPool::InternalPool::InternalPool(CommandPool& owner, Vulkan::Device* graphics, uint32_t initialCapacity, bool isPrimary)
: mOwner(owner),
  mGraphicsDevice(graphics),
  mPoolData{},
  mFirstFree(INVALID_NODE_INDEX),
  mCapacity(initialCapacity),
  mAllocationCount(0u),
  mIsPrimary(isPrimary)
{
  // don't allocate anything if initial capacity is 0
  if(initialCapacity)
  {
    Resize(initialCapacity);
  }
}

CommandPool::InternalPool::~InternalPool()
{
  // free all buffers here
  for(auto&& cmd : mPoolData)
  {
    delete cmd.commandBuffer;
  }
}

std::vector<vk::CommandBuffer>
CommandPool::InternalPool::AllocateVkCommandBuffers(vk::CommandBufferAllocateInfo allocateInfo)
{
  return VkAssert(mGraphicsDevice->GetLogicalDevice().allocateCommandBuffers(allocateInfo));
}

void CommandPool::InternalPool::Resize(uint32_t newCapacity)
{
  if(newCapacity <= mPoolData.size())
  {
    return;
  }

  auto diff = newCapacity - mPoolData.size();

  auto allocateInfo = vk::CommandBufferAllocateInfo{}
                        .setCommandBufferCount(U32(diff))
                        .setCommandPool(mOwner.GetVkHandle())
                        .setLevel(mIsPrimary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary);
  auto newBuffers = AllocateVkCommandBuffers(allocateInfo);

  uint32_t i = U32(mPoolData.size() + 1);

  mFirstFree = U32(mPoolData.size());
  if(!mPoolData.empty())
  {
    mPoolData.back()
      .nextFreeIndex = U32(mPoolData.size());
  }
  for(auto&& cmdbuf : newBuffers)
  {
    auto commandBuffer = new CommandBufferImpl(mOwner, i - 1, allocateInfo, cmdbuf);
    mPoolData.emplace_back(i, commandBuffer);
    ++i;
  }
  mPoolData.back().nextFreeIndex = INVALID_NODE_INDEX;
  mCapacity                      = U32(mPoolData.size());
}

CommandBufferImpl* CommandPool::InternalPool::AllocateCommandBuffer(bool reset)
{
  // resize if no more nodes
  if(mFirstFree == INVALID_NODE_INDEX)
  {
    auto newSize = static_cast<uint32_t>(mPoolData.empty() ? 1 : 2 * mPoolData.size());
    Resize(U32(newSize));
  }

  auto& node = mPoolData[mFirstFree];
  mFirstFree = node.nextFreeIndex;

  if(reset)
  {
    node.commandBuffer->Reset();
  }

  ++mAllocationCount;
  return node.commandBuffer;
}

void CommandPool::InternalPool::ReleaseCommandBuffer(CommandBufferImpl& buffer, bool reset)
{
  auto indexInPool                     = buffer.GetPoolAllocationIndex();
  mPoolData[indexInPool].nextFreeIndex = mFirstFree;
  mFirstFree                           = indexInPool;

  if(reset)
  {
    buffer.Reset();
  }
  --mAllocationCount;
}

uint32_t CommandPool::InternalPool::GetCapacity() const
{
  return mCapacity;
}

uint32_t CommandPool::InternalPool::GetAllocationCount() const
{
  return mAllocationCount;
}

CommandPool* CommandPool::New(Device& graphics, const vk::CommandPoolCreateInfo& createInfo)
{
  auto pool = new CommandPool(graphics, createInfo);

  if(pool)
  {
    pool->Initialize();
  }

  return pool;
}

CommandPool* CommandPool::New(Device& graphics)
{
  return New(graphics, vk::CommandPoolCreateInfo{});
}

bool CommandPool::Initialize()
{
  mCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
  mCommandPool           = VkAssert(mGraphicsDevice->GetLogicalDevice().createCommandPool(mCreateInfo, mGraphicsDevice->GetAllocator()));
  mInternalPoolPrimary   = std::make_unique<InternalPool>(*this, mGraphicsDevice, 0, true);
  mInternalPoolSecondary = std::make_unique<InternalPool>(*this, mGraphicsDevice, 0, false);
  return true;
}

CommandPool::CommandPool(Device& graphics, const vk::CommandPoolCreateInfo& createInfo)
: mGraphicsDevice(&graphics),
  mCreateInfo(createInfo)
{
}

CommandPool::~CommandPool()
{
  Destroy();
}

vk::CommandPool CommandPool::GetVkHandle() const
{
  return mCommandPool;
}

Device& CommandPool::GetGraphicsDevice() const
{
  return *mGraphicsDevice;
}

CommandBufferImpl* CommandPool::NewCommandBuffer(const vk::CommandBufferAllocateInfo& allocateInfo)
{
  return NewCommandBuffer(allocateInfo.level == vk::CommandBufferLevel::ePrimary);
}

CommandBufferImpl* CommandPool::NewCommandBuffer(bool isPrimary)
{
  auto& usedPool = isPrimary ? *mInternalPoolPrimary : *mInternalPoolSecondary;
  return usedPool.AllocateCommandBuffer(false);
}

void CommandPool::Reset(bool releaseResources)
{
  mGraphicsDevice->GetLogicalDevice()
    .resetCommandPool(mCommandPool,
                      releaseResources ? vk::CommandPoolResetFlagBits::eReleaseResources
                                       : vk::CommandPoolResetFlags{});
}

bool CommandPool::ReleaseCommandBuffer(CommandBufferImpl& buffer)
{
  if(buffer.IsPrimary() && mInternalPoolPrimary)
  {
    mInternalPoolPrimary->ReleaseCommandBuffer(buffer);
  }
  else if(mInternalPoolSecondary)
  {
    mInternalPoolSecondary->ReleaseCommandBuffer(buffer);
  }
  return false;
}

uint32_t CommandPool::GetCapacity() const
{
  return mInternalPoolPrimary->GetCapacity() +
         mInternalPoolSecondary->GetCapacity();
}

uint32_t CommandPool::GetAllocationCount() const
{
  return mInternalPoolPrimary->GetAllocationCount() +
         mInternalPoolSecondary->GetAllocationCount();
}

uint32_t CommandPool::GetAllocationCount(vk::CommandBufferLevel level) const
{
  return level == vk::CommandBufferLevel::ePrimary ? mInternalPoolPrimary->GetAllocationCount() : mInternalPoolSecondary->GetAllocationCount();
}

void CommandPool::Destroy()
{
  auto device    = mGraphicsDevice->GetLogicalDevice();
  auto allocator = &mGraphicsDevice->GetAllocator();

  if(mCommandPool)
  {
    DALI_LOG_INFO(gVulkanFilter, Debug::General, "Destroying command pool: %p\n", static_cast<VkCommandPool>(mCommandPool));
    device.destroyCommandPool(mCommandPool, allocator);

    mCommandPool = nullptr;
  }
}

} // namespace Dali::Graphics::Vulkan
