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
#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/debug.h>

namespace Dali
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
    static TbmSurfaceCounter instance;
    return instance;
  }

  /**
   * @brief Increment count when a NativeImage is created
   */
  void AddNativeImage()
  {
    Mutex::ScopedLock lock(mMutex);
    mNativeImageCount++;
    LogTotalCount();
  }

  /**
   * @brief Decrement count when a NativeImage is destroyed
   */
  void RemoveNativeImage()
  {
    Mutex::ScopedLock lock(mMutex);
    if(mNativeImageCount > 0)
    {
      mNativeImageCount--;
    }
    LogTotalCount();
  }

  /**
   * @brief Increment count when a BackBuffer of NativeImage is created
   */
  void AddBackBufferSurface()
  {
    Mutex::ScopedLock lock(mMutex);
    mBackBufferCount++;
    LogTotalCount();
  }

  /**
   * @brief Decrement count when a BackBuffer of NativeImage is destroyed
   */
  void RemoveBackBufferSurface()
  {
    Mutex::ScopedLock lock(mMutex);
    if(mBackBufferCount > 0)
    {
      mBackBufferCount--;
    }
    LogTotalCount();
  }

  /**
   * @brief Increment count when a NativeImageQueue is created
   * @param queueSize Number of tbm_surfaces in the queue
   */
  void AddNativeImageQueue(uint32_t queueSize)
  {
    Mutex::ScopedLock lock(mMutex);
    mNativeImageQueueCount++;
    mQueueSurfaceCount += queueSize;
    LogTotalCount();
  }

  /**
   * @brief Decrement count when a NativeImageQueue is destroyed
   * @param queueSize Number of tbm_surfaces in the queue
   */
  void RemoveNativeImageQueue(uint32_t queueSize)
  {
    Mutex::ScopedLock lock(mMutex);
    if(mNativeImageQueueCount > 0)
    {
      mNativeImageQueueCount--;
    }
    if(mQueueSurfaceCount >= queueSize)
    {
      mQueueSurfaceCount -= queueSize;
    }
    LogTotalCount();
  }

private:
  /**
   * @brief Log the current total count
   */
  void LogTotalCount()
  {
    DALI_LOG_DEBUG_INFO("NativeImage=%u, NativeImageQueue=%u, QueueSurfaceCount=%u, TotalSurfaceCount=%u\n",
                        mNativeImageCount, mNativeImageQueueCount, mQueueSurfaceCount, mNativeImageCount + mQueueSurfaceCount + mBackBufferCount);
  }

private:
  TbmSurfaceCounter()                                    = default;
  ~TbmSurfaceCounter()                                   = default;
  TbmSurfaceCounter(const TbmSurfaceCounter&)            = delete;
  TbmSurfaceCounter& operator=(const TbmSurfaceCounter&) = delete;

  Dali::Mutex mMutex;
  uint32_t    mNativeImageCount      = 0;
  uint32_t    mBackBufferCount       = 0;
  uint32_t    mNativeImageQueueCount = 0;
  uint32_t    mQueueSurfaceCount     = 0;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_TBM_SURFACE_COUNTER_H
