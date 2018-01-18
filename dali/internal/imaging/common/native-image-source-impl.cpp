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

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-source-impl.h>
#include <dali/internal/imaging/common/native-image-source-factory.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

NativeImageSource::~NativeImageSource() = default;

NativeImageSource* NativeImageSource::New(unsigned int width,
                              unsigned int height,
                              Dali::NativeImageSource::ColorDepth depth,
                              Any nativeImageSource)
{
  return Internal::Adaptor::NativeImageSourceFactory::New( width, height, depth, nativeImageSource ).release();
}


Any NativeImageSource::GetNativeImageSource() const
{
  return nullptr;
}


bool NativeImageSource::GetPixels(std::vector<unsigned char> &pixbuf, unsigned int &width, unsigned int &height, Pixel::Format& pixelFormat ) const
{
  return false;
}

bool NativeImageSource::EncodeToFile(const std::string& filename) const
{
  return false;
}

void NativeImageSource::SetSource( Any source )
{

}

bool NativeImageSource::IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth )
{
  return false;
}

bool NativeImageSource::GlExtensionCreate()
{
  return false;
}

void NativeImageSource::GlExtensionDestroy()
{

}

unsigned int NativeImageSource::TargetTexture()
{
  return 0u;
}

void NativeImageSource::PrepareTexture()
{

}

unsigned int NativeImageSource::GetWidth() const
{
  return 0u;
}

unsigned int NativeImageSource::GetHeight() const
{
  return 0u;
}

bool NativeImageSource::RequiresBlending() const
{
  return false;
}


NativeImageInterface::Extension* NativeImageSource::GetNativeImageInterfaceExtension()
{
  return nullptr;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

