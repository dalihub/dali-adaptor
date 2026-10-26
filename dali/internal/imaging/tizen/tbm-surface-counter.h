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

#ifndef DALI_TBM_SURFACE_COUNTER_H
#define DALI_TBM_SURFACE_COUNTER_H

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <atomic>
#include <cstdint>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{

/**
 * @brief Global counter for tracking TBM surface usage across the system
 *
 * This class provides thread-safe counting of:
 * - NativeImage instances (1 tbm_surface each)
 * - NativeImageQueue instances (multiple tbm_surfaces each)
 */
class TbmSurfaceCounter
{
public:
  /**
   * @brief Get singleton instance
   * @return Reference to the singleton instance
   */
  static TbmSurfaceCounter& GetInstance()
  {
    // Trivially destructible, so it is never destroyed at process exit.
    // Native images held in static storage can still use it when they are destroyed.
    static TbmSurfaceCounter instance;
    return instance;
  }

  /**
   * @brief Increment count when a NativeImage is created
   */
  void AddNativeImage()
  {
    ++mNativeImageCount;
    LogTotalCount();
  }

  /**
   * @brief Decrement count when a NativeImage is destroyed
   */
  void RemoveNativeImage()
  {
    Decrease(mNativeImageCount, 1u);
    LogTotalCount();
  }

  /**
   * @brief Increment count when a BackBuffer of NativeImage is created
   */
  void AddBackBufferSurface()
  {
    ++mBackBufferCount;
    LogTotalCount();
  }

  /**
   * @brief Decrement count when a BackBuffer of NativeImage is destroyed
   */
  void RemoveBackBufferSurface()
  {
    Decrease(mBackBufferCount, 1u);
    LogTotalCount();
  }

  /**
   * @brief Increment count when a NativeImageQueue is created
   * @param queueSize Number of tbm_surfaces in the queue
   */
  void AddNativeImageQueue(uint32_t queueSize)
  {
    ++mNativeImageQueueCount;
    mQueueSurfaceCount += queueSize;
    LogTotalCount();
  }

  /**
   * @brief Decrement count when a NativeImageQueue is destroyed
   * @param queueSize Number of tbm_surfaces in the queue
   */
  void RemoveNativeImageQueue(uint32_t queueSize)
  {
    Decrease(mNativeImageQueueCount, 1u);
    Decrease(mQueueSurfaceCount, queueSize);
    LogTotalCount();
  }

private:
  /**
   * @brief Decrease the count by the amount, only if the count is not less than the amount
   * @param[in,out] count The count to decrease
   * @param[in] amount The amount to decrease
   */
  static void Decrease(std::atomic<uint32_t>& count, uint32_t amount)
  {
    uint32_t current = count.load();
    while(current >= amount && !count.compare_exchange_weak(current, current - amount))
    {
    }
  }

  /**
   * @brief Log the current total count
   */
  void LogTotalCount()
  {
    const uint32_t nativeImageCount      = mNativeImageCount.load();
    const uint32_t backBufferCount       = mBackBufferCount.load();
    const uint32_t nativeImageQueueCount = mNativeImageQueueCount.load();
    const uint32_t queueSurfaceCount     = mQueueSurfaceCount.load();
    DALI_LOG_DEBUG_INFO("NativeImage=%u, NativeImageQueue=%u, QueueSurfaceCount=%u, TotalSurfaceCount=%u\n",
                        nativeImageCount, nativeImageQueueCount, queueSurfaceCount, nativeImageCount + queueSurfaceCount + backBufferCount);
  }

private:
  TbmSurfaceCounter()                                    = default;
  ~TbmSurfaceCounter()                                   = default;
  TbmSurfaceCounter(const TbmSurfaceCounter&)            = delete;
  TbmSurfaceCounter& operator=(const TbmSurfaceCounter&) = delete;

  std::atomic<uint32_t> mNativeImageCount{0u};
  std::atomic<uint32_t> mBackBufferCount{0u};
  std::atomic<uint32_t> mNativeImageQueueCount{0u};
  std::atomic<uint32_t> mQueueSurfaceCount{0u};
};

} // namespace Adaptor
} // namespace Internal
} //namespace DALI_NAMESPACE

#endif // DALI_TBM_SURFACE_COUNTER_H
