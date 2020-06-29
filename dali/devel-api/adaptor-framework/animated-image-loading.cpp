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
#include <dali/devel-api/adaptor-framework/animated-image-loading.h>

// INTERNAL HEADER
#include <dali/internal/imaging/common/animated-image-loading-impl.h>
#include <dali/internal/imaging/common/gif-loading.h>
#include <dali/internal/imaging/common/webp-loading.h>

namespace Dali
{

AnimatedImageLoading::AnimatedImageLoading()
{
}

AnimatedImageLoading AnimatedImageLoading::New( const std::string& url, bool isLocalResource )
{
  const std::size_t urlSize = url.length();

  Internal::Adaptor::AnimatedImageLoadingPtr internal = NULL;
  if(urlSize >= 4){ // Avoid throwing out_of_range or failing silently if exceptions are turned-off on the compare(). (http://www.cplusplus.com/reference/string/string/compare/)
    if( !url.compare( urlSize - 4, 4, ".gif" )
        || !url.compare( urlSize - 4, 4, ".GIF" ) )
    {
      internal = Internal::Adaptor::GifLoading::New( url, isLocalResource );
    }
  }
  if(urlSize >= 5){ // Avoid throwing out_of_range or failing silently if exceptions are turned-off on the compare(). (http://www.cplusplus.com/reference/string/string/compare/)
    if( !url.compare( urlSize - 5, 5, ".webp" )
        || !url.compare( urlSize - 5, 5, ".WEBP" ) )
    {
      internal = Internal::Adaptor::WebPLoading::New( url, isLocalResource );
    }
  }

  return AnimatedImageLoading( internal.Get() );
}

AnimatedImageLoading AnimatedImageLoading::DownCast( BaseHandle handle )
{
  return AnimatedImageLoading( dynamic_cast< Internal::Adaptor::AnimatedImageLoading* >( handle.GetObjectPtr() ) );
}

AnimatedImageLoading::~AnimatedImageLoading()
{
}

bool AnimatedImageLoading::LoadNextNFrames( uint32_t frameStartIndex, int count, std::vector<Dali::PixelData>& pixelData )
{
  return GetImplementation( *this ).LoadNextNFrames( frameStartIndex, count, pixelData );
}

Dali::Devel::PixelBuffer AnimatedImageLoading::LoadFrame( uint32_t frameIndex )
{
  return GetImplementation( *this ).LoadFrame( frameIndex );
}

ImageDimensions AnimatedImageLoading::GetImageSize() const
{
  return GetImplementation( *this ).GetImageSize();
}

uint32_t AnimatedImageLoading::GetImageCount() const
{
  return GetImplementation( *this ).GetImageCount();
}

uint32_t AnimatedImageLoading::GetFrameInterval( uint32_t frameIndex ) const
{
  return GetImplementation( *this ).GetFrameInterval( frameIndex );
}

std::string AnimatedImageLoading::GetUrl() const
{
  return GetImplementation( *this ).GetUrl();
}

AnimatedImageLoading::AnimatedImageLoading( Internal::Adaptor::AnimatedImageLoading* internal )
: BaseHandle( internal )
{
}

} // namespace Dali
