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
#include <dali/internal/imaging/macos/native-image-source-impl-mac.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>

namespace Dali::Internal::Adaptor
{
using Dali::Integration::PixelBuffer;

NativeImageSourceCocoa* NativeImageSourceCocoa::New(
  uint32_t                            width,
  uint32_t                            height,
  Dali::NativeImageSource::ColorDepth depth,
  Any                                 nativeImageSource)
{
  return new NativeImageSourceCocoa(width, height, depth, nativeImageSource);
}

NativeImageSourceCocoa::NativeImageSourceCocoa(
  uint32_t                            width,
  uint32_t                            height,
  Dali::NativeImageSource::ColorDepth depth,
  Any                                 nativeImageSource)
: mImage(MakeRef<CGImageRef>(nullptr)),
  mResourceDestructionCallback()
{
  DALI_ASSERT_ALWAYS(Adaptor::IsAvailable());
  DALI_ASSERT_ALWAYS(nativeImageSource.Empty());

  CFStringRef      colorSpaceName;
  CGImageAlphaInfo alphaInfo;
  std::size_t      bitsPerPixel;

  switch(depth)
  {
    case Dali::NativeImageSource::COLOR_DEPTH_8:
      colorSpaceName = kCGColorSpaceGenericGray;
      alphaInfo      = kCGImageAlphaNone;
      bitsPerPixel   = 8;
      break;
    case Dali::NativeImageSource::COLOR_DEPTH_16:
      colorSpaceName = kCGColorSpaceSRGB;
      alphaInfo      = kCGImageAlphaNone;
      bitsPerPixel   = 16;
      break;
    case Dali::NativeImageSource::COLOR_DEPTH_24:
      colorSpaceName = kCGColorSpaceSRGB;
      alphaInfo      = kCGImageAlphaNone;
      bitsPerPixel   = 24;
      break;
    case Dali::NativeImageSource::COLOR_DEPTH_32:
    default:
      colorSpaceName = kCGColorSpaceSRGB;
      alphaInfo      = kCGImageAlphaLast;
      bitsPerPixel   = 32;
      break;
  }

  // round to next 16 bytes boundary
  std::size_t bytesPerRow = width & ~0xf;
  bytesPerRow             = bytesPerRow ? bytesPerRow + 16 : width;

  auto dataProvider = MakeRef(CGDataProviderCreateWithData(nullptr, nullptr, 0, nullptr));
  auto colorSpace   = MakeRef(CGColorSpaceCreateWithName(colorSpaceName));
  mImage            = MakeRef(CGImageCreate(
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
    kCGRenderingIntentDefault));

  if(mImage)
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
  uint32_t&             width,
  uint32_t&             height,
  Pixel::Format&        pixelFormat) const
{
  width  = static_cast<uint32_t>(CGImageGetWidth(mImage.get()));
  height = static_cast<uint32_t>(CGImageGetHeight(mImage.get()));
  return true;
}

void NativeImageSourceCocoa::SetSource(Any source)
{
}

bool NativeImageSourceCocoa::IsColorDepthSupported(Dali::NativeImageSource::ColorDepth colorDepth)
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

uint32_t NativeImageSourceCocoa::TargetTexture()
{
  return 0;
}

void NativeImageSourceCocoa::PrepareTexture()
{
}

bool NativeImageSourceCocoa::ApplyNativeFragmentShader(std::string& shader)
{
  return false;
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

uint32_t NativeImageSourceCocoa::GetWidth() const
{
  return static_cast<uint32_t>(CGImageGetWidth(mImage.get()));
}

uint32_t NativeImageSourceCocoa::GetHeight() const
{
  return static_cast<uint32_t>(CGImageGetHeight(mImage.get()));
}

bool NativeImageSourceCocoa::RequiresBlending() const
{
  const auto alphaInfo = CGImageGetAlphaInfo(mImage.get());
  return alphaInfo != kCGImageAlphaNone && alphaInfo != kCGImageAlphaNoneSkipFirst && alphaInfo != kCGImageAlphaNoneSkipLast;
}

bool NativeImageSourceCocoa::SourceChanged() const
{
  return true;
}

uint8_t* NativeImageSourceCocoa::AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return nullptr;
}

bool NativeImageSourceCocoa::ReleaseBuffer(const Rect<uint32_t>& updatedArea)
{
  return false;
}

void NativeImageSourceCocoa::SetResourceDestructionCallback(EventThreadCallback* callback)
{
  mResourceDestructionCallback = std::unique_ptr<EventThreadCallback>(callback);
}

void NativeImageSourceCocoa::EnableBackBuffer(bool enable)
{
}

} // namespace Dali::Internal::Adaptor
