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

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>

#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
FenceImpl* FenceImpl::New(Device& graphicsDevice, const vk::FenceCreateInfo& fenceCreateInfo)
{
  auto fence = new FenceImpl(graphicsDevice);
  fence->Initialize(fenceCreateInfo);
  return fence;
}

FenceImpl::FenceImpl(Device& graphicsDevice)
: mGraphicsDevice(&graphicsDevice)
{
}

FenceImpl::~FenceImpl()
{
  Destroy();
}

vk::Fence FenceImpl::GetVkHandle() const
{
  return mFence;
}

void FenceImpl::Initialize(const vk::FenceCreateInfo& fenceCreateInfo)
{
  auto device = mGraphicsDevice->GetLogicalDevice();
  VkAssert(device.createFence(&fenceCreateInfo, &mGraphicsDevice->GetAllocator(), &mFence));
}

void FenceImpl::Destroy()
{
  mGraphicsDevice->GetLogicalDevice().destroyFence(mFence, &mGraphicsDevice->GetAllocator());
}

void FenceImpl::Reset()
{
  auto device = mGraphicsDevice->GetLogicalDevice();
  VkAssert(device.resetFences(1, &mFence));
}

void FenceImpl::Wait(uint32_t timeout)
{
  auto device = mGraphicsDevice->GetLogicalDevice();
  VkAssert(device.waitForFences(1, &mFence, VK_TRUE, timeout));
}

vk::Result FenceImpl::GetStatus()
{
  auto       device = mGraphicsDevice->GetLogicalDevice();
  vk::Result result = device.getFenceStatus(mFence);
  return result;
}

} // namespace Dali::Graphics::Vulkan
