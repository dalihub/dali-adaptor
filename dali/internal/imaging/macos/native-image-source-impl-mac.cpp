/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/macos/native-image-source-impl-mac.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>

namespace Dali::Internal::Adaptor
{

using Dali::Integration::PixelBuffer;

NativeImageSourceCocoa* NativeImageSourceCocoa::New(
  unsigned int width,
  unsigned int height,
  Dali::NativeImageSource::ColorDepth depth,
  Any nativeImageSource
)
{
  return new NativeImageSourceCocoa( width, height, depth, nativeImageSource );
}

NativeImageSourceCocoa::NativeImageSourceCocoa(
  unsigned int width,
  unsigned int height,
  Dali::NativeImageSource::ColorDepth depth,
  Any nativeImageSource
)
: mImage(MakeRef<CGImageRef>(nullptr))
{
  DALI_ASSERT_ALWAYS( Adaptor::IsAvailable() );
  DALI_ASSERT_ALWAYS( nativeImageSource.Empty() );

  CFStringRef colorSpaceName;
  CGImageAlphaInfo alphaInfo;
  std::size_t bitsPerPixel;

  switch (depth)
  {
    case Dali::NativeImageSource::COLOR_DEPTH_8:
      colorSpaceName = kCGColorSpaceGenericGray;
      alphaInfo = kCGImageAlphaNone;
      bitsPerPixel = 8;
      break;
    case Dali::NativeImageSource::COLOR_DEPTH_16:
      colorSpaceName = kCGColorSpaceSRGB;
      alphaInfo = kCGImageAlphaNone;
      bitsPerPixel = 16;
      break;
    case Dali::NativeImageSource::COLOR_DEPTH_24:
      colorSpaceName = kCGColorSpaceSRGB;
      alphaInfo = kCGImageAlphaNone;
      bitsPerPixel = 24;
      break;
    case Dali::NativeImageSource::COLOR_DEPTH_32:
    default:
      colorSpaceName = kCGColorSpaceSRGB;
      alphaInfo = kCGImageAlphaLast;
      bitsPerPixel = 32;
      break;
  }

  // round to next 16 bytes boundary
  std::size_t bytesPerRow = width & ~0xf;
  bytesPerRow = bytesPerRow ? bytesPerRow + 16 : width;

  auto dataProvider = MakeRef(CGDataProviderCreateWithData(nullptr, nullptr, 0, nullptr));
  auto colorSpace = MakeRef(CGColorSpaceCreateWithName(colorSpaceName));
  mImage = MakeRef(CGImageCreate(
    width,
    height,
    8,
    bitsPerPixel,
    bytesPerRow,
    colorSpace.get(),
    alphaInfo,
    dataProvider.get(),
    nullptr,
    true,
    kCGRenderingIntentDefault
  ));

  if (mImage)
  {
    colorSpace.release();
    dataProvider.release();
  }

  DALI_ASSERT_ALWAYS(mImage.get());
}

NativeImageSourceCocoa::~NativeImageSourceCocoa()
{
}

Any NativeImageSourceCocoa::GetNativeImageSource() const
{
  return Any();
}

bool NativeImageSourceCocoa::GetPixels(
  std::vector<uint8_t>& pixbuf,
  unsigned& width, unsigned& height,
  Pixel::Format& pixelFormat
) const
{
  width  = CGImageGetWidth(mImage.get());
  height = CGImageGetHeight(mImage.get());
  return true;
}

void NativeImageSourceCocoa::SetSource( Any source )
{
}

bool NativeImageSourceCocoa::IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth )
{
  return true;
}

bool NativeImageSourceCocoa::CreateResource()
{
  return false;
}

void NativeImageSourceCocoa::DestroyResource()
{
}

unsigned int NativeImageSourceCocoa::TargetTexture()
{
  return 0;
}

void NativeImageSourceCocoa::PrepareTexture()
{
}

const char* NativeImageSourceCocoa::GetCustomFragmentPrefix() const
{
  return nullptr;
}

const char* NativeImageSourceCocoa::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageSourceCocoa::GetTextureTarget() const
{
  return GL_TEXTURE_2D;
}

Any NativeImageSourceCocoa::GetNativeImageHandle() const
{
  return Any(mImage.get());
}

unsigned int NativeImageSourceCocoa::GetWidth() const
{
  return CGImageGetWidth(mImage.get());
}

unsigned int NativeImageSourceCocoa::GetHeight() const
{
  return CGImageGetHeight(mImage.get());
}

bool NativeImageSourceCocoa::RequiresBlending() const
{
  const auto alphaInfo = CGImageGetAlphaInfo(mImage.get());
  return
    alphaInfo != kCGImageAlphaNone
    && alphaInfo != kCGImageAlphaNoneSkipFirst
    && alphaInfo != kCGImageAlphaNoneSkipLast;
}

bool NativeImageSourceCocoa::SourceChanged() const
{
  return false;
}

uint8_t* NativeImageSourceCocoa::AcquireBuffer( uint16_t& width, uint16_t& height, uint16_t& stride )
{
  return nullptr;
}


bool NativeImageSourceCocoa::ReleaseBuffer()
{
  return false;
}

} // namespace Dali::Internal::Adaptor
