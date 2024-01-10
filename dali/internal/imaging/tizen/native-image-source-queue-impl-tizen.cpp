/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>
#include <tbm_surface_internal.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>

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

const char* SAMPLER_TYPE = "samplerExternalOES";

constexpr int32_t NUM_FORMATS_BLENDING_REQUIRED = 18;

constexpr int32_t DEFAULT_TBM_SURFACE_QUEUE_SIZE = 3u;
constexpr auto    TBM_SURFACE_QUEUE_SIZE         = "DALI_TBM_SURFACE_QUEUE_SIZE";

int32_t GetTbmSurfaceQueueSize()
{
  static auto    queueSizeString = EnvironmentVariable::GetEnvironmentVariable(TBM_SURFACE_QUEUE_SIZE);
  static int32_t queueSize       = queueSizeString ? std::atoi(queueSizeString) : DEFAULT_TBM_SURFACE_QUEUE_SIZE;
  return queueSize;
}

} // namespace

NativeImageSourceQueueTizen* NativeImageSourceQueueTizen::New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue)
{
  NativeImageSourceQueueTizen* image = new NativeImageSourceQueueTizen(queueCount, width, height, colorFormat, nativeImageSourceQueue);
  DALI_ASSERT_DEBUG(image && "NativeImageSourceQueueTizen allocation failed.");

  if(image)
  {
    image->Initialize(colorFormat);
  }

  return image;
}

NativeImageSourceQueueTizen::NativeImageSourceQueueTizen(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue)
: mMutex(),
  mQueueCount(queueCount),
  mWidth(width),
  mHeight(height),
  mTbmQueue(NULL),
  mConsumeSurface(NULL),
  mEglImages(),
  mBuffers(),
  mEglGraphics(NULL),
  mEglImageExtensions(NULL),
  mOwnTbmQueue(false),
  mBlendingRequired(false),
  mIsResized(false),
  mFreeRequest(false)
{
  DALI_ASSERT_ALWAYS(Adaptor::IsAvailable());

  GraphicsInterface* graphics = &(Adaptor::GetImplementation(Adaptor::Get()).GetGraphicsInterface());
  mEglGraphics                = static_cast<EglGraphics*>(graphics);

  mTbmQueue = GetSurfaceFromAny(nativeImageSourceQueue);

  if(mTbmQueue != NULL)
  {
    mBlendingRequired = CheckBlending(tbm_surface_queue_get_format(mTbmQueue));
    mQueueCount       = tbm_surface_queue_get_size(mTbmQueue);
    mWidth            = tbm_surface_queue_get_width(mTbmQueue);
    mHeight           = tbm_surface_queue_get_height(mTbmQueue);
  }
}

NativeImageSourceQueueTizen::~NativeImageSourceQueueTizen()
{
  if(mOwnTbmQueue)
  {
    if(mTbmQueue != NULL)
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
      case Dali::NativeImageSourceQueue::ColorFormat::RGBA8888: // TODO : Implement me after other codes fixed.
      case Dali::NativeImageSourceQueue::ColorFormat::BGRA8888:
      {
        tbmFormat         = TBM_FORMAT_ARGB8888;
        mBlendingRequired = true;
        break;
      }
      case Dali::NativeImageSourceQueue::ColorFormat::RGBX8888: // TODO : Implement me after other codes fixed.
      case Dali::NativeImageSourceQueue::ColorFormat::BGRX8888:
      {
        tbmFormat         = TBM_FORMAT_XRGB8888;
        mBlendingRequired = false;
        break;
      }
      case Dali::NativeImageSourceQueue::ColorFormat::RGB888: // TODO : Implement me after other codes fixed.
      case Dali::NativeImageSourceQueue::ColorFormat::BGR888:
      {
        tbmFormat         = TBM_FORMAT_RGB888;
        mBlendingRequired = false;
        break;
      }
      default:
      {
        DALI_LOG_WARNING("Wrong color format.\n");
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
      DALI_LOG_ERROR("NativeImageSourceQueueTizen::Initialize: tbm_surface_queue_create is failed! [%p]\n", mTbmQueue);
      return;
    }

    mOwnTbmQueue = true;
  }
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

uint8_t* NativeImageSourceQueueTizen::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(mTbmQueue == NULL)
  {
    DALI_LOG_ERROR("TbmQueue is NULL");
    return NULL;
  }

  tbm_surface_h tbmSurface;
  if(tbm_surface_queue_dequeue(mTbmQueue, &tbmSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to dequeue a tbm_surface [%p]\n", tbmSurface);
    return NULL;
  }

  tbm_surface_info_s info;
  int                ret = tbm_surface_map(tbmSurface, TBM_OPTION_WRITE, &info);
  if(ret != TBM_SURFACE_ERROR_NONE)
  {
    DALI_LOG_ERROR("tbm_surface_map is failed! [%d] [%p]\n", ret, tbmSurface);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return NULL;
  }

  uint8_t* buffer = info.planes[0].ptr;
  if(!buffer)
  {
    DALI_LOG_ERROR("tbm buffer pointer is null! [%p]\n", tbmSurface);
    tbm_surface_unmap(tbmSurface);
    tbm_surface_queue_cancel_dequeue(mTbmQueue, tbmSurface);
    return NULL;
  }

  tbm_surface_internal_ref(tbmSurface);

  stride = info.planes[0].stride;
  width  = mWidth;
  height = mHeight;

  // Push the buffer
  mBuffers.insert({buffer, tbmSurface});
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

void NativeImageSourceQueueTizen::FreeReleasedBuffers()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mFreeRequest = true;
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
  return 0;
}

void NativeImageSourceQueueTizen::PrepareTexture()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  bool updated = false;

  do
  {
    tbm_surface_h oldSurface = mConsumeSurface;

    if(tbm_surface_queue_can_acquire(mTbmQueue, 0))
    {
      if(tbm_surface_queue_acquire(mTbmQueue, &mConsumeSurface) != TBM_SURFACE_QUEUE_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to aquire a tbm_surface\n");
        return;
      }

      if(oldSurface)
      {
        if(tbm_surface_internal_is_valid(oldSurface))
        {
          tbm_surface_queue_release(mTbmQueue, oldSurface);
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
      ResetEglImageList(false);
      mIsResized = false;
    }

    if(mConsumeSurface)
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
  }

  if(mFreeRequest)
  {
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
  }
}

bool NativeImageSourceQueueTizen::ApplyNativeFragmentShader(std::string& shader)
{
  return mEglGraphics->ApplyNativeFragmentShader(shader, SAMPLER_TYPE);
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
