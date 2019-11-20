/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>
#include <dali/devel-api/adaptor-framework/bitmap-saver.h>
#include <dali/integration-api/render-surface.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
using Dali::Integration::PixelBuffer;

NativeImageSourceWin* NativeImageSourceWin::New(unsigned int width, unsigned int height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
{
  NativeImageSourceWin* image = new NativeImageSourceWin( width, height, depth, nativeImageSource );
  DALI_ASSERT_DEBUG( image && "NativeImageSource allocation failed." );

  // 2nd phase construction
  if(image) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

NativeImageSourceWin::NativeImageSourceWin( unsigned int width, unsigned int height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
: mWidth( width ),
  mHeight( height ),
  mOwnPixmap( true ),
  mPixmap( 0 ),
  mBlendingRequired( false ),
  mColorDepth( depth ),
  mEglImageKHR( NULL ),
  mEglImageExtensions( NULL )
{
  DALI_ASSERT_ALWAYS( Adaptor::IsAvailable() );

  GraphicsInterface* graphics = &( Adaptor::GetImplementation( Adaptor::Get() ).GetGraphicsInterface() );
  auto eglGraphics = static_cast<EglGraphics *>(graphics);

  mEglImageExtensions = eglGraphics->GetImageExtensions();

  DALI_ASSERT_DEBUG( mEglImageExtensions );

  // assign the pixmap
  mPixmap = GetPixmapFromAny(nativeImageSource);
}

void NativeImageSourceWin::Initialize()
{
  // if pixmap has been created outside of Windows Image we can return
  if (mPixmap)
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
  mBlendingRequired = ( depth == 32 || depth == 8 );
}

NativeImageSourceWin::~NativeImageSourceWin()
{
}

Any NativeImageSourceWin::GetNativeImageSource() const
{
  return Any(mPixmap);
}

bool NativeImageSourceWin::GetPixels(std::vector<uint8_t>& pixbuf, unsigned& width, unsigned& height, Pixel::Format& pixelFormat) const
{
  DALI_ASSERT_DEBUG(sizeof(unsigned) == 4);
  bool success = false;
  width  = mWidth;
  height = mHeight;

  return success;
}

bool NativeImageSourceWin::EncodeToFile(const std::string& filename) const
{
  std::vector< uint8_t > pixbuf;
  uint32_t width(0), height(0);
  Pixel::Format pixelFormat;

  if(GetPixels(pixbuf, width, height, pixelFormat))
  {
    return Dali::EncodeToFile(&pixbuf[0], filename, pixelFormat, width, height);
  }
  return false;
}

void NativeImageSourceWin::SetSource( Any source )
{
  mPixmap = GetPixmapFromAny( source );

  if (mPixmap)
  {
    // we don't own the pixmap
    mOwnPixmap = false;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
  }
}

bool NativeImageSourceWin::IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth )
{
  return true;
}

bool NativeImageSourceWin::GlExtensionCreate()
{
  // if the image existed previously delete it.
  if (mEglImageKHR != NULL)
  {
    GlExtensionDestroy();
  }

  // casting from an unsigned int to a void *, which should then be cast back
  // to an unsigned int in the driver.
  EGLClientBuffer eglBuffer = reinterpret_cast< EGLClientBuffer > (mPixmap);

  mEglImageKHR = mEglImageExtensions->CreateImageKHR( eglBuffer );

  return mEglImageKHR != NULL;
}

void NativeImageSourceWin::GlExtensionDestroy()
{
  mEglImageExtensions->DestroyImageKHR(mEglImageKHR);

  mEglImageKHR = NULL;
}

unsigned int NativeImageSourceWin::TargetTexture()
{
  mEglImageExtensions->TargetTextureKHR(mEglImageKHR);

  return 0;
}

void NativeImageSourceWin::PrepareTexture()
{
}

int NativeImageSourceWin::GetPixelDepth(Dali::NativeImageSource::ColorDepth depth) const
{
  switch (depth)
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
  if (pixmap.Empty())
  {
    return 0;
  }

  // see if it is of type Windows pixmap
  if (pixmap.GetType() == typeid ( unsigned int ))
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

} // namespace Adaptor

} // namespace internal

} // namespace Dali
