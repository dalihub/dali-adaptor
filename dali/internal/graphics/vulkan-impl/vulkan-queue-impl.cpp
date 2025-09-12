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
 *
 */

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-queue-impl.h>

namespace Dali::Graphics::Vulkan
{
// submission
SubmissionData::SubmissionData(const std::vector<vk::Semaphore>&          waitSemaphores_,
                               const std::vector<vk::PipelineStageFlags>& waitDestinationStageMask_,
                               const std::vector<CommandBufferImpl*>&     commandBuffers_,
                               const std::vector<vk::Semaphore>&          signalSemaphores_)
: waitSemaphores(waitSemaphores_),
  waitDestinationStageMask(waitDestinationStageMask_),
  commandBuffers(commandBuffers_),
  signalSemaphores(signalSemaphores_)
{
}

SubmissionData& SubmissionData::SetWaitSemaphores(const std::vector<vk::Semaphore>& semaphores)
{
  waitSemaphores = semaphores;
  return *this;
}

SubmissionData& SubmissionData::SetWaitDestinationStageMask(const std::vector<vk::PipelineStageFlags>& dstStageMask)
{
  waitDestinationStageMask = dstStageMask;
  return *this;
}

SubmissionData& SubmissionData::SetCommandBuffers(const std::vector<CommandBufferImpl*>& cmdBuffers)
{
  commandBuffers = cmdBuffers;
  return *this;
}

SubmissionData& SubmissionData::SetSignalSemaphores(const std::vector<vk::Semaphore>& semaphores)
{
  signalSemaphores = semaphores;
  return *this;
}

// queue
Queue::Queue(vk::Queue      queue,
             uint32_t       queueFamilyIndex,
             uint32_t       queueIndex,
             vk::QueueFlags queueFlags)
: mQueue(queue),
  mFlags(queueFlags),
  mQueueFamilyIndex(queueFamilyIndex),
  mQueueIndex(queueIndex),
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
  return std::unique_ptr<std::lock_guard<std::recursive_mutex>>(new std::lock_guard<std::recursive_mutex>(mMutex));
}

vk::Result Queue::WaitIdle()
{
  return mQueue.waitIdle();
}

vk::Result Queue::Present(vk::PresentInfoKHR& presentInfo)
{
  return mQueue.presentKHR(&presentInfo);
}

vk::Result Queue::Submit(const std::vector<SubmissionData>& submissionData, FenceImpl* fence)
{
  auto lock(Lock());

  auto submitInfos = std::vector<vk::SubmitInfo>{};
  submitInfos.reserve(submissionData.size());
  auto commandBufferHandles = std::vector<vk::CommandBuffer>{};

  // prepare memory
  auto bufferSize = 0u;
  for(auto& data : submissionData)
  {
    bufferSize += uint32_t(data.commandBuffers.size());
  }
  commandBufferHandles.reserve(bufferSize);

  // Transform SubmissionData to vk::SubmitInfo
  for(const auto& subData : submissionData)
  {
    auto currentBufferIndex = commandBufferHandles.size();

    // Extract the command buffer handles
    std::transform(subData.commandBuffers.cbegin(),
                   subData.commandBuffers.cend(),
                   std::back_inserter(commandBufferHandles),
                   [&](CommandBufferImpl* entry)
    {
      return entry->GetVkHandle();
    });

    // clang-format=off
    auto submitInfo = vk::SubmitInfo()
                        .setWaitSemaphoreCount(U32(subData.waitSemaphores.size()))
                        .setPWaitSemaphores(subData.waitSemaphores.data())
                        .setPWaitDstStageMask(subData.waitDestinationStageMask.data())
                        .setCommandBufferCount(U32(subData.commandBuffers.size()))
                        .setPCommandBuffers(&commandBufferHandles[currentBufferIndex])
                        .setSignalSemaphoreCount(U32(subData.signalSemaphores.size()))
                        .setPSignalSemaphores(subData.signalSemaphores.data());

    submitInfos.push_back(submitInfo);
    // clang-format=on
  }

  return VkAssert(mQueue.submit(submitInfos, fence ? fence->GetVkHandle() : nullptr));
}

} // namespace Dali::Graphics::Vulkan
