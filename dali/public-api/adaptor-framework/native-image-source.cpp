/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/native-image-source.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-source-factory.h>
#include <dali/internal/imaging/common/native-image-source-impl.h>

namespace Dali
{
NativeImageSourcePtr NativeImageSource::New(uint32_t width, uint32_t height, ColorDepth depth)
{
  Any                  empty;
  NativeImageSourcePtr image = new NativeImageSource(width, height, depth, empty);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

NativeImageSourcePtr NativeImageSource::New(Any nativeImageSource)
{
  NativeImageSourcePtr image = new NativeImageSource(0, 0, COLOR_DEPTH_DEFAULT, nativeImageSource);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

Any NativeImageSource::GetNativeImageSource()
{
  return mImpl->GetNativeImageSource();
}

bool NativeImageSource::GetPixels(std::vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const
{
  return mImpl->GetPixels(pixbuf, width, height, pixelFormat);
}

bool NativeImageSource::EncodeToFile(const std::string& filename) const
{
  return mImpl->EncodeToFile(filename);
}

void NativeImageSource::SetSource(Any source)
{
  mImpl->SetSource(source);
}

bool NativeImageSource::IsColorDepthSupported(ColorDepth colorDepth)
{
  return mImpl->IsColorDepthSupported(colorDepth);
}

bool NativeImageSource::CreateResource()
{
  return mImpl->CreateResource();
}

void NativeImageSource::DestroyResource()
{
  mImpl->DestroyResource();
}

uint32_t NativeImageSource::TargetTexture()
{
  return mImpl->TargetTexture();
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageSource::PrepareTexture()
{
  return mImpl->PrepareTexture();
}

uint32_t NativeImageSource::GetWidth() const
{
  return mImpl->GetWidth();
}

uint32_t NativeImageSource::GetHeight() const
{
  return mImpl->GetHeight();
}

bool NativeImageSource::RequiresBlending() const
{
  return mImpl->RequiresBlending();
}

int NativeImageSource::GetTextureTarget() const
{
  return mImpl->GetTextureTarget();
}

bool NativeImageSource::ApplyNativeFragmentShader(std::string& shader)
{
  return ApplyNativeFragmentShader(shader, 1);
}

bool NativeImageSource::ApplyNativeFragmentShader(std::string& shader, int count)
{
  return mImpl->ApplyNativeFragmentShader(shader, count);
}

const char* NativeImageSource::GetCustomSamplerTypename() const
{
  return mImpl->GetCustomSamplerTypename();
}

Any NativeImageSource::GetNativeImageHandle() const
{
  return mImpl->GetNativeImageHandle();
}

bool NativeImageSource::SourceChanged() const
{
  return mImpl->SourceChanged();
}

Rect<uint32_t> NativeImageSource::GetUpdatedArea()
{
  return mImpl->GetUpdatedArea();
}

void NativeImageSource::PostRender()
{
  mImpl->PostRender();
}

NativeImageInterface::Extension* NativeImageSource::GetExtension()
{
  return mImpl->GetNativeImageInterfaceExtension();
}

NativeImageSource::NativeImageSource(uint32_t width, uint32_t height, ColorDepth depth, Any nativeImageSource)
{
  auto factory = Dali::Internal::Adaptor::GetNativeImageSourceFactory();
  DALI_ASSERT_DEBUG(factory && "Unable to get NativeImageSourceFactory\n");

  mImpl = factory->CreateNativeImageSource(width, height, depth, nativeImageSource).release();
}

NativeImageSource::~NativeImageSource()
{
  delete mImpl;
}

} // namespace Dali
