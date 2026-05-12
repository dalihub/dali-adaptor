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
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

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
  mOwnResourceDestructionCallback(false)
{
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

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
  DALI_ASSERT_DEBUG(sizeof(uint32_t) == 4);
  bool success = false;
  width        = mWidth;
  height       = mHeight;

  return success;
}

bool NativeImageWin::SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat)
{
  return false;
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
  mEglImageExtensions->DestroyImageKHR(mEglImageKHR);

  mEglImageKHR     = NULL;
  mEglImageChanged = true;

  if(mResourceDestructionCallback)
  {
    mResourceDestructionCallback->Trigger();
  }
}

uint32_t NativeImageWin::TargetTexture()
{
  mEglImageExtensions->TargetTextureKHR(mEglImageKHR);

  return 0;
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageWin::PrepareTexture()
{
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
