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

#define EGL_EGLEXT_PROTOTYPES
#if __ANDROID_API__ < 26
#error "Unsupported Android API version, must be >= 26"
#endif

// CLASS HEADER
#include <dali/internal/imaging/android/native-image-source-impl-android.h>

// EXTERNAL INCLUDES
#include <EGL/egl.h>
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <include/EGL/eglext.h>

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
using Dali::Integration::PixelBuffer;

NativeImageSourceAndroid* NativeImageSourceAndroid::New(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource)
{
  NativeImageSourceAndroid* image = new NativeImageSourceAndroid(width, height, depth, nativeImageSource);
  DALI_ASSERT_DEBUG(image && "NativeImageSource allocation failed.");

  // 2nd phase construction
  if(image) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

NativeImageSourceAndroid::NativeImageSourceAndroid(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource)
: mWidth(width),
  mHeight(height),
  mOwnPixmap(true),
  mPixmap(NULL),
  mBlendingRequired(false),
  mColorDepth(depth),
  mEglImageKHR(NULL),
  mEglGraphics(NULL),
  mEglImageExtensions(NULL),
  mResourceDestructionCallback()
{
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  auto graphics = &(Adaptor::GetImplementation(Adaptor::Get()).GetGraphicsInterface());
  mEglGraphics  = static_cast<EglGraphics*>(graphics);

  // assign the pixmap
  mPixmap = static_cast<AHardwareBuffer*>(GetPixmapFromAny(nativeImageSource));
  if(!mPixmap)
  {
    AHardwareBuffer_Desc bufferDescription;
    memset(&bufferDescription, 0, sizeof(AHardwareBuffer_Desc));
    bufferDescription.width  = width;
    bufferDescription.height = height;
    bufferDescription.layers = 1;
    bufferDescription.usage  = AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN | AHARDWAREBUFFER_USAGE_CPU_READ_RARELY | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    switch(mColorDepth)
    {
      case Dali::NativeImageSource::COLOR_DEPTH_32:
      case Dali::NativeImageSource::COLOR_DEPTH_DEFAULT:
        bufferDescription.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
        break;
      case Dali::NativeImageSource::COLOR_DEPTH_24:
        bufferDescription.format = AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM;
        break;
      case Dali::NativeImageSource::COLOR_DEPTH_16:
        bufferDescription.format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
        break;
      case Dali::NativeImageSource::COLOR_DEPTH_8:
        bufferDescription.format = AHARDWAREBUFFER_FORMAT_BLOB;
        break;
    }

    int ret = AHardwareBuffer_allocate(&bufferDescription, &mPixmap);
    if(ret)
    {
      DALI_LOG_ERROR("Failed to allocate AHardwareBuffer %d", ret);
    }

    mOwnPixmap = true;
  }
}

void NativeImageSourceAndroid::Initialize()
{
  if(mPixmap && !mOwnPixmap)
  {
    AHardwareBuffer_acquire(mPixmap);

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
  }
}

NativeImageSourceAndroid::~NativeImageSourceAndroid()
{
  AHardwareBuffer_release(mPixmap);
  mPixmap = NULL;
}

Any NativeImageSourceAndroid::GetNativeImageSource() const
{
  return Any(mPixmap);
}

bool NativeImageSourceAndroid::GetPixels(std::vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const
{
  DALI_ASSERT_DEBUG(sizeof(uint32_t) == 4);
  bool success = false;

  width  = mWidth;
  height = mHeight;

  AHardwareBuffer_Desc bufferDescription;
  memset(&bufferDescription, 0, sizeof(AHardwareBuffer_Desc));
  AHardwareBuffer_describe(mPixmap, &bufferDescription);
  switch(bufferDescription.format)
  {
    case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
      pixelFormat = Pixel::Format::RGBA8888;
      break;
    case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
      pixelFormat = Pixel::Format::RGB8888;
      break;
    case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
      pixelFormat = Pixel::Format::RGB888;
      break;
    case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
      pixelFormat = Pixel::Format::RGB565;
      break;
    case AHARDWAREBUFFER_FORMAT_BLOB:
    default:
      pixelFormat = Pixel::Format::A8;
      break;
  }

  void* buffer = NULL;
  int   ret    = AHardwareBuffer_lock(mPixmap, AHARDWAREBUFFER_USAGE_CPU_READ_RARELY, -1, NULL, &buffer);
  if(ret != 0)
  {
    DALI_LOG_ERROR("Failed to AHardwareBuffer_lock %d", ret);
    return success;
  }

  uint32_t pixelBytes = GetBytesPerPixel(pixelFormat);
  if(bufferDescription.stride < (pixelBytes * bufferDescription.width))
  {
    //On Android device, bufferDescription.stride doesn't seem to mean (width * pixelbytes)
    //in an actual case, (AHardwareBuffer_Desc) bufferDescription = (width = 1080, height = 1060, layers = 1, format = 1, usage = 306, stride = 1088, rfu0 = 0, rfu1 = 0)
    //deal with situation
    uint32_t dstStride = pixelBytes * bufferDescription.width;
    uint32_t srcStride = pixelBytes * bufferDescription.stride;
    uint32_t size      = dstStride * bufferDescription.height;
    pixbuf.resize(size);
    //copy each row over
    const uint8_t* ptrSrc = reinterpret_cast<const uint8_t*>(buffer);
    uint8_t*       ptrDst = pixbuf.data();
    for(int y = 0; y < bufferDescription.height; y++, ptrSrc += srcStride, ptrDst += dstStride)
    {
      memcpy(ptrDst, ptrSrc, dstStride);
    }
  }
  else
  {
    uint32_t size = bufferDescription.stride * bufferDescription.height;
    pixbuf.resize(size);
    memcpy(pixbuf.data(), buffer, size);
  }

  ret = AHardwareBuffer_unlock(mPixmap, NULL);
  if(ret != 0)
  {
    DALI_LOG_ERROR("failed to AHardwareBuffer_unlock %d", ret);
    return success;
  }
  success = true;

  return success;
}

void NativeImageSourceAndroid::SetSource(Any source)
{
  if(mPixmap)
  {
    mOwnPixmap = false;

    AHardwareBuffer_release(mPixmap);
    mPixmap = NULL;
  }

  mPixmap = static_cast<AHardwareBuffer*>(GetPixmapFromAny(source));

  if(mPixmap)
  {
    // we don't own the pixmap
    mOwnPixmap = false;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
  }
}

bool NativeImageSourceAndroid::IsColorDepthSupported(Dali::NativeImageSource::ColorDepth colorDepth)
{
  return true;
}

bool NativeImageSourceAndroid::CreateResource()
{
  mEglImageExtensions = mEglGraphics->GetImageExtensions();
  DALI_ASSERT_DEBUG(mEglImageExtensions);

  // if the image existed previously delete it.
  if(mEglImageKHR != NULL)
  {
    DestroyResource();
  }

  DALI_ASSERT_ALWAYS(mPixmap);
  EGLClientBuffer eglBuffer = eglGetNativeClientBufferANDROID(mPixmap);
  switch(eglGetError())
  {
    case EGL_SUCCESS:
    {
      break;
    }
    case EGL_BAD_PARAMETER:
    {
      DALI_LOG_ERROR("EGL_BAD_PARAMETER: bad pixmap parameter\n");
      break;
    }
    case EGL_BAD_ACCESS:
    {
      DALI_LOG_ERROR("EGL_BAD_ACCESS: bad access to pixmap\n");
      break;
    }
    case EGL_BAD_ALLOC:
    {
      DALI_LOG_ERROR("EGL_BAD_ALLOC: Insufficient memory is available\n");
      break;
    }
    default:
    {
      DALI_LOG_ERROR("eglGetNativeClientBufferANDROID error\n");
      break;
    }
  }

  DALI_ASSERT_ALWAYS(eglBuffer);
  mEglImageKHR = mEglImageExtensions->CreateImageKHR(eglBuffer);

  return mEglImageKHR != NULL;
}

void NativeImageSourceAndroid::DestroyResource()
{
  mEglImageExtensions->DestroyImageKHR(mEglImageKHR);

  mEglImageKHR = NULL;

  if(mResourceDestructionCallback)
  {
    mResourceDestructionCallback->Trigger();
  }
}

uint32_t NativeImageSourceAndroid::TargetTexture()
{
  mEglImageExtensions->TargetTextureKHR(mEglImageKHR);

  return 0;
}

void NativeImageSourceAndroid::PrepareTexture()
{
}

int NativeImageSourceAndroid::GetTextureTarget() const
{
  return GL_TEXTURE_2D;
}

bool NativeImageSourceAndroid::ApplyNativeFragmentShader(std::string& shader)
{
  return false;
}

const char* NativeImageSourceAndroid::GetCustomSamplerTypename() const
{
  return nullptr;
}

void* NativeImageSourceAndroid::GetPixmapFromAny(Any pixmap) const
{
  if(pixmap.Empty())
  {
    return 0;
  }

  return AnyCast<void*>(pixmap);
}

void NativeImageSourceAndroid::GetPixmapDetails()
{
  // get the width, height and depth
  mBlendingRequired = false;

  AHardwareBuffer_Desc bufferDescription;
  memset(&bufferDescription, 0, sizeof(AHardwareBuffer_Desc));
  AHardwareBuffer_describe(mPixmap, &bufferDescription);

  mWidth  = bufferDescription.width;
  mHeight = bufferDescription.height;
  switch(bufferDescription.format)
  {
    case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
      mColorDepth = Dali::NativeImageSource::COLOR_DEPTH_32;
      break;
    case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
      mColorDepth = Dali::NativeImageSource::COLOR_DEPTH_24;
      break;
    case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
      mColorDepth = Dali::NativeImageSource::COLOR_DEPTH_16;
      break;
    case AHARDWAREBUFFER_FORMAT_BLOB:
    default:
      mColorDepth = Dali::NativeImageSource::COLOR_DEPTH_8;
  }
}

uint8_t* NativeImageSourceAndroid::AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  if(mPixmap)
  {
    AHardwareBuffer_Desc bufferDescription;
    memset(&bufferDescription, 0, sizeof(AHardwareBuffer_Desc));
    AHardwareBuffer_describe(mPixmap, &bufferDescription);

    void* buffer = NULL;
    if(AHardwareBuffer_lock(mPixmap, AHARDWAREBUFFER_USAGE_CPU_READ_RARELY, -1, NULL, &buffer) != 0)
    {
      DALI_LOG_ERROR("Failed to AHardwareBuffer_lock\n");
      return NULL;
    }

    stride = bufferDescription.stride;
    width  = bufferDescription.width;
    height = bufferDescription.height;

    return static_cast<uint8_t*>(buffer);
  }

  return NULL;
}

bool NativeImageSourceAndroid::ReleaseBuffer(const Rect<uint32_t>& updatedArea)
{
  if(mPixmap)
  {
    if(AHardwareBuffer_unlock(mPixmap, NULL) != 0)
    {
      DALI_LOG_ERROR("failed to AHardwareBuffer_unlock\n");
      return false;
    }
    return true;
  }
  return false;
}

void NativeImageSourceAndroid::SetResourceDestructionCallback(EventThreadCallback* callback)
{
  mResourceDestructionCallback = std::unique_ptr<EventThreadCallback>(callback);
}

void NativeImageSourceAndroid::EnableBackBuffer(bool enable)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
