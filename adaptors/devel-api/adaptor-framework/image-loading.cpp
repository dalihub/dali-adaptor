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
#include <resource-loader/network/file-download.h>
#include <platform-abstractions/portable/file-closer.h>

namespace Dali
{

namespace
{
// limit maximum image down load size to 50 MB
const size_t MAXIMUM_DOWNLOAD_IMAGE_SIZE  = 50 * 1024 * 1024 ;
}

PixelData LoadImageFromFile( const std::string& url, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection )
{
  Integration::BitmapResourceType resourceType( size, fittingMode, samplingMode, orientationCorrection );
  IntrusivePtr<Dali::RefObject> resource = TizenPlatform::ImageLoader::LoadImageSynchronously( resourceType, url );

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

ImageDimensions GetClosestImageSize( const std::string& filename,
                                     ImageDimensions size,
                                     FittingMode::Type fittingMode,
                                     SamplingMode::Type samplingMode,
                                     bool orientationCorrection )
{
  return TizenPlatform::ImageLoader::GetClosestImageSize( filename, size, fittingMode, samplingMode, orientationCorrection );
}


PixelData DownloadImageSynchronously( const std::string& url, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection )
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
          return Dali::PixelData::New( bitmap->GetBufferOwnership(),
                                       bitmap->GetBufferSize(),
                                       bitmap->GetImageWidth(),
                                       bitmap->GetImageHeight(),
                                       bitmap->GetPixelFormat(),
                                       Dali::PixelData::FREE );
        }
        else
        {
          DALI_LOG_WARNING( "Unable to decode bitmap supplied as in-memory blob.\n" );
        }
      }
    }

  }
  return Dali::PixelData();
}


} // namespace Dali
