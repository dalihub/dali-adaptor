#ifndef DALI_INTERNAL_NATIVE_IMAGE_QUEUE_PRODUCER_X_H
#define DALI_INTERNAL_NATIVE_IMAGE_QUEUE_PRODUCER_X_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-namespace.h>
#include <cstdint>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
/**
 * The producer side of NativeImageQueueX, which NativeImageSurfaceX drives.
 *
 * Elsewhere there is nothing to implement, because the graphics driver owns the
 * queue: it is handed to EGL as the native window, and dequeueing a buffer and
 * enqueueing the finished frame both happen inside eglSwapBuffers(). No EGL
 * here accepts a buffer queue as a native window, so the queue has to be driven
 * by hand instead. NativeImageSurfaceX obtains this interface from
 * Dali::NativeImageQueue::GetNativeImageQueue() and takes the driver's part: it
 * dequeues a buffer before the frame, and enqueues it once the frame has been
 * written into it.
 *
 * The consumer side of the queue runs on the DALi render thread and the producer
 * may run on any other thread, so implementations must be safe to call
 * concurrently with the consumer methods of NativeImageQueue.
 */
class NativeImageQueueProducerX
{
public:
  /**
   * The order the colour channels appear in within a buffer.
   *
   * The producer has to write the order the queue allocated for, and only the
   * queue knows which that is, so it travels with the buffer.
   */
  enum class ChannelOrder : uint8_t
  {
    RGBA, ///< Red, green, blue, alpha
    BGRA  ///< Blue, green, red, alpha
  };

  /**
   * A buffer handed to the producer, and the CPU mapping to write it through.
   */
  struct Buffer
  {
    uint8_t*     data{nullptr}; ///< CPU-writable mapping, or nullptr if nothing was available
    uint32_t     stride{0};     ///< Bytes per row, which may exceed the tightly packed row size
    uint32_t     width{0};      ///< Width in pixels
    uint32_t     height{0};     ///< Height in pixels
    int32_t      index{-1};     ///< Identifies the buffer back to the queue
    ChannelOrder channelOrder{ChannelOrder::RGBA};

    bool IsValid() const
    {
      return data != nullptr && index >= 0;
    }
  };

  /**
   * @brief Destructor.
   */
  virtual ~NativeImageQueueProducerX() = default;

  /**
   * @brief Whether a buffer is free to produce into.
   *
   * This is what NativeImageSurface::CanRender() reports.
   *
   * @return True if Dequeue() would return a valid buffer
   */
  virtual bool CanDequeue() = 0;

  /**
   * @brief Takes a free buffer to produce into.
   *
   * @return The buffer, or one whose IsValid() is false if none was free
   */
  virtual Buffer Dequeue() = 0;

  /**
   * @brief Publishes a produced buffer, making it the newest frame.
   *
   * Everything written through Buffer::data must be visible to the GPU by the
   * time this is called; the queue does no fencing of its own.
   *
   * @param[in] buffer A buffer previously returned by Dequeue()
   */
  virtual void Enqueue(const Buffer& buffer) = 0;

  /**
   * @brief Returns a dequeued buffer without publishing it.
   *
   * @param[in] buffer A buffer previously returned by Dequeue()
   */
  virtual void CancelDequeued(const Buffer& buffer) = 0;
};

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_NATIVE_IMAGE_QUEUE_PRODUCER_X_H
