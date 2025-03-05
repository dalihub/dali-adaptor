#ifndef VULKAN_DISCARD_QUEUE_H
#define VULKAN_DISCARD_QUEUE_H

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
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <memory>
#include <queue>

namespace Dali::Graphics::Vulkan
{
class Device;

/**
 * @brief DiscardQueues holds a growable number of queues to hold
 * resource data until it's no longer in use and can safely be
 * discarded.
 *
 * The number of discard queues corresponds to the number of swapchain
 * images that can be drawn to, and isn't known at creation time.
 * Instead, the vector of queues grows to match the bufferIndex of the
 * current frame.
 * In practice, this is either 2 or 3.
 *
 * @tparam ResourceType The resource type for this discard queue
 */
template<class ResourceType>
class DiscardQueues
{
public:
  DiscardQueues()
  : mDevice(nullptr),
    mQueues()
  {
    // Nothing to do.
  }

  /**
   * Initialize the object.
   * @param[in] device The vulkan device
   */
  void Initialize(Device& device)
  {
    mDevice = &device;
  }

  using DiscardQueue               = std::queue<ResourceType*>;
  using OwnedDiscardQueue          = std::unique_ptr<DiscardQueue>;
  using OwnedDiscardQueueContainer = std::vector<OwnedDiscardQueue>;

  /**
   * Check if we need to increase the number of queues.
   *
   * If we do need more queues, resize the vector, but don't allocate until
   * we need to discard resources into it.
   *
   * @param[in] bufferCount The current number of swapchain buffers
   */
  void Resize(uint32_t bufferCount)
  {
    if(mQueues.size() < bufferCount)
    {
      mQueues.resize(bufferCount);
    }
  }

  /**
   * Discard the resource - add to this frame's queue.
   * If there is no queue for this buffer index, create a queue.
   *
   * @param[in] resource The resource to discard
   */
  void Discard(ResourceType* resource)
  {
    DALI_ASSERT_DEBUG(mDevice && "No graphics device available");
    uint32_t bufferIndex = mDevice->GetCurrentBufferIndex();

    // Ensure there is a discard queue for this buffer index
    if(mQueues.size() < bufferIndex || !mQueues[bufferIndex])
    {
      for(auto i = 0u; i <= bufferIndex; ++i)
      {
        if(!mQueues[i])
        {
          mQueues[i] = std::make_unique<std::queue<ResourceType*>>();
        }
      }
    }
    mQueues[bufferIndex]->push(resource);
  }

  /**
   * @brief Processes a discard queue for objects created with NewObject
   *
   * @param[in] bufferIndex The buffer index of the oldest queue
   */
  void Process(uint32_t bufferIndex)
  {
    if(bufferIndex < mQueues.size())
    {
      auto& queue = mQueues[bufferIndex];
      while(queue && !queue->empty())
      {
        auto* object = const_cast<ResourceType*>(queue->front());

        // Destroy the resource
        object->DestroyResource();

        // If there are allocation callbacks, then invoke the deleter and
        // execute the callback.
        auto* allocationCallbacks = object->GetAllocationCallbacks();
        if(allocationCallbacks)
        {
          object->InvokeDeleter();
          allocationCallbacks->freeCallback(object, allocationCallbacks->userData);
        }
        else
        {
          // Otherwise, just delete the object.
          delete object;
        }
        queue->pop();
      }
    }
  }

  /**
   * Check if the discard queue is empty.
   *
   * @param bufferIndex The buffer index of the current frame
   * @return true if the discard queue is empty for this frame
   */
  bool IsEmpty(uint32_t bufferIndex)
  {
    if(bufferIndex < mQueues.size())
    {
      return mQueues[bufferIndex]->empty();
    }
    return true;
  }

private:
  Device*                    mDevice;
  OwnedDiscardQueueContainer mQueues;
};

} // namespace Dali::Graphics::Vulkan

#endif // VULKAN_DISCARD_QUEUE_H
