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
 */

// EXTERNAL INCLUDES
#include <string>

// INTERNAL INCLUDES
#include "bitmap-loader-impl.h"
#include "image-loaders/image-loader.h"

namespace Dali
{
namespace Internal
{

IntrusivePtr<BitmapLoader> BitmapLoader::New(const std::string& url,
                                             ImageDimensions size,
                                             FittingMode::Type fittingMode,
                                             SamplingMode::Type samplingMode,
                                             bool orientationCorrection)
{
  IntrusivePtr<BitmapLoader> internal = new BitmapLoader( url, size, fittingMode, samplingMode, orientationCorrection );
  return internal;
}

BitmapLoader::BitmapLoader(const std::string& url,
                           ImageDimensions size,
                           FittingMode::Type fittingMode,
                           SamplingMode::Type samplingMode,
                           bool orientationCorrection)
: mResourceType( size, fittingMode, samplingMode, orientationCorrection ),
  mPixelData(),
  mUrl(url)
{
}

BitmapLoader::~BitmapLoader()
{
}

void BitmapLoader::Load()
{
  IntrusivePtr<Dali::RefObject> resource = TizenPlatform::ImageLoader::LoadResourceSynchronously( mResourceType, mUrl );

  if( resource )
  {
    Integration::Bitmap* bitmap = static_cast<Integration::Bitmap*>(resource.Get());

    // Use bitmap->GetBufferOwnership() to transfer the buffer ownership to pixelData.
    // The destroy of bitmap will not release the buffer, instead, the pixelData is responsible for releasing when its reference count falls to zero.
    mPixelData = Dali::PixelData::New( bitmap->GetBufferOwnership(),
                                       bitmap->GetImageWidth(),
                                       bitmap->GetImageHeight(),
                                       bitmap->GetPixelFormat(),
                                       Dali::PixelData::FREE);
  }
}

bool BitmapLoader::IsLoaded()
{
  return mPixelData ? true : false ;
}

const std::string& BitmapLoader::GetUrl() const
{
  return mUrl;
}

Dali::PixelData BitmapLoader::GetPixelData() const
{
  return mPixelData;
}

} // namespace Internal
} // namespace Dali
