/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/windows/native-image-source-impl-win.h>

// EXTERNAL INCLUDES
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
using Dali::Integration::PixelBuffer;

NativeImageSourceWin* NativeImageSourceWin::New(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource)
{
  NativeImageSourceWin* image = new NativeImageSourceWin(width, height, depth, nativeImageSource);
  DALI_ASSERT_DEBUG(image && "NativeImageSource allocation failed.");

  // 2nd phase construction
  if(image) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

NativeImageSourceWin::NativeImageSourceWin(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource)
: mWidth(width),
  mHeight(height),
  mOwnPixmap(true),
  mPixmap(0),
  mBlendingRequired(false),
  mColorDepth(depth),
  mEglImageKHR(NULL),
  mEglGraphics(NULL),
  mEglImageExtensions(NULL),
  mResourceDestructionCallback()
{
  DALI_ASSERT_ALWAYS(Adaptor::IsAvailable());

  GraphicsInterface* graphics = &(Adaptor::GetImplementation(Adaptor::Get()).GetGraphicsInterface());
  mEglGraphics                = static_cast<EglGraphics*>(graphics);

  // assign the pixmap
  mPixmap = GetPixmapFromAny(nativeImageSource);
}

void NativeImageSourceWin::Initialize()
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

NativeImageSourceWin::~NativeImageSourceWin()
{
}

Any NativeImageSourceWin::GetNativeImageSource() const
{
  return Any(mPixmap);
}

bool NativeImageSourceWin::GetPixels(std::vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const
{
  DALI_ASSERT_DEBUG(sizeof(uint32_t) == 4);
  bool success = false;
  width        = mWidth;
  height       = mHeight;

  return success;
}

void NativeImageSourceWin::SetSource(Any source)
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

bool NativeImageSourceWin::IsColorDepthSupported(Dali::NativeImageSource::ColorDepth colorDepth)
{
  return true;
}

bool NativeImageSourceWin::CreateResource()
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

  mEglImageKHR = mEglImageExtensions->CreateImageKHR(eglBuffer);

  return mEglImageKHR != NULL;
}

void NativeImageSourceWin::DestroyResource()
{
  mEglImageExtensions->DestroyImageKHR(mEglImageKHR);

  mEglImageKHR = NULL;

  if(mResourceDestructionCallback)
  {
    mResourceDestructionCallback->Trigger();
  }
}

uint32_t NativeImageSourceWin::TargetTexture()
{
  mEglImageExtensions->TargetTextureKHR(mEglImageKHR);

  return 0;
}

void NativeImageSourceWin::PrepareTexture()
{
}

int NativeImageSourceWin::GetPixelDepth(Dali::NativeImageSource::ColorDepth depth) const
{
  switch(depth)
  {
    case Dali::NativeImageSource::COLOR_DEPTH_DEFAULT:
    {
      return 32;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_8:
    {
      return 8;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_16:
    {
      return 16;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_24:
    {
      return 24;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_32:
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

unsigned int NativeImageSourceWin::GetPixmapFromAny(Any pixmap) const
{
  if(pixmap.Empty())
  {
    return 0;
  }

  // see if it is of type Windows pixmap
  if(pixmap.GetType() == typeid(unsigned int))
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

void NativeImageSourceWin::GetPixmapDetails()
{
}

bool NativeImageSourceWin::ApplyNativeFragmentShader(std::string& shader)
{
  return false;
}

const char* NativeImageSourceWin::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageSourceWin::GetTextureTarget() const
{
  return GL_TEXTURE_2D;
}

Any NativeImageSourceWin::GetNativeImageHandle() const
{
  return mPixmap;
}

bool NativeImageSourceWin::SourceChanged() const
{
  return true;
}

uint8_t* NativeImageSourceWin::AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return NULL;
}

bool NativeImageSourceWin::ReleaseBuffer(const Rect<uint32_t>& updatedArea)
{
  return false;
}

void NativeImageSourceWin::SetResourceDestructionCallback(EventThreadCallback* callback)
{
  mResourceDestructionCallback = std::unique_ptr<EventThreadCallback>(callback);
}

void NativeImageSourceWin::EnableBackBuffer(bool enable)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
