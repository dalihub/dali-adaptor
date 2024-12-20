#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_QUEUE_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_QUEUE_IMPL_H

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
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

#include <mutex>

namespace Dali::Graphics::Vulkan
{
class CommandBufferImpl;
class FenceImpl;
class Device;

struct SubmissionData
{
  SubmissionData() = default;

  explicit SubmissionData(const std::vector<vk::Semaphore>&          waitSemaphores_,
                          const std::vector<vk::PipelineStageFlags>& waitDestinationStageMask_,
                          const std::vector<CommandBufferImpl*>&     commandBuffers_,
                          const std::vector<vk::Semaphore>&          signalSemaphores_);

  SubmissionData& SetWaitSemaphores(const std::vector<vk::Semaphore>& semaphores);

  SubmissionData& SetWaitDestinationStageMask(const std::vector<vk::PipelineStageFlags>& dstStageMask);

  SubmissionData& SetCommandBuffers(const std::vector<CommandBufferImpl*>& cmdBuffers);

  SubmissionData& SetSignalSemaphores(const std::vector<vk::Semaphore>& semaphores);

  std::vector<vk::Semaphore>          waitSemaphores;
  std::vector<vk::PipelineStageFlags> waitDestinationStageMask;
  std::vector<CommandBufferImpl*>     commandBuffers;
  std::vector<vk::Semaphore>          signalSemaphores;
};

class Queue
{
public:
  Queue(vk::Queue      queue,
        uint32_t       queueFamilyIndex,
        uint32_t       queueIndex,
        vk::QueueFlags queueFlags);

  ~Queue(); // queues are non-destructible

  vk::Queue GetVkHandle();

  std::unique_ptr<std::lock_guard<std::recursive_mutex>> Lock();

  vk::Result WaitIdle();

  vk::Result Present(vk::PresentInfoKHR& presentInfo);

  vk::Result Submit(const std::vector<SubmissionData>& submissionData, FenceImpl* fence);

private:
  vk::Queue      mQueue;
  vk::QueueFlags mFlags;
  uint32_t       mQueueFamilyIndex;
  uint32_t       mQueueIndex;

  std::recursive_mutex mMutex;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_QUEUE_IMPL_H
