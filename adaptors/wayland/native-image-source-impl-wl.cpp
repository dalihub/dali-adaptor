/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "native-image-source-impl.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <gl/egl-image-extensions.h>
#include <gl/egl-factory.h>
#include <adaptor-impl.h>
#include <render-surface.h>

// Allow this to be encoded and saved:
#include <platform-abstractions/tizen/resource-loader/resource-loader.h>
#include <bitmap-saver.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
using Dali::Integration::PixelBuffer;

NativeImageSource* NativeImageSource::New(unsigned int width, unsigned int height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
{
  NativeImageSource* image = new NativeImageSource( width, height, depth, nativeImageSource );
  DALI_ASSERT_DEBUG( image && "NativeImageSource allocation failed." );

  // 2nd phase construction
  if(image) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

NativeImageSource::NativeImageSource( unsigned int width, unsigned int height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
: mWidth( width ),
  mHeight( height ),
  mOwnPixmap( true ),
  mColorDepth( depth ),
  mEglImageKHR( NULL ),
  mEglImageExtensions( NULL )
{
  DALI_ASSERT_ALWAYS( Adaptor::IsAvailable() );
  EglFactory& eglFactory = Adaptor::GetImplementation( Adaptor::Get() ).GetEGLFactory();
  mEglImageExtensions = eglFactory.GetImageExtensions();
  SetBlending( mColorDepth );

  DALI_ASSERT_DEBUG( mEglImageExtensions );
}

void NativeImageSource::Initialize()
{
}

NativeImageSource::~NativeImageSource()
{
}

Any NativeImageSource::GetNativeImageSource() const
{
  DALI_ASSERT_ALWAYS( false && "NativeImageSource::GetNativeImageSource() is not supported for Wayland." );
  return Any();
}

bool NativeImageSource::GetPixels(std::vector<unsigned char>& pixbuf, unsigned& width, unsigned& height, Pixel::Format& pixelFormat) const
{
    return false;
}

bool NativeImageSource::EncodeToFile(const std::string& filename) const
{
  std::vector< unsigned char > pixbuf;
  unsigned int width(0), height(0);
  Pixel::Format pixelFormat;

  if(GetPixels(pixbuf, width, height, pixelFormat))
  {
    return Dali::EncodeToFile(&pixbuf[0], filename, pixelFormat, width, height);
  }
  return false;
}

bool NativeImageSource::GlExtensionCreate()
{
    return false;
}

void NativeImageSource::GlExtensionDestroy()
{
  mEglImageExtensions->DestroyImageKHR(mEglImageKHR);

  mEglImageKHR = NULL;
}

unsigned int NativeImageSource::TargetTexture()
{
  mEglImageExtensions->TargetTextureKHR(mEglImageKHR);

  return 0;
}

void NativeImageSource::SetBlending(Dali::NativeImageSource::ColorDepth depth)
{
  switch (depth)
  {
    case Dali::NativeImageSource::COLOR_DEPTH_16: //Pixel::RGB565
    case Dali::NativeImageSource::COLOR_DEPTH_24: // Pixel::RGB888
    {
      mBlendingRequired = false;
      break;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_8: //Pixel::A8
    case Dali::NativeImageSource::COLOR_DEPTH_32: // Pixel::RGBA8888
    {
      mBlendingRequired = true;
      break;
    }
    default:
    {
      DALI_ASSERT_DEBUG(0 && "unknown color enum");
    }
  }
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
