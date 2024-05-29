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

#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/integration-api/debug.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{

Fence::Fence( Device& graphicsDevice, vk::Fence handle )
: mGraphicsDevice( &graphicsDevice ),
  mFence(handle)
{
}

const Fence& Fence::ConstRef() const
{
  return *this;
}

Fence& Fence::Ref()
{
  return *this;
}

Fence::~Fence() = default;

vk::Fence Fence::GetVkHandle() const
{
  return mFence;
}

bool Fence::OnDestroy()
{
  // Copy the Vulkan handles and pointers here.
  // Cannot capture the "this" pointer in the lambda.
  // When the lambda is invoked "this" is already destroyed.
  // This method is only deferring execution to the end of the frame.
  auto device = mGraphicsDevice->GetLogicalDevice();
  auto fence = mFence;
  auto allocator = &mGraphicsDevice->GetAllocator();

  // capture copies of the pointers and handles
  mGraphicsDevice->DiscardResource( [ device, fence, allocator ]() {
    DALI_LOG_INFO( gVulkanFilter, Debug::General, "Invoking deleter function: fence->%p\n",
                   static_cast< VkFence >(fence) )
    device.destroyFence( fence, allocator );
  } );

  return false;
}

} // namespace Dali::Graphics::Vulkan
