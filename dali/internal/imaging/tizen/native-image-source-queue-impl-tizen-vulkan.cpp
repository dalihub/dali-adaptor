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

// CLASS HEADER
#include <dali/internal/imaging/tizen/native-image-source-queue-impl-tizen-vulkan.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <tbm_surface_internal.h>
#include <algorithm>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/environment-variables.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
// clang-format off
int FORMATS_BLENDING_REQUIRED[] = {
  TBM_FORMAT_ARGB4444, TBM_FORMAT_ABGR4444,
  TBM_FORMAT_RGBA4444, TBM_FORMAT_BGRA4444,
  TBM_FORMAT_RGBX5551, TBM_FORMAT_BGRX5551,
  TBM_FORMAT_ARGB1555, TBM_FORMAT_ABGR1555,
  TBM_FORMAT_RGBA5551, TBM_FORMAT_BGRA5551,
  TBM_FORMAT_ARGB8888, TBM_FORMAT_ABGR8888,
  TBM_FORMAT_RGBA8888, TBM_FORMAT_BGRA8888,
  TBM_FORMAT_ARGB2101010, TBM_FORMAT_ABGR2101010,
  TBM_FORMAT_RGBA1010102, TBM_FORMAT_BGRA1010102
};
// clang-format on

constexpr int32_t NUM_FORMATS_BLENDING_REQUIRED = 18;

constexpr int32_t DEFAULT_TBM_SURFACE_QUEUE_SIZE = 3u;

int32_t GetTbmSurfaceQueueSize()
{
  static auto    queueSizeString = EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_TBM_SURFACE_QUEUE_SIZE);
  static int32_t queueSize       = queueSizeString ? std::atoi(queueSizeString) : DEFAULT_TBM_SURFACE_QUEUE_SIZE;
  return queueSize;
}

} // namespace

NativeImageSourceQueueTizenVulkan* NativeImageSourceQueueTizenVulkan::New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue)
{
  NativeImageSourceQueueTizenVulkan* image = new NativeImageSourceQueueTizenVulkan(queueCount, width, height, nativeImageSourceQueue);
  DALI_ASSERT_DEBUG(image && "NativeImageSourceQueueTizenVulkan allocation failed.");

  if(image)
  {
    image->Initialize(colorFormat);
  }

  return image;
}

NativeImageSourceQueueTizenVulkan::NativeImageSourceQueueTizenVulkan(uint32_t queueCount, uint32_t width, uint32_t height, Any nativeImageSourceQueue)
: mMutex(),
  mQueueCount(queueCount),
  mWidth(width),
  mHeight(height),
  mTbmQueue(nullptr),
  mConsumeSurface(nullptr),
  mSurfaces(),
  mBuffers(),
  mOwnTbmQueue(false),
  mBlendingRequired(false),
  mIsResized(false),
  mFreeRequest(false),
  mSurfaceRefs(),
  mPendingRelease()
{
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  mTbmQueue = GetSurfaceFromAny(nativeImageSourceQueue);

  if(mTbmQueue != nullptr)
  {
    mBlendingRequired = CheckBlending(tbm_surface_queue_get_format(mTbmQueue));
    mQueueCount       = tbm_surface_queue_get_size(mTbmQueue);
    mWidth            = tbm_surface_queue_get_width(mTbmQueue);
    mHeight           = tbm_surface_queue_get_height(mTbmQueue);
  }
}

NativeImageSourceQueueTizenVulkan::~NativeImageSourceQueueTizenVulkan()
{
  if(mOwnTbmQueue)
  {
    if(mTbmQueue != nullptr)
    {
      tbm_surface_queue_destroy(mTbmQueue);
    }
  }
}

void NativeImageSourceQueueTizenVulkan::Initialize(Dali::NativeImageSourceQueue::ColorFormat colorFormat)
{
  if(mWidth == 0 || mHeight == 0)
  {
    return;
  }

  if(mTbmQueue == nullptr)
  {
    int tbmFormat = TBM_FORMAT_ARGB8888;

    switch(colorFormat)
    {
      case Dali::NativeImageSourceQueue::ColorFormat::BGRA8888:
      {
        tbmFormat         = TBM_FORMAT_ARGB8888;
        mBlendingRequired = true;
        break;
      }
      case Dali::NativeImageSourceQueue::ColorFormat::BGRX8888:
      {
        tbmFormat         = TBM_FORMAT_XRGB8888;
        mBlendingRequired = false;
        break;
      }
      case Dali::NativeImageSourceQueue::ColorFormat::BGR888:
      {
        tbmFormat         = TBM_FORMAT_RGB888;
        mBlendingRequired = false;
        break;
      }
      case Dali::NativeImageSourceQueue::ColorFormat::RGBA8888:
      {
        tbmFormat         = TBM_FORMAT_ABGR8888;
        mBlendingRequired = true;
        break;
      }
      case Dali::NativeImageSourceQueue::ColorFormat::RGBX8888:
      {
        tbmFormat         = TBM_FORMAT_XBGR8888;
        mBlendingRequired = false;
        break;
      }
      case Dali::NativeImageSourceQueue::ColorFormat::RGB888:
      {
        tbmFormat         = TBM_FORMAT_BGR888;
        mBlendingRequired = false;
        break;
      }
      default:
      {
        DALI_LOG_ERROR("Wrong color format.\n");
        return;
      }
    }

    if(mQueueCount == 0)
    {
      mQueueCount = GetTbmSurfaceQueueSize();
    }

    mTbmQueue = tbm_surface_queue_create(mQueueCount, mWidth, mHeight, tbmFormat, 0);
    if(!mTbmQueue)
    {
      DALI_LOG_ERROR("NativeImageSourceQueueTizenVulkan::Initialize: tbm_surface_queue_create is failed! [%p]\n", mTbmQueue);
      return;
    }

    mOwnTbmQueue = true;
  }
}

tbm_surface_queue_h NativeImageSourceQueueTizenVulkan::GetSurfaceFromAny(Any source) const
{
  if(source.Empty())
  {
    return nullptr;
  }

  if(source.GetType() == typeid(tbm_surface_queue_h))
  {
    return AnyCast<tbm_surface_queue_h>(source);
  }
  else
  {
    return nullptr;
  }
}

Any NativeImageSourceQueueTizenVulkan::GetNativeImageSourceQueue() const
{
  return Any(mTbmQueue);
}

void NativeImageSourceQueueTizenVulkan::SetSize(uint32_t width, uint32_t height)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mWidth == width && mHeight == height)
  {
    return;
  }

  tbm_surface_queue_reset(mTbmQueue, width, height, tbm_surface_queue_get_format(mTbmQueue));

  mWidth     = width;
  mHeight    = height;
  mIsResized = true;
}

void NativeImageSourceQueueTizenVulkan::IgnoreSourceImage()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  tbm_surface_h           surface;

  if(tbm_surface_queue_can_acquire(mTbmQueue, 0))
  {
    if(tbm_surface_queue_acquire(mTbmQueue, &surface) != TBM_SURFACE_QUEUE_ERROR_NONE)
    {
      DALI_LOG_ERROR("NativeImageSourceQueueTizenVulkan::IgnoreSourceImage: Failed to aquire a tbm_surface\n");
      return;
    }

    if(tbm_surface_internal_is_valid(surface))
    {
      tbm_surface_queue_release(mTbmQueue, surface);
    }
  }
}

bool NativeImageSourceQueueTizenVulkan::CanDequeueBuffer()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(tbm_surface_queue_can_dequeue(mTbmQueue, 0))
  {
    return true;
  }
  return false;
}

uint8_t* NativeImageSourceQueueTizenVulkan::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride, Dali::NativeImageSourceQueue::BufferAccessType type)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(mTbmQueue == nullptr)
  {
    DALI_LOG_ERROR("TbmQueue is nullptr");
    return nullptr;
  }

  tbm_surface_h tbmSurface;
  if(tbm_surface_queue_dequeue(mTbmQueue, &tbmSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to dequeue a tbm_surface [%p]\n", tbmSurface);
    return nullptr;
  }

  int tbmOption = 0;
  if(type & Dali::NativeImageSourceQueue::BufferAccessType::READ)
  {
    tbmOption |= TBM_OPTION_READ;
  }
  if(type & Dali::NativeImageSourceQueue::BufferAccessType::WRITE)
  {
    tbmOption |= TBM_OPTION_WRITE;
  }

  tbm_surface_info_s info;
  int                ret = tbm_surface_map(tbmSurface, tbmOption, &info);
  if(ret != TBM_SURFACE_ERROR_NONE)
  {
    DALI_LOG_ERROR("tbm_surface_map is failed! [%d] [%p]\n", ret, tbmSurface);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return nullptr;
  }

  uint8_t* buffer = info.planes[0].ptr;
  if(!buffer)
  {
    DALI_LOG_ERROR("tbm buffer pointer is null! [%p]\n", tbmSurface);
    tbm_surface_unmap(tbmSurface);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return nullptr;
  }

  tbm_surface_internal_ref(tbmSurface);

  stride = info.planes[0].stride;
  width  = mWidth;
  height = mHeight;

  // Push the buffer
  mBuffers.insert({buffer, tbmSurface});
  return buffer;
}

bool NativeImageSourceQueueTizenVulkan::EnqueueBuffer(uint8_t* buffer)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  auto                    bufferInstance = mBuffers.find(buffer);
  if(bufferInstance != mBuffers.end())
  {
    tbm_surface_internal_unref((*bufferInstance).second);
    tbm_surface_unmap((*bufferInstance).second);
    tbm_surface_queue_enqueue(mTbmQueue, (*bufferInstance).second);
    mBuffers.erase(bufferInstance);
    return true;
  }
  return false;
}

void NativeImageSourceQueueTizenVulkan::CancelDequeuedBuffer(uint8_t* buffer)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  auto                    bufferInstance = mBuffers.find(buffer);
  if(bufferInstance != mBuffers.end())
  {
    tbm_surface_internal_unref((*bufferInstance).second);
    tbm_surface_unmap((*bufferInstance).second);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, (*bufferInstance).second);
    mBuffers.erase(bufferInstance);
  }
}

void NativeImageSourceQueueTizenVulkan::FreeReleasedBuffers()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mFreeRequest = true;
}

bool NativeImageSourceQueueTizenVulkan::CreateResource()
{
  return true;
}

void NativeImageSourceQueueTizenVulkan::DestroyResource()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  ResetSurfaceList(true);
}

uint32_t NativeImageSourceQueueTizenVulkan::TargetTexture()
{
  return 0;
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageSourceQueueTizenVulkan::PrepareTexture()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  // Clean up pending releases first
  CleanupPendingReleases();

  bool updated = false;

  do
  {
    tbm_surface_h oldSurface = mConsumeSurface;

    if(tbm_surface_queue_can_acquire(mTbmQueue, 0))
    {
      if(tbm_surface_queue_acquire(mTbmQueue, &mConsumeSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to aquire a tbm_surface\n");
        return Dali::NativeImageInterface::PrepareTextureResult::UNKNOWN_ERROR;
      }

      // Schedule old surface for delayed release instead of immediate release
      if(oldSurface)
      {
        if(tbm_surface_internal_is_valid(oldSurface))
        {
          ScheduleSurfaceForDelayedRelease(oldSurface);
        }
      }
      updated = true;
    }
    else
    {
      break;
    }
  } while(mFreeRequest); // Get the last one if buffer free was requested

  if(updated)
  {
    if(mIsResized)
    {
      ResetSurfaceList(false);
      mIsResized = false;
    }
  }

  if(mFreeRequest)
  {
    // Unref and erase all surfaces except mConsumeSurface
    for(auto iter = mSurfaces.begin(); iter != mSurfaces.end();)
    {
      if(*iter == mConsumeSurface)
      {
        ++iter;
      }
      else
      {
        tbm_surface_internal_unref(*iter);
        iter = mSurfaces.erase(iter);
      }
    }
    tbm_surface_queue_free_flush(mTbmQueue);
    mFreeRequest = false;
  }

  if(DALI_LIKELY(mConsumeSurface))
  {
    return updated ? Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED : Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR;
  }
  else
  {
    return Dali::NativeImageInterface::PrepareTextureResult::NOT_INITIALIZED_IMAGE;
  }
}

bool NativeImageSourceQueueTizenVulkan::ApplyNativeFragmentShader(std::string& shader, int count)
{
  return false;
}

const char* NativeImageSourceQueueTizenVulkan::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageSourceQueueTizenVulkan::GetTextureTarget() const
{
  return 0;
}

Any NativeImageSourceQueueTizenVulkan::GetNativeImageHandle() const
{
  //  return nullptr;
  return Any(mConsumeSurface);
}

bool NativeImageSourceQueueTizenVulkan::SourceChanged() const
{
  return true;
}

void NativeImageSourceQueueTizenVulkan::ResetSurfaceList(bool releaseConsumeSurface)
{
  // When Tbm surface queue is reset(resized), the surface acquired before reset() is still valid, not the others.
  // We can still use the acquired surface so that we will release it as the oldSurface in PrepareTexture() when the next surface is ready.
  if(releaseConsumeSurface && mConsumeSurface)
  {
    if(tbm_surface_internal_is_valid(mConsumeSurface))
    {
      ScheduleSurfaceForDelayedRelease(mConsumeSurface);
    }
    mConsumeSurface = nullptr;
  }

  // Release all tracked surfaces
  for(auto&& surface : mSurfaces)
  {
    tbm_surface_internal_unref(surface);
  }
  mSurfaces.clear();
}

bool NativeImageSourceQueueTizenVulkan::CheckBlending(int format)
{
  for(int32_t i = 0; i < NUM_FORMATS_BLENDING_REQUIRED; ++i)
  {
    if(format == FORMATS_BLENDING_REQUIRED[i])
    {
      return true;
    }
  }

  return false;
}

// NativeImageSurfaceReference implementation
void NativeImageSourceQueueTizenVulkan::AcquireSurfaceReference(void* surface)
{
  if(!surface) return;

  Dali::Mutex::ScopedLock lock(mMutex);
  tbm_surface_h           tbmSurface = reinterpret_cast<tbm_surface_h>(surface);

  auto& surfaceRef = GetOrCreateSurfaceRef(tbmSurface);
  surfaceRef.refCount++;
  surfaceRef.lastUsed = std::chrono::steady_clock::now();
}

void NativeImageSourceQueueTizenVulkan::ReleaseSurfaceReference(void* surface)
{
  if(!surface) return;

  Dali::Mutex::ScopedLock lock(mMutex);
  tbm_surface_h           tbmSurface = reinterpret_cast<tbm_surface_h>(surface);

  auto it = mSurfaceRefs.find(tbmSurface);
  if(it != mSurfaceRefs.end())
  {
    auto& surfaceRef  = *it->second;
    int   newRefCount = --surfaceRef.refCount;

    if(newRefCount <= 0)
    {
      // Check if this surface is in pending release list
      auto pendingIt = std::find_if(mPendingRelease.begin(), mPendingRelease.end(), [tbmSurface](const PendingRelease& pr) {
        return pr.surface == tbmSurface;
      });

      if(pendingIt != mPendingRelease.end())
      {
        // Safe to release now
        tbm_surface_queue_release(mTbmQueue, tbmSurface);
        mPendingRelease.erase(pendingIt);
      }

      mSurfaceRefs.erase(it);
    }
  }
}

void NativeImageSourceQueueTizenVulkan::ScheduleSurfaceForDelayedRelease(tbm_surface_h surface)
{
  auto it = mSurfaceRefs.find(surface);
  if(it != mSurfaceRefs.end() && it->second->refCount > 0)
  {
    // Surface is still referenced, add to pending release
    mPendingRelease.push_back({surface, std::chrono::steady_clock::now()});
  }
  else
  {
    // Safe to release immediately
    tbm_surface_queue_release(mTbmQueue, surface);
  }
}

void NativeImageSourceQueueTizenVulkan::CleanupPendingReleases()
{
  auto now = std::chrono::steady_clock::now();

  for(auto it = mPendingRelease.begin(); it != mPendingRelease.end();)
  {
    auto& [surface, timestamp] = *it;
    auto surfaceRefIt          = mSurfaceRefs.find(surface);

    bool canRelease = (surfaceRefIt == mSurfaceRefs.end()) || (surfaceRefIt->second->refCount <= 0);
    bool timedOut   = (now - timestamp) > std::chrono::seconds(2); // 2-second timeout fallback

    if(canRelease || timedOut)
    {
      tbm_surface_queue_release(mTbmQueue, surface);

      if(surfaceRefIt != mSurfaceRefs.end())
      {
        mSurfaceRefs.erase(surfaceRefIt);
      }

      it = mPendingRelease.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

NativeImageSourceQueueTizenVulkan::SurfaceReferenceData& NativeImageSourceQueueTizenVulkan::GetOrCreateSurfaceRef(tbm_surface_h surface)
{
  auto it = mSurfaceRefs.find(surface);
  if(it == mSurfaceRefs.end())
  {
    auto  surfaceRef      = std::make_unique<SurfaceReferenceData>(surface);
    auto* surfaceRefPtr   = surfaceRef.get();
    mSurfaceRefs[surface] = std::move(surfaceRef);
    return *surfaceRefPtr;
  }
  return *it->second;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
