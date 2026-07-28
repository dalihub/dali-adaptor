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
#include <dali/internal/imaging/windows/native-image-impl-win.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

#if defined(_WIN32)
#undef NO_ERROR
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

NativeImageWin* NativeImageWin::New(uint32_t width, uint32_t height, Dali::NativeImage::ColorDepth depth, Any nativeImage)
{
  NativeImageWin* image = new NativeImageWin(width, height, depth, nativeImage);
  DALI_ASSERT_DEBUG(image && "NativeImage allocation failed.");

  // 2nd phase construction
  if(image) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

NativeImageWin::NativeImageWin(uint32_t width, uint32_t height, Dali::NativeImage::ColorDepth depth, Any nativeImage)
: mWidth(width),
  mHeight(height),
  mOwnPixmap(true),
  mPixmap(0),
  mBlendingRequired(false),
  mColorDepth(depth),
  mEglImageChanged(false),
  mEglImageKHR(NULL),
  mEglGraphics(NULL),
  mEglImageExtensions(NULL),
  mResourceDestructionCallback(nullptr),
  mOwnResourceDestructionCallback(false),
  mUseCpuBuffer(false)
{
  DALI_ASSERT_ALWAYS(Dali::Adaptor::IsEventThread() && "Must be called from the event thread!");

  auto graphics = &(Adaptor::GetImplementation(Adaptor::Get()).GetGraphicsInterface());
  mEglGraphics  = static_cast<EglGraphics*>(graphics);

  // assign the pixmap
  mPixmap = GetPixmapFromAny(nativeImage);
}

void NativeImageWin::Initialize()
{
  // if pixmap has been created outside of Windows Image we can return
  if(mPixmap)
  {
    // we don't own the pixmap
    mOwnPixmap = false;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
    return;
  }

  // get the pixel depth
  int depth = GetPixelDepth(mColorDepth);

  // set whether blending is required according to pixel format based on the depth
  /* default pixel format is RGB888
     If depth = 8, Pixel::A8;
     If depth = 16, Pixel::RGB565;
     If depth = 32, Pixel::RGBA8888 */
  mBlendingRequired = (depth == 32 || depth == 8);
}

NativeImageWin::~NativeImageWin()
{
  if(mOwnResourceDestructionCallback)
  {
    delete mResourceDestructionCallback;
  }
}

Any NativeImageWin::GetNativeImage() const
{
  return Any(mPixmap);
}

bool NativeImageWin::GetPixels(Dali::Vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const
{
  width  = mWidth;
  height = mHeight;

  // The Windows backend has no pixmap read-back (the EGLImage path is not wired up under
  // ANGLE), so pixels can only be returned from the CPU buffer populated by SetPixels().
  std::lock_guard<std::mutex> lock(mCpuBufferMutex);
  if(mCpuBuffer.empty())
  {
    return false;
  }

  pixelFormat = Pixel::RGBA8888;
  pixbuf.Resize(mCpuBuffer.size());
  memcpy(pixbuf.Begin(), mCpuBuffer.data(), mCpuBuffer.size());
  return true;
}

bool NativeImageWin::SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat)
{
  if(!pixbuf)
  {
    return false;
  }

  // TargetTexture() uploads the native-image pixels as GL_RGBA, so keep a normalised
  // RGBA8888 copy here, converting from the supported input formats.
  const size_t pixelCount = static_cast<size_t>(mWidth) * static_cast<size_t>(mHeight);

  std::lock_guard<std::mutex> lock(mCpuBufferMutex);
  mCpuBuffer.resize(pixelCount * 4u);

  switch(pixelFormat)
  {
    case Pixel::RGBA8888:
    {
      memcpy(mCpuBuffer.data(), pixbuf, pixelCount * 4u);
      break;
    }
    case Pixel::BGRA8888:
    {
      for(size_t i = 0u; i < pixelCount; ++i)
      {
        const size_t offset      = i * 4u;
        mCpuBuffer[offset + 0u] = pixbuf[offset + 2u]; // R <- B
        mCpuBuffer[offset + 1u] = pixbuf[offset + 1u]; // G
        mCpuBuffer[offset + 2u] = pixbuf[offset + 0u]; // B <- R
        mCpuBuffer[offset + 3u] = pixbuf[offset + 3u]; // A
      }
      break;
    }
    case Pixel::RGB888:
    {
      for(size_t i = 0u; i < pixelCount; ++i)
      {
        mCpuBuffer[i * 4u + 0u] = pixbuf[i * 3u + 0u];
        mCpuBuffer[i * 4u + 1u] = pixbuf[i * 3u + 1u];
        mCpuBuffer[i * 4u + 2u] = pixbuf[i * 3u + 2u];
        mCpuBuffer[i * 4u + 3u] = 0xFFu;
      }
      break;
    }
    default:
    {
      DALI_LOG_ERROR("NativeImageWin::SetPixels: unsupported pixel format %d\n", static_cast<int>(pixelFormat));
      mCpuBuffer.clear();
      return false;
    }
  }

  mUseCpuBuffer    = true;
  mEglImageChanged = true; // PrepareTexture() reports IMAGE_CHANGED so the backend re-uploads.
  return true;
}

void NativeImageWin::SetSource(Any source)
{
  mPixmap = GetPixmapFromAny(source);

  if(mPixmap)
  {
    // we don't own the pixmap
    mOwnPixmap = false;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
  }
}

bool NativeImageWin::IsColorDepthSupported(Dali::NativeImage::ColorDepth colorDepth)
{
  return true;
}

bool NativeImageWin::CreateResource()
{
  // CPU-buffer path: no EGLImage. The pixels are uploaded to a plain GL_TEXTURE_2D in
  // TargetTexture(); the backend still creates and binds the GL texture for us.
  if(mUseCpuBuffer || mPixmap == 0)
  {
    return true;
  }

  mEglImageExtensions = mEglGraphics->GetImageExtensions();
  DALI_ASSERT_DEBUG(mEglImageExtensions);

  // if the image existed previously delete it.
  if(mEglImageKHR != NULL)
  {
    DestroyResource();
  }

  // casting from an unsigned int to a void *, which should then be cast back
  // to an unsigned int in the driver.
  EGLClientBuffer eglBuffer = reinterpret_cast<EGLClientBuffer>(mPixmap);

  mEglImageKHR     = mEglImageExtensions->CreateImageKHR(eglBuffer);
  mEglImageChanged = true;

  return mEglImageKHR != NULL;
}

void NativeImageWin::DestroyResource()
{
  // In the CPU-buffer path mEglImageExtensions is never assigned, so guard the destroy.
  if(mEglImageExtensions && mEglImageKHR)
  {
    mEglImageExtensions->DestroyImageKHR(mEglImageKHR);
  }

  mEglImageKHR     = NULL;
  mEglImageChanged = true;

  if(mResourceDestructionCallback)
  {
    mResourceDestructionCallback->Trigger();
  }
}

uint32_t NativeImageWin::TargetTexture()
{
  // CPU-buffer path: upload the pixels into the currently bound GL_TEXTURE_2D.
  if(mUseCpuBuffer || mPixmap == 0)
  {
    std::lock_guard<std::mutex> lock(mCpuBufferMutex);
    if(!mCpuBuffer.empty())
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(mWidth), static_cast<GLsizei>(mHeight), 0, GL_RGBA, GL_UNSIGNED_BYTE, mCpuBuffer.data());
    }
    return 0;
  }

  mEglImageExtensions->TargetTextureKHR(mEglImageKHR);

  return 0;
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageWin::PrepareTexture()
{
  // CPU-buffer path: always ready. Report IMAGE_CHANGED after a SetPixels() so the backend
  // re-binds and calls TargetTexture() to re-upload.
  if(mUseCpuBuffer || mPixmap == 0)
  {
    Dali::NativeImageInterface::PrepareTextureResult cpuResult =
      mEglImageChanged ? Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED
                       : Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR;
    mEglImageChanged = false;
    return cpuResult;
  }

  Dali::NativeImageInterface::PrepareTextureResult result = Dali::NativeImageInterface::PrepareTextureResult::UNKNOWN_ERROR;
  if(DALI_LIKELY(mEglImageKHR))
  {
    result           = mEglImageChanged ? Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED : Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR;
    mEglImageChanged = false;
  }
  else
  {
    result = mEglImageExtensions ? Dali::NativeImageInterface::PrepareTextureResult::NOT_INITIALIZED_GRAPHICS : Dali::NativeImageInterface::PrepareTextureResult::NOT_INITIALIZED_IMAGE;
  }

  return result;
}

int NativeImageWin::GetPixelDepth(Dali::NativeImage::ColorDepth depth) const
{
  switch(depth)
  {
    case Dali::NativeImage::COLOR_DEPTH_DEFAULT:
    {
      return 32;
    }
    case Dali::NativeImage::COLOR_DEPTH_8:
    {
      return 8;
    }
    case Dali::NativeImage::COLOR_DEPTH_16:
    {
      return 16;
    }
    case Dali::NativeImage::COLOR_DEPTH_24:
    {
      return 24;
    }
    case Dali::NativeImage::COLOR_DEPTH_32:
    {
      return 32;
    }
    default:
    {
      DALI_ASSERT_DEBUG(0 && "unknown color enum");
      return 0;
    }
  }
}

unsigned int NativeImageWin::GetPixmapFromAny(Any pixmap) const
{
  if(pixmap.Empty())
  {
    return 0;
  }

  // see if it is of type Windows pixmap
  if(pixmap.IsType<unsigned int>())
  {
    // get the Windows pixmap type
    unsigned int xpixmap = AnyCast<unsigned int>(pixmap);

    // cast it to a Windows pixmap type
    return static_cast<unsigned int>(xpixmap);
  }
  else
  {
    return AnyCast<unsigned int>(pixmap);
  }
}

void NativeImageWin::GetPixmapDetails()
{
}

bool NativeImageWin::ApplyNativeFragmentShader(std::string& shader, int mask)
{
  return false;
}

const char* NativeImageWin::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageWin::GetTextureTarget() const
{
  return GL_TEXTURE_2D;
}

Any NativeImageWin::GetNativeImageHandle() const
{
  return mPixmap;
}

bool NativeImageWin::SourceChanged() const
{
  return true;
}

uint8_t* NativeImageWin::AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return NULL;
}

bool NativeImageWin::ReleaseBuffer(const Rect<uint32_t>& updatedArea)
{
  return false;
}

void NativeImageWin::SetResourceDestructionCallback(EventThreadCallback* callback, bool ownedCallback)
{
  if(mOwnResourceDestructionCallback)
  {
    delete mResourceDestructionCallback;
  }
  mResourceDestructionCallback    = callback;
  mOwnResourceDestructionCallback = ownedCallback;
}

void NativeImageWin::EnableBackBuffer(bool enable)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
