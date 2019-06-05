/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/android/native-image-source-impl-android.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/devel-api/adaptor-framework/bitmap-saver.h>
#include <dali/integration-api/render-surface.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
using Dali::Integration::PixelBuffer;

NativeImageSourceAndroid* NativeImageSourceAndroid::New( uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
{
  NativeImageSourceAndroid* image = new NativeImageSourceAndroid( width, height, depth, nativeImageSource );
  DALI_ASSERT_DEBUG( image && "NativeImageSource allocation failed." );

  // 2nd phase construction
  if( image ) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

NativeImageSourceAndroid::NativeImageSourceAndroid( uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
: mWidth( width ),
  mHeight( height ),
  mOwnPixmap( true ),
  mPixmap( NULL ),
  mBlendingRequired( false ),
  mColorDepth( depth ),
  mEglImageKHR( NULL ),
  mEglImageExtensions( NULL )
{
  DALI_ASSERT_ALWAYS( Adaptor::IsAvailable() );
  DALI_LOG_ERROR("Not supported!\n");

  GraphicsInterface* graphics = &( Adaptor::GetImplementation( Adaptor::Get() ).GetGraphicsInterface() );
  auto eglGraphics = static_cast<EglGraphics *>( graphics );

  mEglImageExtensions = eglGraphics->GetImageExtensions();

  DALI_ASSERT_DEBUG( mEglImageExtensions );

  // assign the pixmap
  mPixmap = static_cast<AHardwareBuffer*>( GetPixmapFromAny( nativeImageSource ) );
}

void NativeImageSourceAndroid::Initialize()
{
  if( mPixmap )
  {
    // we don't own the pixmap
    mOwnPixmap = false;
#if __ANDROID_API__ >= 26
    AHardwareBuffer_acquire( mPixmap ) ;
#endif

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
    return;
  }

  mOwnPixmap = false;
  mWidth = 0;
  mHeight = 0;
  mBlendingRequired = false;
}

NativeImageSourceAndroid::~NativeImageSourceAndroid()
{
#if __ANDROID_API__ >= 26
    AHardwareBuffer_release( mPixmap ) ;
#endif
}

Any NativeImageSourceAndroid::GetNativeImageSource() const
{
  return Any( mPixmap );
}

bool NativeImageSourceAndroid::GetPixels(std::vector<unsigned char>& pixbuf, unsigned& width, unsigned& height, Pixel::Format& pixelFormat) const
{
  DALI_ASSERT_DEBUG( sizeof(unsigned) == 4 );
  bool success = false;
  width  = mWidth;
  height = mHeight;

  // TODO: Implement using AHardwareBuffer_lock / AHardwareBuffer_unlock
  DALI_LOG_ERROR("Not supported!\n");

  return success;
}

bool NativeImageSourceAndroid::EncodeToFile(const std::string& filename) const
{
  std::vector< unsigned char > pixbuf;
  unsigned int width(0), height(0);
  Pixel::Format pixelFormat;

  if( GetPixels( pixbuf, width, height, pixelFormat ) )
  {
    return Dali::EncodeToFile( &pixbuf[0], filename, pixelFormat, width, height );
  }
  return false;
}

void NativeImageSourceAndroid::SetSource( Any source )
{
  mPixmap = static_cast<AHardwareBuffer*>( GetPixmapFromAny( source ) );

  if( mPixmap )
  {
    // we don't own the pixmap
    mOwnPixmap = false;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
  }
}

bool NativeImageSourceAndroid::IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth )
{
  return true;
}

bool NativeImageSourceAndroid::GlExtensionCreate()
{
  DALI_LOG_ERROR("Not supported!\n");

  // if the image existed previously delete it.
  if( mEglImageKHR != NULL )
  {
    GlExtensionDestroy();
  }

/*
  TODO: implement using eglGetNativeClientBufferANDROID

  // casting from an unsigned int to a void *, which should then be cast back
  // to an unsigned int in the driver.
  EGLClientBuffer eglBuffer = reinterpret_cast< EGLClientBuffer >( mPixmap );

  mEglImageKHR = mEglImageExtensions->CreateImageKHR( eglBuffer );
*/
  return mEglImageKHR != NULL;
}

void NativeImageSourceAndroid::GlExtensionDestroy()
{
  mEglImageExtensions->DestroyImageKHR( mEglImageKHR );

  mEglImageKHR = NULL;
}

uint32_t NativeImageSourceAndroid::TargetTexture()
{
  mEglImageExtensions->TargetTextureKHR( mEglImageKHR );

  return 0;
}

void NativeImageSourceAndroid::PrepareTexture()
{
}

int NativeImageSourceAndroid::GetPixelDepth(Dali::NativeImageSource::ColorDepth depth) const
{
  switch (depth)
  {
    case Dali::NativeImageSource::COLOR_DEPTH_DEFAULT:
    {
      // Get the default screen depth
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

void* NativeImageSourceAndroid::GetPixmapFromAny(Any pixmap) const
{
  if( pixmap.Empty() )
  {
    return 0;
  }

  return AnyCast<void*>( pixmap );
}

void NativeImageSourceAndroid::GetPixmapDetails()
{
  // get the width, height and depth
  mWidth = 0;
  mHeight = 0;

  mBlendingRequired = false;

  // TODO: implement using AHardwareBuffer_lockAndGetInfo
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
