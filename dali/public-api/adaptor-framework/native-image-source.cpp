/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/common/native-image-source-impl.h>

namespace Dali
{

NativeImageSourcePtr NativeImageSource::New( unsigned int width, unsigned int height, ColorDepth depth )
{
  Any empty;
  NativeImageSourcePtr image = new NativeImageSource( width, height, depth, empty );
  return image;
}

Any NativeImageSource::GetNativeImageSource()
{
  return mImpl->GetNativeImageSource();
}

NativeImageSourcePtr NativeImageSource::New( Any nativeImageSource )
{
  NativeImageSourcePtr image = new NativeImageSource(0, 0, COLOR_DEPTH_DEFAULT, nativeImageSource);
  return image;
}

bool NativeImageSource::GetPixels( std::vector<unsigned char> &pixbuf, unsigned int &width, unsigned int &height, Pixel::Format& pixelFormat ) const
{
  return mImpl->GetPixels( pixbuf, width, height, pixelFormat );
}

bool NativeImageSource::EncodeToFile(const std::string& filename) const
{
  return mImpl->EncodeToFile(filename);
}

void NativeImageSource::SetSource( Any source )
{
  mImpl->SetSource( source );
}

bool NativeImageSource::IsColorDepthSupported( ColorDepth colorDepth )
{
  return mImpl->IsColorDepthSupported( colorDepth );
}

bool NativeImageSource::GlExtensionCreate()
{
  return mImpl->GlExtensionCreate();
}

void NativeImageSource::GlExtensionDestroy()
{
  mImpl->GlExtensionDestroy();
}

unsigned int NativeImageSource::TargetTexture()
{
  return mImpl->TargetTexture();
}

void NativeImageSource::PrepareTexture()
{
  mImpl->PrepareTexture();
}

unsigned int NativeImageSource::GetWidth() const
{
  return mImpl->GetWidth();
}

unsigned int NativeImageSource::GetHeight() const
{
  return mImpl->GetHeight();
}

bool NativeImageSource::RequiresBlending() const
{
  return mImpl->RequiresBlending();
}

NativeImageInterface::Extension* NativeImageSource::GetExtension()
{
  return mImpl->GetNativeImageInterfaceExtension();
}

NativeImageSource::NativeImageSource( unsigned int width, unsigned int height, ColorDepth depth, Any nativeImageSource )
{
   mImpl = Internal::Adaptor::NativeImageSource::New( width, height, depth, nativeImageSource );
}

NativeImageSource::~NativeImageSource()
{
  delete mImpl;
}

} // namespace Dali
