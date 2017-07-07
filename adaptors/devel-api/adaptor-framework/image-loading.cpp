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
#include "image-loading.h"

// INTERNAL INCLUDES
#include "image-loaders/image-loader.h"
#include <resource-loader/network/file-download.h>
#include <platform-abstractions/portable/file-closer.h>
#include "pixel-buffer-impl.h"

namespace Dali
{

namespace
{

// limit maximum image down load size to 50 MB
const size_t MAXIMUM_DOWNLOAD_IMAGE_SIZE  = 50 * 1024 * 1024 ;

static unsigned int gMaxTextureSize = 4096;

}

Devel::PixelBuffer LoadImageFromFile( const std::string& url, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection )
{
  Integration::BitmapResourceType resourceType( size, fittingMode, samplingMode, orientationCorrection );

  Internal::Platform::FileCloser fc( url.c_str(), "rb");
  FILE * const fp = fc.GetFile();
  if( fp != NULL )
  {
    Integration::BitmapPtr bitmap;
    bool success = TizenPlatform::ImageLoader::ConvertStreamToBitmap( resourceType, url, fp, bitmap );
    if( success && bitmap )
    {
      // Use bitmap->GetBufferOwnership() to transfer the buffer ownership
      // to pixelData.  The destroy of bitmap will not release the buffer,
      // instead, the pixelBuffer is responsible for releasing when its
      // reference count falls to zero.
      Internal::Adaptor::PixelBufferPtr pixelBufferImpl =
        Internal::Adaptor::PixelBuffer::New( bitmap->GetBufferOwnership(),
                                             bitmap->GetBufferSize(),
                                             bitmap->GetImageWidth(),
                                             bitmap->GetImageHeight(),
                                             bitmap->GetPixelFormat() );

      Dali::Devel::PixelBuffer pixelBuffer( pixelBufferImpl.Get() );
      return pixelBuffer;
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
    void *blobBytes = static_cast<void*>(&dataBuffer[0]);
    size_t blobSize = dataBuffer.Size();

    DALI_ASSERT_DEBUG( blobSize > 0U );
    DALI_ASSERT_DEBUG( blobBytes != 0U );

    if( blobBytes != 0 && blobSize > 0U )
    {
      // Open a file handle on the memory buffer:
      Dali::Internal::Platform::FileCloser fileCloser( blobBytes, blobSize, "rb" );
      FILE * const fp = fileCloser.GetFile();
      if ( NULL != fp )
      {
        Integration::BitmapPtr bitmap;
        bool result = TizenPlatform::ImageLoader::ConvertStreamToBitmap(
          resourceType,
          url,
          fp,
          bitmap );

        if ( result && bitmap )
        {
          Internal::Adaptor::PixelBufferPtr pixelBufferImpl =
            Internal::Adaptor::PixelBuffer::New( bitmap->GetBufferOwnership(),
                                                 bitmap->GetBufferSize(),
                                                 bitmap->GetImageWidth(),
                                                 bitmap->GetImageHeight(),
                                                 bitmap->GetPixelFormat() );

          Dali::Devel::PixelBuffer pixelBuffer( pixelBufferImpl.Get() );
          return pixelBuffer;
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

void SetMaxTextureSize( unsigned int size )
{
  gMaxTextureSize = size;
}

unsigned int GetMaxTextureSize()
{
  return gMaxTextureSize;
}

} // namespace Dali
