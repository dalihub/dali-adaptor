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
#include <dali/internal/imaging/macos/native-image-impl-mac.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali::Internal::Adaptor
{

NativeImageCocoa* NativeImageCocoa::New(
  uint32_t                      width,
  uint32_t                      height,
  Dali::NativeImage::ColorDepth depth,
  Any                           nativeImage)
{
  return new NativeImageCocoa(width, height, depth, nativeImage);
}

NativeImageCocoa::NativeImageCocoa(
  uint32_t                      width,
  uint32_t                      height,
  Dali::NativeImage::ColorDepth depth,
  Any                           nativeImage)
: mImage(MakeRef<CGImageRef>(nullptr)),
  mResourceDestructionCallback(nullptr),
  mOwnResourceDestructionCallback(false)
{
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");
  DALI_ASSERT_ALWAYS(nativeImage.Empty());

  CFStringRef      colorSpaceName;
  CGImageAlphaInfo alphaInfo;
  std::size_t      bitsPerPixel;

  switch(depth)
  {
    case Dali::NativeImage::COLOR_DEPTH_8:
      colorSpaceName = kCGColorSpaceGenericGray;
      alphaInfo      = kCGImageAlphaNone;
      bitsPerPixel   = 8;
      break;
    case Dali::NativeImage::COLOR_DEPTH_16:
      colorSpaceName = kCGColorSpaceSRGB;
      alphaInfo      = kCGImageAlphaNone;
      bitsPerPixel   = 16;
      break;
    case Dali::NativeImage::COLOR_DEPTH_24:
      colorSpaceName = kCGColorSpaceSRGB;
      alphaInfo      = kCGImageAlphaNone;
      bitsPerPixel   = 24;
      break;
    case Dali::NativeImage::COLOR_DEPTH_32:
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

NativeImageCocoa::~NativeImageCocoa()
{
  if(mOwnResourceDestructionCallback)
  {
    delete mResourceDestructionCallback;
  }
}

Any NativeImageCocoa::GetNativeImage() const
{
  return Any();
}

bool NativeImageCocoa::GetPixels(
  Dali::Vector<uint8_t>& pixbuf,
  uint32_t&              width,
  uint32_t&              height,
  Pixel::Format&         pixelFormat) const
{
  width  = static_cast<uint32_t>(CGImageGetWidth(mImage.get()));
  height = static_cast<uint32_t>(CGImageGetHeight(mImage.get()));
  return true;
}

bool NativeImageCocoa::SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat)
{
  return false;
}

void NativeImageCocoa::SetSource(Any source)
{
}

bool NativeImageCocoa::IsColorDepthSupported(Dali::NativeImage::ColorDepth colorDepth)
{
  return true;
}

bool NativeImageCocoa::CreateResource()
{
  return false;
}

void NativeImageCocoa::DestroyResource()
{
  if(mResourceDestructionCallback)
  {
    mResourceDestructionCallback->Trigger();
  }
}

uint32_t NativeImageCocoa::TargetTexture()
{
  return 0;
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageCocoa::PrepareTexture()
{
  return Dali::NativeImageInterface::PrepareTextureResult::NOT_SUPPORTED;
}

bool NativeImageCocoa::ApplyNativeFragmentShader(std::string& shader, int mask)
{
  return false;
}

const char* NativeImageCocoa::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageCocoa::GetTextureTarget() const
{
  return GL_TEXTURE_2D;
}

Any NativeImageCocoa::GetNativeImageHandle() const
{
  return Any(mImage.get());
}

uint32_t NativeImageCocoa::GetWidth() const
{
  return static_cast<uint32_t>(CGImageGetWidth(mImage.get()));
}

uint32_t NativeImageCocoa::GetHeight() const
{
  return static_cast<uint32_t>(CGImageGetHeight(mImage.get()));
}

bool NativeImageCocoa::RequiresBlending() const
{
  const auto alphaInfo = CGImageGetAlphaInfo(mImage.get());
  return alphaInfo != kCGImageAlphaNone && alphaInfo != kCGImageAlphaNoneSkipFirst && alphaInfo != kCGImageAlphaNoneSkipLast;
}

bool NativeImageCocoa::SourceChanged() const
{
  return true;
}

uint8_t* NativeImageCocoa::AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return nullptr;
}

bool NativeImageCocoa::ReleaseBuffer(const Rect<uint32_t>& updatedArea)
{
  return false;
}

void NativeImageCocoa::SetResourceDestructionCallback(EventThreadCallback* callback, bool ownedCallback)
{
  if(mOwnResourceDestructionCallback)
  {
    delete mResourceDestructionCallback;
  }
  mResourceDestructionCallback    = callback;
  mOwnResourceDestructionCallback = ownedCallback;
}

void NativeImageCocoa::EnableBackBuffer(bool enable)
{
}

} // namespace Dali::Internal::Adaptor
