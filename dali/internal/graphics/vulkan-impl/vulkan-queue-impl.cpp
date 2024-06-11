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

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-queue-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>

namespace Dali::Graphics::Vulkan
{

// submission
SubmissionData::SubmissionData( const std::vector<vk::Semaphore>& waitSemaphores_,
                                vk::PipelineStageFlags waitDestinationStageMask_,
                                const std::vector<CommandBufferImpl*>& commandBuffers_,
                                const std::vector<vk::Semaphore>& signalSemaphores_ )
        : waitSemaphores( waitSemaphores_ ),
          waitDestinationStageMask( waitDestinationStageMask_ ),
          commandBuffers( commandBuffers_ ),
          signalSemaphores( signalSemaphores_ )
{
}

SubmissionData& SubmissionData::SetWaitSemaphores( const std::vector< vk::Semaphore >& semaphores )
{
  waitSemaphores = semaphores;
  return *this;
}

SubmissionData& SubmissionData::SetWaitDestinationStageMask( vk::PipelineStageFlags dstStageMask )
{
  waitDestinationStageMask = dstStageMask;
  return *this;
}

SubmissionData& SubmissionData::SetCommandBuffers( const std::vector<CommandBufferImpl* >& cmdBuffers )
{
  commandBuffers = cmdBuffers;
  return *this;
}

SubmissionData& SubmissionData::SetSignalSemaphores( const std::vector< vk::Semaphore >& semaphores )
{
  signalSemaphores = semaphores;
  return *this;
}

// queue
Queue::Queue( vk::Queue queue,
              uint32_t queueFamilyIndex,
              uint32_t queueIndex,
              vk::QueueFlags queueFlags )
        : mQueue( queue ),
          mFlags( queueFlags ),
          mQueueFamilyIndex( queueFamilyIndex ),
          mQueueIndex( queueIndex ),
          mMutex()
{
}

Queue::~Queue() = default; // queues are non-destructible

vk::Queue Queue::GetVkHandle()
{
  return mQueue;
}

std::unique_ptr<std::lock_guard<std::recursive_mutex>> Queue::Lock()
{
  return std::unique_ptr<std::lock_guard<std::recursive_mutex>>( new std::lock_guard<std::recursive_mutex>( mMutex ) );
}

vk::Result Queue::WaitIdle()
{
  return mQueue.waitIdle();
}

vk::Result Queue::Present(vk::PresentInfoKHR& presentInfo)
{
  return mQueue.presentKHR(&presentInfo);
}

vk::Result Queue::Submit(std::vector<vk::SubmitInfo>& info, Fence* fence)
{
  return VkAssert(mQueue.submit(info, fence?fence->GetVkHandle():nullptr));
}

} // namespace Dali::Graphics::Vulkan
