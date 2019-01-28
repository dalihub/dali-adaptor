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
 */

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/image-loading.h>

// INTERNAL INCLUDES
#include <dali/public-api/object/property-map.h>
#include <dali/internal/imaging/common/image-loader.h>
#include <dali/internal/imaging/common/file-download.h>
#include <dali/internal/system/common/file-reader.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

namespace Dali
{

namespace
{

// limit maximum image down load size to 50 MB
const size_t MAXIMUM_DOWNLOAD_IMAGE_SIZE  = 50 * 1024 * 1024 ;

}

Devel::PixelBuffer LoadImageFromFile( const std::string& url, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection )
{
  Integration::BitmapResourceType resourceType( size, fittingMode, samplingMode, orientationCorrection );

  Internal::Platform::FileReader fileReader( url );
  FILE * const fp = fileReader.GetFile();
  if( fp != NULL )
  {
    Dali::Devel::PixelBuffer bitmap;
    bool success = TizenPlatform::ImageLoader::ConvertStreamToBitmap( resourceType, url, fp, bitmap );
    if( success && bitmap )
    {
      return bitmap;
    }
  }
  return Dali::Devel::PixelBuffer();
}

ImageDimensions GetClosestImageSize( const std::string& filename,
                                     ImageDimensions size,
                                     FittingMode::Type fittingMode,
                                     SamplingMode::Type samplingMode,
                                     bool orientationCorrection )
{
  ImageDimensions dimension = TizenPlatform::ImageLoader::GetClosestImageSize( filename, size, fittingMode, samplingMode, orientationCorrection );

  dimension.SetWidth( std::min( dimension.GetWidth(), static_cast< uint16_t >( GetMaxTextureSize() ) ) );
  dimension.SetHeight( std::min( dimension.GetHeight(), static_cast< uint16_t >( GetMaxTextureSize() ) ) );

  return dimension;
}

ImageDimensions GetOriginalImageSize( const std::string& filename )
{
   return TizenPlatform::ImageLoader::GetClosestImageSize( filename, ImageDimensions(0, 0), FittingMode::DEFAULT, SamplingMode::BOX_THEN_LINEAR, true );
}

Devel::PixelBuffer DownloadImageSynchronously( const std::string& url, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection )
{
  Integration::BitmapResourceType resourceType( size, fittingMode, samplingMode, orientationCorrection );

  bool succeeded;
  Dali::Vector<uint8_t> dataBuffer;
  size_t dataSize;

  succeeded = TizenPlatform::Network::DownloadRemoteFileIntoMemory( url, dataBuffer, dataSize,
                                                                    MAXIMUM_DOWNLOAD_IMAGE_SIZE );
  if( succeeded )
  {
    size_t blobSize = dataBuffer.Size();

    DALI_ASSERT_DEBUG( blobSize > 0U );

    if( blobSize > 0U )
    {
      // Open a file handle on the memory buffer:
      Dali::Internal::Platform::FileReader fileReader( dataBuffer, blobSize );
      FILE * const fp = fileReader.GetFile();
      if ( NULL != fp )
      {
        Dali::Devel::PixelBuffer bitmap;
        bool result = TizenPlatform::ImageLoader::ConvertStreamToBitmap(
          resourceType,
          url,
          fp,
          bitmap );

        if ( result && bitmap )
        {
          return bitmap;
        }
        else
        {
          DALI_LOG_WARNING( "Unable to decode bitmap supplied as in-memory blob.\n" );
        }
      }
    }
  }
  return Dali::Devel::PixelBuffer();
}

unsigned int GetMaxTextureSize()
{
  return TizenPlatform::ImageLoader::GetMaxTextureSize();
}

} // namespace Dali
