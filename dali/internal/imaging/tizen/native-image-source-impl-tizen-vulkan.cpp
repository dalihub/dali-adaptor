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
#include <dali/internal/imaging/tizen/native-image-source-impl-tizen-vulkan.h>

// EXTERNAL INCLUDES
#include <errno.h>
#include <tbm_bo.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>
#include <tbm_type_common.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

#ifdef EXPORT_API
#undef EXPORT_API
#endif

#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0
#endif

#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/pixel-data-integ.h>
#include <tbm_surface_internal.h>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
// clang-format off
tbm_format FORMATS_BLENDING_REQUIRED[] = {
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

const int NUM_FORMATS_BLENDING_REQUIRED = 18;

} // namespace

using Dali::Integration::PixelBuffer;

NativeImageSourceTizenVulkan* NativeImageSourceTizenVulkan::New(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource)
{
  NativeImageSourceTizenVulkan* image = new NativeImageSourceTizenVulkan(width, height, depth, nativeImageSource);
  DALI_ASSERT_DEBUG(image && "NativeImageSource allocation failed.");

  if(image)
  {
    image->Initialize();
  }

  return image;
}

NativeImageSourceTizenVulkan::NativeImageSourceTizenVulkan(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource)
: mWidth(width),
  mHeight(height),
  mColorDepth(depth),
  mTbmSurface(nullptr),
  mTbmBackSurface(nullptr),
  mTbmFormat(0),
  mMutex(),
  mResourceDestructionCallback(),
  mOwnTbmSurface(false),
  mBlendingRequired(false),
  mSetSource(false),
  mResourceCreated(false),
  mIsBufferAcquired(false),
  mBackBufferEnabled(false)
{
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  mTbmSurface = GetSurfaceFromAny(nativeImageSource);

  if(mTbmSurface != nullptr)
  {
    tbm_surface_internal_ref(mTbmSurface);
    mBlendingRequired = CheckBlending(tbm_surface_get_format(mTbmSurface));
    mWidth            = tbm_surface_get_width(mTbmSurface);
    mHeight           = tbm_surface_get_height(mTbmSurface);
  }
}

void NativeImageSourceTizenVulkan::Initialize()
{
  if(mTbmSurface != nullptr || mWidth == 0 || mHeight == 0)
  {
    return;
  }

  tbm_format format = TBM_FORMAT_RGB888;
  int        depth  = 0;

  switch(mColorDepth)
  {
    case Dali::NativeImageSource::COLOR_DEPTH_DEFAULT:
    {
      format = TBM_FORMAT_ARGB8888;
      depth  = 32;
      break;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_8:
    {
      format = TBM_FORMAT_C8;
      depth  = 8;
      break;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_16:
    {
      format = TBM_FORMAT_RGB565;
      depth  = 16;
      break;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_24:
    {
      format = TBM_FORMAT_RGB888;
      depth  = 24;
      break;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_32:
    {
      format = TBM_FORMAT_ARGB8888;
      depth  = 32;
      break;
    }
    default:
    {
      DALI_LOG_WARNING("Wrong color depth.\n");
      return;
    }
  }

  // set whether blending is required according to pixel format based on the depth
  /* default pixel format is RGB888
     If depth = 8, Pixel::A8;
     If depth = 16, Pixel::RGB565;
     If depth = 32, Pixel::RGBA8888 */
  mBlendingRequired = (depth == 32 || depth == 8);

  mTbmSurface    = tbm_surface_create(mWidth, mHeight, format);
  mOwnTbmSurface = true;
}

tbm_surface_h NativeImageSourceTizenVulkan::GetSurfaceFromAny(Any source) const
{
  if(source.Empty())
  {
    return nullptr;
  }

  if(source.GetType() == typeid(tbm_surface_h))
  {
    return AnyCast<tbm_surface_h>(source);
  }
  else
  {
    return nullptr;
  }
}

// TbmOffscreenTexture::freeTextureData
void NativeImageSourceTizenVulkan::DestroySurface()
{
  if(mTbmSurface)
  {
    if(mIsBufferAcquired)
    {
      Rect<uint32_t> emptyRect{};
      ReleaseBuffer(emptyRect);
    }
    if(mOwnTbmSurface)
    {
      if(tbm_surface_destroy(mTbmSurface) != TBM_SURFACE_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to destroy tbm_surface\n");
      }
    }
    else
    {
      tbm_surface_internal_unref(mTbmSurface);
    }
    mTbmSurface = nullptr;

    DestroyBackBuffer();
  }
}

NativeImageSourceTizenVulkan::~NativeImageSourceTizenVulkan()
{
  DestroySurface();
}

Any NativeImageSourceTizenVulkan::GetNativeImageSource() const
{
  return Any(mTbmSurface);
}

bool NativeImageSourceTizenVulkan::GetPixels(std::vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const
{
  std::scoped_lock lock(mMutex);
  if(mTbmSurface != nullptr)
  {
    tbm_surface_info_s surface_info;

    if(tbm_surface_map(mTbmSurface, TBM_SURF_OPTION_READ, &surface_info) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to map tbm_surface\n");

      width  = 0;
      height = 0;

      return false;
    }

    tbm_format format = surface_info.format;
    uint32_t   stride = surface_info.planes[0].stride;
    uint8_t*   ptr    = surface_info.planes[0].ptr;

    width  = mWidth;
    height = mHeight;
    size_t lineSize;
    size_t offset;
    size_t cOffset;

    switch(format)
    {
      case TBM_FORMAT_RGB888:
      {
        lineSize    = width * 3;
        pixelFormat = Pixel::RGB888;
        pixbuf.resize(lineSize * height);
        uint8_t* bufptr = &pixbuf[0];

        for(uint32_t r = 0; r < height; ++r, bufptr += lineSize)
        {
          for(uint32_t c = 0; c < width; ++c)
          {
            cOffset                 = c * 3;
            offset                  = cOffset + r * stride;
            *(bufptr + cOffset)     = ptr[offset + 2];
            *(bufptr + cOffset + 1) = ptr[offset + 1];
            *(bufptr + cOffset + 2) = ptr[offset];
          }
        }
        break;
      }
      case TBM_FORMAT_RGBA8888:
      {
        lineSize    = width * 4;
        pixelFormat = Pixel::RGBA8888;
        pixbuf.resize(lineSize * height);
        uint8_t* bufptr = &pixbuf[0];

        for(uint32_t r = 0; r < height; ++r, bufptr += lineSize)
        {
          for(uint32_t c = 0; c < width; ++c)
          {
            cOffset                 = c * 4;
            offset                  = cOffset + r * stride;
            *(bufptr + cOffset)     = ptr[offset + 3];
            *(bufptr + cOffset + 1) = ptr[offset + 2];
            *(bufptr + cOffset + 2) = ptr[offset + 1];
            *(bufptr + cOffset + 3) = ptr[offset];
          }
        }
        break;
      }
      case TBM_FORMAT_ARGB8888:
      {
        lineSize    = width * 4;
        pixelFormat = Pixel::RGBA8888;
        pixbuf.resize(lineSize * height);
        uint8_t* bufptr = &pixbuf[0];

        for(uint32_t r = 0; r < height; ++r, bufptr += lineSize)
        {
          for(uint32_t c = 0; c < width; ++c)
          {
            cOffset                 = c * 4;
            offset                  = cOffset + r * stride;
            *(bufptr + cOffset)     = ptr[offset + 2];
            *(bufptr + cOffset + 1) = ptr[offset + 1];
            *(bufptr + cOffset + 2) = ptr[offset];
            *(bufptr + cOffset + 3) = ptr[offset + 3];
          }
        }
        break;
      }
      default:
      {
        DALI_ASSERT_ALWAYS(0 && "Tbm surface has unsupported pixel format.\n");

        return false;
      }
    }

    if(tbm_surface_unmap(mTbmSurface) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to unmap tbm_surface\n");
    }

    return true;
  }

  DALI_LOG_ERROR("TBM surface does not exist.\n");

  width  = 0;
  height = 0;

  return false;
}

bool NativeImageSourceTizenVulkan::SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat)
{
  std::scoped_lock lock(mMutex);
  if(mTbmSurface != nullptr)
  {
    tbm_surface_info_s surface_info;

    if(tbm_surface_map(mTbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &surface_info) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to map tbm_surface\n");
      return false;
    }
    tbm_surface_internal_ref(mTbmSurface);

    if(pixelFormat != Pixel::Format::RGBA8888 && pixelFormat != Pixel::Format::RGB888)
    {
      DALI_LOG_ERROR("Not Supported PixelFormat\n");
      return false;
    }

    tbm_format format = surface_info.format;
    uint32_t   stride = surface_info.planes[0].stride;
    uint8_t*   ptr    = static_cast<uint8_t*>(surface_info.planes[0].ptr);

    size_t lineSize;
    size_t inputBufferLinePixelSize = Dali::Pixel::GetBytesPerPixel(pixelFormat);

    switch(format)
    {
      case TBM_FORMAT_RGB888:
      {
        lineSize        = mWidth * inputBufferLinePixelSize;
        uint8_t* bufptr = &pixbuf[0];

        for(uint32_t r = 0; r < mHeight; ++r, bufptr += lineSize)
        {
          for(uint32_t c = 0; c < mWidth; ++c)
          {
            size_t sOffset  = c * inputBufferLinePixelSize;
            size_t dOffset  = c * 3;
            size_t offset   = dOffset + r * stride;
            ptr[offset + 2] = *(bufptr + sOffset);
            ptr[offset + 1] = *(bufptr + sOffset + 1);
            ptr[offset]     = *(bufptr + sOffset + 2);
          }
        }
        break;
      }
      case TBM_FORMAT_RGBA8888:
      {
        lineSize        = mWidth * inputBufferLinePixelSize;
        uint8_t* bufptr = &pixbuf[0];

        for(uint32_t r = 0; r < mHeight; ++r, bufptr += lineSize)
        {
          for(uint32_t c = 0; c < mWidth; ++c)
          {
            size_t sOffset  = c * inputBufferLinePixelSize;
            size_t dOffset  = c * 4;
            size_t offset   = dOffset + r * stride;
            ptr[offset + 3] = *(bufptr + sOffset);
            ptr[offset + 2] = *(bufptr + sOffset + 1);
            ptr[offset + 1] = *(bufptr + sOffset + 2);
            ptr[offset]     = (inputBufferLinePixelSize == 4) ? *(bufptr + sOffset + 3) : 0xFF;
          }
        }
        break;
      }
      case TBM_FORMAT_ARGB8888:
      {
        lineSize        = mWidth * inputBufferLinePixelSize;
        uint8_t* bufptr = &pixbuf[0];

        for(uint32_t r = 0; r < mHeight; ++r, bufptr += lineSize)
        {
          for(uint32_t c = 0; c < mWidth; ++c)
          {
            size_t sOffset  = c * inputBufferLinePixelSize;
            size_t dOffset  = c * 4;
            size_t offset   = dOffset + r * stride;
            ptr[offset + 2] = *(bufptr + sOffset);
            ptr[offset + 1] = *(bufptr + sOffset + 1);
            ptr[offset]     = *(bufptr + sOffset + 2);
            ptr[offset + 3] = (inputBufferLinePixelSize == 4) ? *(bufptr + sOffset + 3) : 0xFF;
          }
        }
        break;
      }
      default:
      {
        DALI_ASSERT_ALWAYS(0 && "Tbm surface has unsupported pixel format.\n");

        return false;
      }
    }

    if(tbm_surface_unmap(mTbmSurface) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to unmap tbm_surface\n");
    }
    tbm_surface_internal_unref(mTbmSurface);

    return true;
  }

  DALI_LOG_ERROR("TBM surface does not exist.\n");

  return false;
}

void NativeImageSourceTizenVulkan::SetSource(Any source)
{
  std::scoped_lock lock(mMutex);

  DestroySurface();

  mOwnTbmSurface = false;
  mTbmSurface    = GetSurfaceFromAny(source);

  if(mTbmSurface != nullptr)
  {
    mSetSource = true;
    tbm_surface_internal_ref(mTbmSurface);
    mBlendingRequired = CheckBlending(tbm_surface_get_format(mTbmSurface));
    mWidth            = tbm_surface_get_width(mTbmSurface);
    mHeight           = tbm_surface_get_height(mTbmSurface);

    if(mBackBufferEnabled)
    {
      DestroyBackBuffer();
      CreateBackBuffer();
    }
  }
}

bool NativeImageSourceTizenVulkan::IsColorDepthSupported(Dali::NativeImageSource::ColorDepth colorDepth)
{
  uint32_t*  formats;
  uint32_t   formatNum;
  tbm_format format = TBM_FORMAT_RGB888;

  switch(colorDepth)
  {
    case Dali::NativeImageSource::COLOR_DEPTH_DEFAULT:
    {
      format = TBM_FORMAT_ARGB8888;
      break;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_8:
    {
      format = TBM_FORMAT_C8;
      break;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_16:
    {
      format = TBM_FORMAT_RGB565;
      break;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_24:
    {
      format = TBM_FORMAT_RGB888;
      break;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_32:
    {
      format = TBM_FORMAT_ARGB8888;
      break;
    }
  }

  if(tbm_surface_query_formats(&formats, &formatNum))
  {
    for(uint32_t i = 0; i < formatNum; i++)
    {
      if(formats[i] == format)
      {
        free(formats);
        return true;
      }
    }
  }

  free(formats);
  return false;
}

bool NativeImageSourceTizenVulkan::CreateResource()
{
  std::scoped_lock lock(mMutex);

  if(mResourceCreated || !mTbmSurface)
  {
    return mResourceCreated;
  }

  // Get TBM surface format for mapping
  mTbmFormat = tbm_surface_get_format(mTbmSurface);

  mResourceCreated = true;
  return mResourceCreated;
}

void NativeImageSourceTizenVulkan::DestroyResource()
{
  std::scoped_lock lock(mMutex);

  if(!mResourceCreated)
  {
    return;
  }

  mResourceCreated = false;

  if(mResourceDestructionCallback)
  {
    mResourceDestructionCallback->Trigger();
  }
}

uint32_t NativeImageSourceTizenVulkan::TargetTexture()
{
  // Not used in Vulkan backend

  return 0;
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageSourceTizenVulkan::PrepareTexture()
{
  std::scoped_lock lock(mMutex);

  if(mSetSource)
  {
    CreateResource();
    mSetSource = false;
  }

  // For single buffer, always return IMAGE_CHANGED
  return NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED;
}

bool NativeImageSourceTizenVulkan::ApplyNativeFragmentShader(std::string& shader, int count)
{
  // Not used in Vulkan backend
  return false;
}

const char* NativeImageSourceTizenVulkan::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageSourceTizenVulkan::GetTextureTarget() const
{
  // Not used in Vulkan backend
  return 0;
}

Any NativeImageSourceTizenVulkan::GetNativeImageHandle() const
{
  return GetNativeImageSource();
}

bool NativeImageSourceTizenVulkan::SourceChanged() const
{
  std::scoped_lock lock(mMutex);
  if(mTbmBackSurface)
  {
    return mUpdatedArea.IsEmpty() ? false : true;
  }
  return true;
}

Rect<uint32_t> NativeImageSourceTizenVulkan::GetUpdatedArea()
{
  std::scoped_lock lock(mMutex);
  Rect<uint32_t>   updatedArea{0, 0, mWidth, mHeight};
  if(!mUpdatedArea.IsEmpty() && mTbmSurface != nullptr && mTbmBackSurface != nullptr)
  {
    updatedArea = mUpdatedArea;

    tbm_surface_info_s info;
    if(tbm_surface_map(mTbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &info) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to map tbm_surface\n");
      return updatedArea;
    }

    tbm_surface_info_s backBufferInfo;
    if(tbm_surface_map(mTbmBackSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &backBufferInfo) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to map tbm_surface - backbuffer\n");
      tbm_surface_unmap(mTbmSurface);
      return updatedArea;
    }

    uint8_t* srcBuffer = info.planes[0].ptr;
    uint8_t* dstBuffer = backBufferInfo.planes[0].ptr;

    uint32_t stride        = info.planes[0].stride;
    uint32_t bytesPerPixel = info.bpp >> 3;

    srcBuffer += updatedArea.y * stride + updatedArea.x * bytesPerPixel;
    dstBuffer += updatedArea.y * stride + updatedArea.x * bytesPerPixel;

    // Copy to back buffer
    for(uint32_t y = 0; y < updatedArea.height; y++)
    {
      memcpy(dstBuffer, srcBuffer, updatedArea.width * bytesPerPixel);
      srcBuffer += stride;
      dstBuffer += stride;
    }

    tbm_surface_unmap(mTbmSurface);
    tbm_surface_unmap(mTbmBackSurface);

    // Reset the updated area
    mUpdatedArea.Set(0u, 0u, 0u, 0u);
  }
  return updatedArea;
}

bool NativeImageSourceTizenVulkan::CheckBlending(tbm_format format)
{
  if(mTbmFormat != format)
  {
    for(int i = 0; i < NUM_FORMATS_BLENDING_REQUIRED; ++i)
    {
      if(format == FORMATS_BLENDING_REQUIRED[i])
      {
        mBlendingRequired = true;
        break;
      }
    }
    mTbmFormat = format;
  }

  return mBlendingRequired;
}

uint8_t* NativeImageSourceTizenVulkan::AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  mMutex.lock(); // We don't use std::scoped_lock here
  if(mTbmSurface != nullptr)
  {
    tbm_surface_info_s info;

    if(tbm_surface_map(mTbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &info) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to map tbm_surface\n");

      width  = 0;
      height = 0;

      mMutex.unlock();
      return nullptr;
    }
    tbm_surface_internal_ref(mTbmSurface);
    mIsBufferAcquired = true;

    stride = info.planes[0].stride;
    width  = mWidth;
    height = mHeight;

    // The lock is held until ReleaseBuffer is called
    return info.planes[0].ptr;
  }
  mMutex.unlock();
  return nullptr;
}

bool NativeImageSourceTizenVulkan::ReleaseBuffer(const Rect<uint32_t>& updatedArea)
{
  bool ret = false;

  if(mTbmSurface != nullptr)
  {
    if(mTbmBackSurface)
    {
      if(updatedArea.IsEmpty())
      {
        mUpdatedArea.Set(0, 0, mWidth, mHeight);
      }
      else
      {
        if(mUpdatedArea.IsEmpty())
        {
          mUpdatedArea = updatedArea;
        }
        else
        {
          mUpdatedArea.Merge(updatedArea);
        }
      }
    }

    ret = (tbm_surface_unmap(mTbmSurface) == TBM_SURFACE_ERROR_NONE);
    if(!ret)
    {
      DALI_LOG_ERROR("Fail to unmap tbm_surface\n");
    }

    tbm_surface_internal_unref(mTbmSurface);
    mIsBufferAcquired = false;
  }

  // Unlock the mutex locked by AcquireBuffer.
  mMutex.unlock();
  return ret;
}

void NativeImageSourceTizenVulkan::SetResourceDestructionCallback(EventThreadCallback* callback)
{
  std::scoped_lock lock(mMutex);
  mResourceDestructionCallback = std::unique_ptr<EventThreadCallback>(callback);
}

void NativeImageSourceTizenVulkan::EnableBackBuffer(bool enable)
{
  std::scoped_lock lock(mMutex);
  if(enable != mBackBufferEnabled)
  {
    mBackBufferEnabled = enable;

    if(mBackBufferEnabled)
    {
      CreateBackBuffer();
    }
    else
    {
      DestroyBackBuffer();
    }
  }
}

void NativeImageSourceTizenVulkan::CreateBackBuffer()
{
  if(!mTbmBackSurface && mTbmSurface)
  {
    mTbmBackSurface = tbm_surface_create(mWidth, mHeight, tbm_surface_get_format(mTbmSurface));
  }
}

void NativeImageSourceTizenVulkan::DestroyBackBuffer()
{
  if(mTbmBackSurface)
  {
    if(tbm_surface_destroy(mTbmBackSurface) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Failed to destroy tbm_surface\n");
    }
    mTbmBackSurface = nullptr;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
