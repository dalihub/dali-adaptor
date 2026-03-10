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
#include <dali/public-api/adaptor-framework/native-image.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-factory.h>
#include <dali/internal/imaging/common/native-image-impl.h>

namespace Dali
{
NativeImagePtr NativeImage::New(uint32_t width, uint32_t height, ColorDepth depth)
{
  Any            empty;
  NativeImagePtr image = new NativeImage(width, height, depth, empty);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

NativeImagePtr NativeImage::New(Any nativeImage)
{
  NativeImagePtr image = new NativeImage(0, 0, COLOR_DEPTH_DEFAULT, nativeImage);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

Any NativeImage::GetNativeImage()
{
  return mImpl->GetNativeImage();
}

bool NativeImage::GetPixels(Dali::Vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const
{
  return mImpl->GetPixels(pixbuf, width, height, pixelFormat);
}

bool NativeImage::EncodeToFile(const std::string& filename) const
{
  return mImpl->EncodeToFile(filename);
}

void NativeImage::SetSource(Any source)
{
  mImpl->SetSource(source);
}

bool NativeImage::IsColorDepthSupported(ColorDepth colorDepth)
{
  return mImpl->IsColorDepthSupported(colorDepth);
}

bool NativeImage::CreateResource()
{
  return mImpl->CreateResource();
}

void NativeImage::DestroyResource()
{
  mImpl->DestroyResource();
}

uint32_t NativeImage::TargetTexture()
{
  return mImpl->TargetTexture();
}

Dali::NativeImageInterface::PrepareTextureResult NativeImage::PrepareTexture()
{
  return mImpl->PrepareTexture();
}

uint32_t NativeImage::GetWidth() const
{
  return mImpl->GetWidth();
}

uint32_t NativeImage::GetHeight() const
{
  return mImpl->GetHeight();
}

bool NativeImage::RequiresBlending() const
{
  return mImpl->RequiresBlending();
}

int NativeImage::GetTextureTarget() const
{
  return mImpl->GetTextureTarget();
}

bool NativeImage::ApplyNativeFragmentShader(std::string& shader)
{
  return ApplyNativeFragmentShader(shader, 1);
}

bool NativeImage::ApplyNativeFragmentShader(std::string& shader, int mask)
{
  return mImpl->ApplyNativeFragmentShader(shader, mask);
}

const char* NativeImage::GetCustomSamplerTypename() const
{
  return mImpl->GetCustomSamplerTypename();
}

Any NativeImage::GetNativeImageHandle() const
{
  return mImpl->GetNativeImageHandle();
}

bool NativeImage::SourceChanged() const
{
  return mImpl->SourceChanged();
}

Rect<uint32_t> NativeImage::GetUpdatedArea()
{
  return mImpl->GetUpdatedArea();
}

void NativeImage::PostRender()
{
  mImpl->PostRender();
}

NativeImageInterface::Extension* NativeImage::GetExtension()
{
  return mImpl->GetNativeImageInterfaceExtension();
}

NativeImage::NativeImage(uint32_t width, uint32_t height, ColorDepth depth, Any nativeImage)
{
  auto factory = Dali::Internal::Adaptor::GetNativeImageFactory();
  DALI_ASSERT_DEBUG(factory && "Unable to get NativeImageFactory\n");

  mImpl = factory->CreateNativeImage(width, height, depth, nativeImage).release();
}

NativeImage::~NativeImage()
{
  delete mImpl;
}

} // namespace Dali
