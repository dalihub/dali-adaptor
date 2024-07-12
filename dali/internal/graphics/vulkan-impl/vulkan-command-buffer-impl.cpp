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

#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-swapchain-impl.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

CommandBufferImpl::CommandBufferImpl( CommandPool& commandPool,
                              uint32_t poolIndex,
                              const vk::CommandBufferAllocateInfo& allocateInfo,
                              vk::CommandBuffer vulkanHandle )
: mOwnerCommandPool( &commandPool ),
  mGraphicsDevice( mOwnerCommandPool->GetGraphicsDevice() ),
  mPoolAllocationIndex( poolIndex ),
  mAllocateInfo( allocateInfo ),
  mCommandBuffer( vulkanHandle )
{
}

CommandBufferImpl::~CommandBufferImpl() = default;

/** Begin recording */
void CommandBufferImpl::Begin( vk::CommandBufferUsageFlags usageFlags,
                           vk::CommandBufferInheritanceInfo* inheritanceInfo )
{
  assert( !mRecording && "CommandBufferImpl already is in the recording state" );
  auto info = vk::CommandBufferBeginInfo{};
  info.setPInheritanceInfo( inheritanceInfo );
  info.setFlags( usageFlags );

  VkAssert( mCommandBuffer.begin( info ) );

  mRecording = true;
}

/** Finish recording */
void CommandBufferImpl::End()
{
  assert( mRecording && "CommandBufferImpl is not in the recording state!" );
  VkAssert( mCommandBuffer.end() );
  mRecording = false;
}

/** Reset command buffer */
void CommandBufferImpl::Reset()
{
  assert( !mRecording && "Can't reset command buffer during recording!" );
  assert( mCommandBuffer && "Invalid command buffer!" );
  mCommandBuffer.reset( vk::CommandBufferResetFlagBits::eReleaseResources );
}

/** Free command buffer */
void CommandBufferImpl::Free()
{
  assert( mCommandBuffer && "Invalid command buffer!" );
  mGraphicsDevice->GetLogicalDevice().freeCommandBuffers( mOwnerCommandPool->GetVkHandle(), mCommandBuffer );
}

vk::CommandBuffer CommandBufferImpl::GetVkHandle() const
{
  return mCommandBuffer;
}

bool CommandBufferImpl::IsPrimary() const
{
  return mAllocateInfo.level == vk::CommandBufferLevel::ePrimary;
}

void CommandBufferImpl::BeginRenderPass( vk::RenderPassBeginInfo renderPassBeginInfo, vk::SubpassContents subpassContents )
{
  mCommandBuffer.beginRenderPass( renderPassBeginInfo, subpassContents );
}

void CommandBufferImpl::EndRenderPass()
{
  mCommandBuffer.endRenderPass();
}


uint32_t CommandBufferImpl::GetPoolAllocationIndex() const
{
  return mPoolAllocationIndex;
}

bool CommandBufferImpl::OnDestroy()
{
  mOwnerCommandPool->ReleaseCommandBuffer( *this );
  return true;
}

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali
