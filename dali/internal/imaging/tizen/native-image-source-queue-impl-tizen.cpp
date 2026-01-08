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

// CLASS HEADER
#include <dali/internal/imaging/tizen/native-image-source-queue-impl-tizen.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>
#include <sys/poll.h>
#include <tbm_surface_internal.h>
#include <unistd.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles-impl/egl-sync-object.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/imaging/tizen/tbm-surface-counter.h>
#include <dali/internal/system/common/environment-variables.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
DALI_INIT_TIME_CHECKER_FILTER_WITH_DEFAULT_THRESHOLD(gTimeCheckerFilter, DALI_NATIVE_IMAGE_LOG_THRESHOLD_TIME, 48);

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

const char* SAMPLER_TYPE = "samplerExternalOES";

constexpr int32_t NUM_FORMATS_BLENDING_REQUIRED = 18;

constexpr int32_t DEFAULT_TBM_SURFACE_QUEUE_SIZE = 3u;

int32_t GetTbmSurfaceQueueSize()
{
  static auto    queueSizeString = EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_TBM_SURFACE_QUEUE_SIZE);
  static int32_t queueSize       = queueSizeString ? std::atoi(queueSizeString) : DEFAULT_TBM_SURFACE_QUEUE_SIZE;
  return queueSize;
}

} // namespace

NativeImageSourceQueueTizen* NativeImageSourceQueueTizen::New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue)
{
  NativeImageSourceQueueTizen* image = new NativeImageSourceQueueTizen(queueCount, width, height, nativeImageSourceQueue);
  DALI_ASSERT_DEBUG(image && "NativeImageSourceQueueTizen allocation failed.");

  if(image)
  {
    image->Initialize(colorFormat);
  }

  return image;
}

NativeImageSourceQueueTizen::NativeImageSourceQueueTizen(uint32_t queueCount, uint32_t width, uint32_t height, Any nativeImageSourceQueue)
: mMutex(),
  mQueueCount(queueCount),
  mWidth(width),
  mHeight(height),
  mTbmQueue(nullptr),
  mConsumeSurface(nullptr),
  mOldSurface(nullptr),
  mEglImages(),
  mBuffers(),
  mEglSyncObjects(),
  mEglSyncDiscardList(),
  mEglGraphics(NULL),
  mEglImageExtensions(NULL),
  mImageState(ImageState::INITIALIZED),
  mOwnTbmQueue(false),
  mBlendingRequired(false),
  mIsResized(false),
  mFreeRequest(false),
  mNeedSync(false),
  mWaitInWorkerThread(false)
{
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  auto graphics = &(Adaptor::GetImplementation(Adaptor::Get()).GetGraphicsInterface());
  mEglGraphics  = static_cast<EglGraphics*>(graphics);

  mTbmQueue = GetSurfaceFromAny(nativeImageSourceQueue);

  if(mTbmQueue != NULL)
  {
    mBlendingRequired = CheckBlending(tbm_surface_queue_get_format(mTbmQueue));
    mQueueCount       = tbm_surface_queue_get_size(mTbmQueue);
    mWidth            = tbm_surface_queue_get_width(mTbmQueue);
    mHeight           = tbm_surface_queue_get_height(mTbmQueue);
    DALI_ASSERT_ALWAYS(mQueueCount >= 2 && "Inputed tbm surface queue count must be 2 or more.");
  }
}

NativeImageSourceQueueTizen::~NativeImageSourceQueueTizen()
{
  // Remove from counter before destroying queue
  if(mTbmQueue)
  {
    TbmSurfaceCounter::GetInstance().RemoveNativeImageSourceQueue(mQueueCount);

    if(mOwnTbmQueue)
    {
      tbm_surface_queue_destroy(mTbmQueue);
    }
  }
}

void NativeImageSourceQueueTizen::Initialize(Dali::NativeImageSourceQueue::ColorFormat colorFormat)
{
  if(mWidth == 0 || mHeight == 0)
  {
    return;
  }

  if(mTbmQueue == NULL)
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
    DALI_ASSERT_ALWAYS(mQueueCount >= 2 && "Tbm surface queue count must be 2 or more.");

    mTbmQueue = tbm_surface_queue_create(mQueueCount, mWidth, mHeight, tbmFormat, 0);
    if(!mTbmQueue)
    {
      DALI_LOG_ERROR("NativeImageSourceQueueTizen::Initialize: tbm_surface_queue_create is failed! [%p]\n", mTbmQueue);
      return;
    }

    mOwnTbmQueue = true;
  }

  // Add to counter for newly created tbm_queue
  TbmSurfaceCounter::GetInstance().AddNativeImageSourceQueue(mQueueCount);
}

tbm_surface_queue_h NativeImageSourceQueueTizen::GetSurfaceFromAny(Any source) const
{
  if(source.Empty())
  {
    return NULL;
  }

  if(source.GetType() == typeid(tbm_surface_queue_h))
  {
    return AnyCast<tbm_surface_queue_h>(source);
  }
  else
  {
    return NULL;
  }
}

Any NativeImageSourceQueueTizen::GetNativeImageSourceQueue() const
{
  return Any(mTbmQueue);
}

void NativeImageSourceQueueTizen::SetSize(uint32_t width, uint32_t height)
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

void NativeImageSourceQueueTizen::IgnoreSourceImage()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  tbm_surface_h           surface;

  if(tbm_surface_queue_can_acquire(mTbmQueue, 0))
  {
    if(tbm_surface_queue_acquire(mTbmQueue, &surface) != TBM_SURFACE_QUEUE_ERROR_NONE)
    {
      DALI_LOG_ERROR("NativeImageSourceQueueTizen::IgnoreSourceImage: Failed to aquire a tbm_surface\n");
      return;
    }

    if(tbm_surface_internal_is_valid(surface))
    {
      tbm_surface_queue_release(mTbmQueue, surface);
    }
  }
}

bool NativeImageSourceQueueTizen::CanDequeueBuffer()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(tbm_surface_queue_can_dequeue(mTbmQueue, 0))
  {
    return true;
  }
  return false;
}

uint8_t* NativeImageSourceQueueTizen::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride, Dali::NativeImageSourceQueue::BufferAccessType type)
{
  tbm_surface_h  tbmSurface;
  uint8_t*       buffer     = nullptr;
  EglSyncObject* syncObject = nullptr;
  int32_t        fenceFd    = -1;

  {
    Dali::Mutex::ScopedLock lock(mMutex);
    if(mTbmQueue == nullptr)
    {
      DALI_LOG_ERROR("TbmQueue is NULL");
      return nullptr;
    }

    if(tbm_surface_queue_dequeue(mTbmQueue, &tbmSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Failed to dequeue a tbm_surface [%p]\n", tbmSurface);
      return nullptr;
    }

    tbm_surface_internal_ref(tbmSurface);

    if(mWaitInWorkerThread)
    {
      auto iter = mEglSyncObjects.find(tbmSurface);
      if(iter != mEglSyncObjects.end())
      {
        syncObject = iter->second.first;
        fenceFd    = iter->second.second;
        mEglSyncObjects.erase(iter);
      }
    }
  }

  if(mWaitInWorkerThread)
  {
    if(syncObject && fenceFd != -1)
    {
      struct pollfd fds;
      fds.fd      = fenceFd;
      fds.events  = POLLIN;
      fds.revents = 0;

      DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);

      int ret = poll(&fds, 1, 5000); // timeout 5sec

      DALI_TIME_CHECKER_END_WITH_MESSAGE_GENERATOR(gTimeCheckerFilter, [&](std::ostringstream& oss)
      {
        oss << "Wait sync: poll(" << tbmSurface << ", " << fenceFd << ")";
      });

      if(ret <= 0 || (fds.revents & (POLLERR | POLLNVAL)))
      {
        DALI_LOG_ERROR("poll failed or timeout [%d, %d]\n", ret, fds.revents);
      }
    }
  }

  {
    Dali::Mutex::ScopedLock lock(mMutex);

    if(mWaitInWorkerThread)
    {
      mEglSyncDiscardList[tbmSurface].emplace_back(syncObject, fenceFd);
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

    buffer = info.planes[0].ptr;
    if(!buffer)
    {
      DALI_LOG_ERROR("tbm buffer pointer is null! [%p]\n", tbmSurface);
      tbm_surface_unmap(tbmSurface);
      tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
      return nullptr;
    }

    if(DALI_UNLIKELY(info.width != mWidth || info.height != mHeight))
    {
      DALI_LOG_ERROR("tbm queue changed during dequeue! [%ux%u -> %ux%u] ignore buffer [%p]\n", info.width, info.height, mWidth, mHeight, tbmSurface);
      tbm_surface_unmap(tbmSurface);
      tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
      return nullptr;
    }

    stride = info.planes[0].stride;
    width  = mWidth;
    height = mHeight;

    // Push the buffer
    mBuffers.insert({buffer, tbmSurface});
  }

  return buffer;
}

bool NativeImageSourceQueueTizen::EnqueueBuffer(uint8_t* buffer)
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

void NativeImageSourceQueueTizen::CancelDequeuedBuffer(uint8_t* buffer)
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

void NativeImageSourceQueueTizen::FreeReleasedBuffers()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mFreeRequest = true;
}

void NativeImageSourceQueueTizen::SetQueueUsageHint(Dali::NativeImageSourceQueue::QueueUsageType type)
{
  mWaitInWorkerThread = (type == Dali::NativeImageSourceQueue::QueueUsageType::ENQUEUE_DEQUEUE) ? true : false;
}

bool NativeImageSourceQueueTizen::CreateResource()
{
  mEglImageExtensions = mEglGraphics->GetImageExtensions();
  DALI_ASSERT_DEBUG(mEglImageExtensions);

  return true;
}

void NativeImageSourceQueueTizen::DestroyResource()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  ResetEglImageList(true);
}

uint32_t NativeImageSourceQueueTizen::TargetTexture()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(DALI_LIKELY(mEglImageExtensions))
  {
    if(mImageState == ImageState::CHANGED && mConsumeSurface)
    {
      auto iter = mEglImages.find(mConsumeSurface);
      if(iter == mEglImages.end())
      {
        // Push the surface
        tbm_surface_internal_ref(mConsumeSurface);

        void* eglImageKHR = mEglImageExtensions->CreateImageKHR(reinterpret_cast<EGLClientBuffer>(mConsumeSurface));
        mEglImageExtensions->TargetTextureKHR(eglImageKHR);

        mEglImages.insert({mConsumeSurface, eglImageKHR});
      }
      else
      {
        mEglImageExtensions->TargetTextureKHR(iter->second);
      }
    }
    return 0;
  }
  return 1; // error case
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageSourceQueueTizen::PrepareTexture()
{
  DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);

  Dali::Mutex::ScopedLock lock(mMutex);

  if(mImageState != ImageState::INITIALIZED)
  {
    DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "PrepareTexture");
    return Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR;
  }

  // Destroy discard sync objects
  ResetDiscardSyncObjects();

  if(!mWaitInWorkerThread)
  {
    for(auto&& iter : mEglSyncObjects)
    {
      // Actually we only have 1 element in the list if mWaitInWorkerThread is true
      // But iterate the loop for safety
      iter.second.first->ClientWait();
    }

    ResetSyncObjects();
  }

  bool          updated    = false;
  tbm_surface_h oldSurface = mConsumeSurface;
  tbm_surface_h newSurface = mConsumeSurface;

  do
  {
    if(tbm_surface_queue_can_acquire(mTbmQueue, 0))
    {
      if(tbm_surface_queue_acquire(mTbmQueue, &newSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to aquire a tbm_surface\n");
        DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "PrepareTexture");
        return Dali::NativeImageInterface::PrepareTextureResult::UNKNOWN_ERROR;
      }

      if(oldSurface != mConsumeSurface)
      {
        // We need a sync in case of the current consumed surface.
        // So we will release the buffer in PostRender().
        tbm_surface_queue_release(mTbmQueue, oldSurface);
      }
      oldSurface = newSurface;
      updated    = true;
    }
    else
    {
      break;
    }
  } while(mFreeRequest); // Get the last one if buffer free was requested

  if(updated)
  {
    mOldSurface     = mConsumeSurface;
    mConsumeSurface = newSurface;
    mImageState     = ImageState::CHANGED;

    if(mIsResized)
    {
      // We will free all images except the current surface. Releaes the old surface now.
      if(tbm_surface_internal_is_valid(mOldSurface))
      {
        tbm_surface_queue_release(mTbmQueue, mOldSurface);
      }
      mOldSurface = nullptr;

      ResetEglImageList(false);
      mIsResized = false;
    }
    else
    {
      // Mark to make a sync object
      mNeedSync = true;
    }
  }
  else
  {
    mImageState = ImageState::NOT_CHANGED;
  }

  if(mFreeRequest)
  {
    // We will free all images except the current surface. Releaes the old surface now.
    if(tbm_surface_internal_is_valid(mOldSurface))
    {
      tbm_surface_queue_release(mTbmQueue, mOldSurface);
    }
    mOldSurface = nullptr;
    mNeedSync   = false;

    // Destroy all egl images which is not mConsumeSurface.
    for(auto iter = mEglImages.begin(); iter != mEglImages.end();)
    {
      if(iter->first == mConsumeSurface)
      {
        ++iter;
      }
      else
      {
        mEglImageExtensions->DestroyImageKHR(iter->second);
        tbm_surface_internal_unref(iter->first);

        iter = mEglImages.erase(iter);
      }
    }

    tbm_surface_queue_free_flush(mTbmQueue);
    mFreeRequest = false;

    // We freed all released buffers. So we don't need the sync objects.
    ResetSyncObjects();
  }

  DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "PrepareTexture");

  if(DALI_LIKELY(mConsumeSurface))
  {
    return updated ? Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED : Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR;
  }
  else
  {
    return mEglImageExtensions ? Dali::NativeImageInterface::PrepareTextureResult::NOT_INITIALIZED_GRAPHICS : Dali::NativeImageInterface::PrepareTextureResult::NOT_INITIALIZED_IMAGE;
  }
}

bool NativeImageSourceQueueTizen::ApplyNativeFragmentShader(std::string& shader, int count)
{
  return mEglGraphics->ApplyNativeFragmentShader(shader, SAMPLER_TYPE, count);
}

const char* NativeImageSourceQueueTizen::GetCustomSamplerTypename() const
{
  return SAMPLER_TYPE;
}

int NativeImageSourceQueueTizen::GetTextureTarget() const
{
  return GL_TEXTURE_EXTERNAL_OES;
}

Any NativeImageSourceQueueTizen::GetNativeImageHandle() const
{
  return nullptr;
}

bool NativeImageSourceQueueTizen::SourceChanged() const
{
  return true;
}

bool NativeImageSourceQueueTizen::CreateSyncObject()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  tbm_surface_h tbmSurface;
  bool          ret = true;

  // We need a sync for the current surface if we should wait in the render thread
  tbmSurface = mWaitInWorkerThread ? mOldSurface : mConsumeSurface;

  if(tbm_surface_internal_is_valid(tbmSurface))
  {
    // Discard previous sync object first
    {
      auto iter = mEglSyncObjects.find(tbmSurface);
      if(iter != mEglSyncObjects.end())
      {
        mEglSyncDiscardList[tbmSurface].emplace_back(iter->second);
        mEglSyncObjects.erase(iter);
      }
    }

    Internal::Adaptor::EglSyncObject* syncObject = static_cast<Internal::Adaptor::EglSyncObject*>(mEglGraphics->GetSyncImplementation().CreateSyncObject(EglSyncObject::SyncType::NATIVE_FENCE_SYNC));
    if(DALI_UNLIKELY(!syncObject))
    {
      DALI_LOG_ERROR("CreateSyncObject failed [%d]\n");
      ret = false;
    }
    else
    {
      int32_t fenceFd = syncObject->DuplicateNativeFenceFD();
      if(fenceFd == -1)
      {
        // We can't wait for the sync object to be signaled in the worker thread
        mWaitInWorkerThread = false;
      }

      mEglGraphics->GetGlAbstraction().Flush();

      // Insert the new object
      [[maybe_unused]] auto insertResult = mEglSyncObjects.insert({tbmSurface, std::make_pair(syncObject, fenceFd)});
      DALI_ASSERT_DEBUG(insertResult.second && "We don't allow multiple sync objects cache!\n");
    }
  }

  // Release the old surface now
  if(tbm_surface_internal_is_valid(mOldSurface))
  {
    tbm_surface_queue_release(mTbmQueue, mOldSurface);
  }

  mOldSurface = nullptr;

  return ret;
}

// This Method is called inside mMutex
void NativeImageSourceQueueTizen::ResetSyncObjects()
{
  for(auto&& iter : mEglSyncObjects)
  {
    mEglGraphics->GetSyncImplementation().DestroySyncObject(iter.second.first);

    if(iter.second.second != -1)
    {
      close(iter.second.second);
    }
  }
  mEglSyncObjects.clear();
}

// This Method is called inside mMutex
void NativeImageSourceQueueTizen::ResetDiscardSyncObjects()
{
  for(auto&& iter : mEglSyncDiscardList)
  {
    for(auto&& syncPair : iter.second)
    {
      mEglGraphics->GetSyncImplementation().DestroySyncObject(syncPair.first);

      if(syncPair.second != -1)
      {
        close(syncPair.second);
      }
    }
  }
  mEglSyncDiscardList.clear();
}

void NativeImageSourceQueueTizen::PostRender()
{
  // Create the sync object when we change the egl image
  // We need the sync every frame if we should wait in the render thread
  if(mNeedSync || (!mWaitInWorkerThread && mImageState != ImageState::INITIALIZED))
  {
    CreateSyncObject();

    mNeedSync = false;
  }

  // Reset the image state
  mImageState = ImageState::INITIALIZED;
}

// This Method is called inside mMutex
void NativeImageSourceQueueTizen::ResetEglImageList(bool releaseConsumeSurface)
{
  // When Tbm surface queue is reset(resized), the surface acquired before reset() is still valid, not the others.
  // We can still use the acquired surface so that we will release it as the oldSurface in PrepareTexture() when the next surface is ready.
  if(releaseConsumeSurface && mConsumeSurface)
  {
    if(tbm_surface_internal_is_valid(mConsumeSurface))
    {
      tbm_surface_queue_release(mTbmQueue, mConsumeSurface);
    }
    mConsumeSurface = NULL;
  }

  for(auto&& iter : mEglImages)
  {
    mEglImageExtensions->DestroyImageKHR(iter.second);

    tbm_surface_internal_unref(iter.first);
  }
  mEglImages.clear();

  ResetSyncObjects();
  ResetDiscardSyncObjects();
}

bool NativeImageSourceQueueTizen::CheckBlending(int format)
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

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
