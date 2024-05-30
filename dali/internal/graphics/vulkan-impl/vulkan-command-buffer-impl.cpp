/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

CommandBuffer::CommandBuffer( CommandPool& commandPool,
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

CommandBuffer::~CommandBuffer() = default;

/** Begin recording */
void CommandBuffer::Begin( vk::CommandBufferUsageFlags usageFlags,
                           vk::CommandBufferInheritanceInfo* inheritanceInfo )
{
  assert( !mRecording && "CommandBuffer already is in the recording state" );
  auto info = vk::CommandBufferBeginInfo{};
  info.setPInheritanceInfo( inheritanceInfo );
  info.setFlags( usageFlags );

  VkAssert( mCommandBuffer.begin( info ) );

  mRecording = true;
}

/** Finish recording */
void CommandBuffer::End()
{
  assert( mRecording && "CommandBuffer is not in the recording state!" );
  VkAssert( mCommandBuffer.end() );
  mRecording = false;
}

/** Reset command buffer */
void CommandBuffer::Reset()
{
  assert( !mRecording && "Can't reset command buffer during recording!" );
  assert( mCommandBuffer && "Invalid command buffer!" );
  mCommandBuffer.reset( vk::CommandBufferResetFlagBits::eReleaseResources );
}

/** Free command buffer */
void CommandBuffer::Free()
{
  assert( mCommandBuffer && "Invalid command buffer!" );
  mGraphicsDevice->GetLogicalDevice().freeCommandBuffers( mOwnerCommandPool->GetVkHandle(), mCommandBuffer );
}

vk::CommandBuffer CommandBuffer::GetVkHandle() const
{
  return mCommandBuffer;
}

bool CommandBuffer::IsPrimary() const
{
  return mAllocateInfo.level == vk::CommandBufferLevel::ePrimary;
}

void CommandBuffer::BeginRenderPass( Graphics::FramebufferId framebufferId, uint32_t bufferIndex )
{
  auto swapchain = mGraphicsDevice->GetSwapchainForFramebuffer( 0u );
  auto surface = mGraphicsDevice->GetSurface( 0u );
  auto frameBuffer = swapchain->GetCurrentFramebuffer();
  auto renderPass = frameBuffer->GetRenderPass();
  auto clearValues = frameBuffer->GetClearValues();

  auto info = vk::RenderPassBeginInfo{};
  info.setFramebuffer( frameBuffer->GetVkHandle() );
  info.setRenderPass( renderPass );
  info.setClearValueCount( U32( clearValues.size() ) );
  info.setPClearValues( clearValues.data() );
  info.setRenderArea( vk::Rect2D( { 0, 0 }, surface->GetCapabilities().currentExtent ) );

  mCommandBuffer.beginRenderPass( info, vk::SubpassContents::eInline );
}

void CommandBuffer::BeginRenderPass( vk::RenderPassBeginInfo renderPassBeginInfo, vk::SubpassContents subpassContents )
{
  mCommandBuffer.beginRenderPass( renderPassBeginInfo, subpassContents );
}

void CommandBuffer::EndRenderPass()
{
  mCommandBuffer.endRenderPass();
}


uint32_t CommandBuffer::GetPoolAllocationIndex() const
{
  return mPoolAllocationIndex;
}

bool CommandBuffer::OnDestroy()
{
  mOwnerCommandPool->ReleaseCommandBuffer( *this );
  return true;
}

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali
