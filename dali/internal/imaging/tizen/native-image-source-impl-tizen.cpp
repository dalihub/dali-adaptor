/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/tizen/native-image-source-impl-tizen.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>
#include <tbm_surface_internal.h>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
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
const char* SAMPLER_TYPE = "samplerExternalOES";

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

NativeImageSourceTizen* NativeImageSourceTizen::New(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource)
{
  NativeImageSourceTizen* image = new NativeImageSourceTizen(width, height, depth, nativeImageSource);
  DALI_ASSERT_DEBUG(image && "NativeImageSource allocation failed.");

  if(image)
  {
    image->Initialize();
  }

  return image;
}

NativeImageSourceTizen::NativeImageSourceTizen(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource)
: mWidth(width),
  mHeight(height),
  mOwnTbmSurface(false),
  mTbmSurface(NULL),
  mTbmFormat(0),
  mBlendingRequired(false),
  mColorDepth(depth),
  mEglImageKHR(NULL),
  mEglGraphics(NULL),
  mEglImageExtensions(NULL),
  mSetSource(false),
  mMutex(),
  mIsBufferAcquired(false),
  mResourceDestructionCallback()

{
  DALI_ASSERT_ALWAYS(Adaptor::IsAvailable());

  GraphicsInterface* graphics = &(Adaptor::GetImplementation(Adaptor::Get()).GetGraphicsInterface());
  mEglGraphics                = static_cast<EglGraphics*>(graphics);

  mTbmSurface = GetSurfaceFromAny(nativeImageSource);

  if(mTbmSurface != NULL)
  {
    tbm_surface_internal_ref(mTbmSurface);
    mBlendingRequired = CheckBlending(tbm_surface_get_format(mTbmSurface));
    mWidth            = tbm_surface_get_width(mTbmSurface);
    mHeight           = tbm_surface_get_height(mTbmSurface);
  }
}

void NativeImageSourceTizen::Initialize()
{
  if(mTbmSurface != NULL || mWidth == 0 || mHeight == 0)
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

tbm_surface_h NativeImageSourceTizen::GetSurfaceFromAny(Any source) const
{
  if(source.Empty())
  {
    return NULL;
  }

  if(source.GetType() == typeid(tbm_surface_h))
  {
    return AnyCast<tbm_surface_h>(source);
  }
  else
  {
    return NULL;
  }
}

void NativeImageSourceTizen::DestroySurface()
{
  if(mTbmSurface)
  {
    if(mIsBufferAcquired)
    {
      ReleaseBuffer();
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
  }
}

NativeImageSourceTizen::~NativeImageSourceTizen()
{
  DestroySurface();
}

Any NativeImageSourceTizen::GetNativeImageSource() const
{
  return Any(mTbmSurface);
}

bool NativeImageSourceTizen::GetPixels(std::vector<unsigned char>& pixbuf, unsigned& width, unsigned& height, Pixel::Format& pixelFormat) const
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(mTbmSurface != NULL)
  {
    tbm_surface_info_s surface_info;

    if(tbm_surface_map(mTbmSurface, TBM_SURF_OPTION_READ, &surface_info) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to map tbm_surface\n");

      width  = 0;
      height = 0;

      return false;
    }

    tbm_format     format = surface_info.format;
    uint32_t       stride = surface_info.planes[0].stride;
    unsigned char* ptr    = surface_info.planes[0].ptr;

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
        unsigned char* bufptr = &pixbuf[0];

        for(unsigned int r = 0; r < height; ++r, bufptr += lineSize)
        {
          for(unsigned int c = 0; c < width; ++c)
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
        unsigned char* bufptr = &pixbuf[0];

        for(unsigned int r = 0; r < height; ++r, bufptr += lineSize)
        {
          for(unsigned int c = 0; c < width; ++c)
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
        unsigned char* bufptr = &pixbuf[0];

        for(unsigned int r = 0; r < height; ++r, bufptr += lineSize)
        {
          for(unsigned int c = 0; c < width; ++c)
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

  DALI_LOG_WARNING("TBM surface does not exist.\n");

  width  = 0;
  height = 0;

  return false;
}

void NativeImageSourceTizen::SetSource(Any source)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  DestroySurface();

  mOwnTbmSurface = false;
  mTbmSurface    = GetSurfaceFromAny(source);

  if(mTbmSurface != NULL)
  {
    mSetSource = true;
    tbm_surface_internal_ref(mTbmSurface);
    mBlendingRequired = CheckBlending(tbm_surface_get_format(mTbmSurface));
    mWidth            = tbm_surface_get_width(mTbmSurface);
    mHeight           = tbm_surface_get_height(mTbmSurface);
  }
}

bool NativeImageSourceTizen::IsColorDepthSupported(Dali::NativeImageSource::ColorDepth colorDepth)
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
    for(unsigned int i = 0; i < formatNum; i++)
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

bool NativeImageSourceTizen::CreateResource()
{
  // If an EGL image exists, use it as it is without creating it.
  if(mEglImageKHR != NULL)
  {
    return true;
  }

  // casting from an unsigned int to a void *, which should then be cast back
  // to an unsigned int in the driver.
  EGLClientBuffer eglBuffer = reinterpret_cast<EGLClientBuffer>(mTbmSurface);
  if(!eglBuffer || !tbm_surface_internal_is_valid(mTbmSurface))
  {
    return false;
  }

  mEglImageExtensions = mEglGraphics->GetImageExtensions();
  DALI_ASSERT_DEBUG(mEglImageExtensions);

  mEglImageKHR = mEglImageExtensions->CreateImageKHR(eglBuffer);

  return mEglImageKHR != NULL;
}

void NativeImageSourceTizen::DestroyResource()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(mEglImageKHR)
  {
    DALI_ASSERT_DEBUG(mEglImageExtensions);
    mEglImageExtensions->DestroyImageKHR(mEglImageKHR);

    mEglImageKHR = NULL;
  }

  if(mResourceDestructionCallback)
  {
    mResourceDestructionCallback->Trigger();
  }
}

uint32_t NativeImageSourceTizen::TargetTexture()
{
  if(DALI_LIKELY(mEglImageExtensions && mEglImageKHR))
  {
    mEglImageExtensions->TargetTextureKHR(mEglImageKHR);
  }

  return 0;
}

void NativeImageSourceTizen::PrepareTexture()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(mSetSource)
  {
    // Destroy previous eglImage because use for new one.
    // if mEglImageKHR is not to be NULL here, it will not be updated with a new eglImage.
    if(mEglImageKHR)
    {
      DALI_ASSERT_DEBUG(mEglImageExtensions);
      mEglImageExtensions->DestroyImageKHR(mEglImageKHR);
      mEglImageKHR = NULL;
    }

    if(CreateResource())
    {
      TargetTexture();
    }

    mSetSource = false;
  }
}

bool NativeImageSourceTizen::ApplyNativeFragmentShader(std::string& shader)
{
  return mEglGraphics->ApplyNativeFragmentShader(shader, SAMPLER_TYPE);
}

const char* NativeImageSourceTizen::GetCustomSamplerTypename() const
{
  return SAMPLER_TYPE;
}

int NativeImageSourceTizen::GetTextureTarget() const
{
  return GL_TEXTURE_EXTERNAL_OES;
}

Any NativeImageSourceTizen::GetNativeImageHandle() const
{
  return GetNativeImageSource();
}

bool NativeImageSourceTizen::SourceChanged() const
{
  return false;
}

bool NativeImageSourceTizen::CheckBlending(tbm_format format)
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

uint8_t* NativeImageSourceTizen::AcquireBuffer(uint16_t& width, uint16_t& height, uint16_t& stride)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(mTbmSurface != NULL)
  {
    tbm_surface_info_s info;

    if(tbm_surface_map(mTbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &info) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("Fail to map tbm_surface\n");

      width  = 0;
      height = 0;

      return NULL;
    }
    tbm_surface_internal_ref(mTbmSurface);
    mIsBufferAcquired = true;

    stride = info.planes[0].stride;
    width  = mWidth;
    height = mHeight;

    return info.planes[0].ptr;
  }
  return NULL;
}

bool NativeImageSourceTizen::ReleaseBuffer()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  bool                    ret = false;
  if(mTbmSurface != NULL)
  {
    ret = (tbm_surface_unmap(mTbmSurface) == TBM_SURFACE_ERROR_NONE);
    if(!ret)
    {
      DALI_LOG_ERROR("Fail to unmap tbm_surface\n");
    }
    tbm_surface_internal_unref(mTbmSurface);
    mIsBufferAcquired = false;
  }
  return ret;
}

void NativeImageSourceTizen::SetResourceDestructionCallback(EventThreadCallback* callback)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mResourceDestructionCallback = std::unique_ptr<EventThreadCallback>(callback);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
