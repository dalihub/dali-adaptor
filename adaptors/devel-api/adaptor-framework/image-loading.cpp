/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
 */

// CLASS HEADER
#include "image-loading.h"

// INTERNAL INCLUDES
#include "image-loaders/image-loader.h"

namespace Dali
{

PixelData LoadImageFromFile( const std::string& url, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection )
{
  Integration::BitmapResourceType resourceType( size, fittingMode, samplingMode, orientationCorrection );
  IntrusivePtr<Dali::RefObject> resource = TizenPlatform::ImageLoader::LoadResourceSynchronously( resourceType, url );

  if( resource )
  {
    Integration::Bitmap* bitmap = static_cast<Integration::Bitmap*>( resource.Get() );

    // Use bitmap->GetBufferOwnership() to transfer the buffer ownership to pixelData.
    // The destroy of bitmap will not release the buffer, instead, the pixelData is responsible for releasing when its reference count falls to zero.
    return Dali::PixelData::New( bitmap->GetBufferOwnership(),
                                 bitmap->GetBufferSize(),
                                 bitmap->GetImageWidth(),
                                 bitmap->GetImageHeight(),
                                 bitmap->GetPixelFormat(),
                                 Dali::PixelData::FREE );
  }
  return Dali::PixelData();
}

} // namespace Dali
