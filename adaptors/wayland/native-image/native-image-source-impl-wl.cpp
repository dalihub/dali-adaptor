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
#include "native-image-source-impl.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>




namespace Dali
{

namespace Internal
{

namespace Adaptor
{


NativeImageSource* NativeImageSource::New(unsigned int width, unsigned int height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
{
  return NULL;
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
  return 0;
}



} // namespace Adaptor

} // namespace internal

} // namespace Dali
