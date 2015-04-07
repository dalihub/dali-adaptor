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
 *
 */

#include "resource-thread-image.h"
#include <dali/public-api/common/ref-counted-dali-vector.h>
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/resource-cache.h>
#include <dali/integration-api/resource-types.h>
#include <curl/curl.h>
#include "portable/file-closer.h"
#include "image-loaders/image-loader.h"

using namespace Dali::Integration;

namespace
{
const int CONNECTION_TIMEOUT( 30 );
}

namespace Dali
{

namespace TizenPlatform
{

ResourceThreadImage::ResourceThreadImage(ResourceLoader& resourceLoader, bool forRemoteImage)
: ResourceThreadBase(resourceLoader)
{
}

ResourceThreadImage::~ResourceThreadImage()
{
}

void ResourceThreadImage::Load(const ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD( mLogFilter );
  DALI_LOG_INFO( mLogFilter, Debug::Verbose, "%s(%s)\n", __FUNCTION__, request.GetPath().c_str() );

  LoadImageFromLocalFile(request);
}

void ResourceThreadImage::Download(const ResourceRequest& request)
{
  bool succeeded;

  DALI_LOG_TRACE_METHOD( mLogFilter );
  DALI_LOG_INFO( mLogFilter, Debug::Verbose, "%s(%s)\n", __FUNCTION__, request.GetPath().c_str() );

  Dali::Vector<uint8_t> dataBuffer;
  size_t dataSize;
  succeeded = DownloadRemoteImageIntoMemory( request, dataBuffer, dataSize );
  if( succeeded )
  {
    DecodeImageFromMemory(static_cast<void*>(&dataBuffer[0]), dataBuffer.Size(), request);
  }
}

void ResourceThreadImage::Decode(const ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD( mLogFilter );
  DALI_LOG_INFO(mLogFilter, Debug::Verbose, "%s(%s)\n", __FUNCTION__, request.GetPath().c_str());

  // Get the blob of binary data that we need to decode:
  DALI_ASSERT_DEBUG( request.GetResource() );

  DALI_ASSERT_DEBUG( 0 != dynamic_cast<Dali::RefCountedVector<uint8_t>*>( request.GetResource().Get() ) && "Only blobs of binary data can be decoded." );
  Dali::RefCountedVector<uint8_t>* const encodedBlob = reinterpret_cast<Dali::RefCountedVector<uint8_t>*>( request.GetResource().Get() );

  if( 0 != encodedBlob )
  {
    const size_t blobSize     = encodedBlob->GetVector().Size();
    uint8_t * const blobBytes = &(encodedBlob->GetVector()[0]);
    DecodeImageFromMemory(blobBytes, blobSize, request);
  }
  else
  {
    FailedResource resource(request.GetId(), FailureUnknown);
    mResourceLoader.AddFailedLoad(resource);
  }
}

void ResourceThreadImage::Save(const Integration::ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD( mLogFilter );
  DALI_ASSERT_DEBUG( request.GetType()->id == ResourceBitmap );
  DALI_LOG_WARNING( "Image saving not supported on background resource threads." );
}

bool ResourceThreadImage::DownloadRemoteImageIntoMemory(const Integration::ResourceRequest& request, Dali::Vector<uint8_t>& dataBuffer, size_t& dataSize)
{
  bool succeeded = true;
  CURLcode cresult;

  CURL* curl_handle = curl_easy_init();
  curl_easy_setopt( curl_handle, CURLOPT_VERBOSE, 0 );
  curl_easy_setopt( curl_handle, CURLOPT_URL, request.GetPath().c_str() );
  curl_easy_setopt( curl_handle, CURLOPT_FAILONERROR, 1 );
  curl_easy_setopt( curl_handle, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT );

  // Download header first to get data size
  char* headerBytes = NULL;
  size_t headerSize = 0;
  FILE* header_fp = open_memstream( &headerBytes, &headerSize );
  double size;

  if( NULL != header_fp)
  {
    curl_easy_setopt( curl_handle, CURLOPT_HEADER, 1 );
    curl_easy_setopt( curl_handle, CURLOPT_NOBODY, 1 );
    curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, header_fp );

    cresult = curl_easy_perform( curl_handle );
    if( cresult == CURLE_OK )
    {
      curl_easy_getinfo( curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size );
    }
    else
    {
      DALI_LOG_WARNING( "Failed to download http header for \"%s\" with error code %d\n", request.GetPath().c_str(), cresult );
      succeeded = false;
    }

    fclose( header_fp );
  }
  else
  {
    succeeded = false;
  }

  if( NULL != headerBytes )
  {
    free( headerBytes );
  }

  if( succeeded )
  {
    // Download file data
    dataSize = static_cast<size_t>( size );
    dataBuffer.Reserve( dataSize );
    dataBuffer.Resize( dataSize );

    Dali::Internal::Platform::FileCloser fileCloser( static_cast<void*>(&dataBuffer[0]), dataSize, "wb" );
    FILE* data_fp = fileCloser.GetFile();
    if( NULL != data_fp )
    {
      curl_easy_setopt( curl_handle, CURLOPT_HEADER, 0 );
      curl_easy_setopt( curl_handle, CURLOPT_NOBODY, 0 );
      curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, data_fp );

      cresult = curl_easy_perform( curl_handle );
      if( CURLE_OK != cresult )
      {
        DALI_LOG_WARNING( "Failed to download image file \"%s\" with error code %d\n", request.GetPath().c_str(), cresult );
        succeeded = false;
      }
    }
    else
    {
      succeeded = false;
    }
  }

  curl_easy_cleanup( curl_handle );

  if( !succeeded )
  {
    FailedResource resource(request.GetId(), FailureUnknown);
    mResourceLoader.AddFailedLoad(resource);
  }

  return succeeded;
}

void ResourceThreadImage::LoadImageFromLocalFile(const Integration::ResourceRequest& request)
{
  bool fileNotFound = false;
  BitmapPtr bitmap = 0;
  bool result = false;

  Dali::Internal::Platform::FileCloser fileCloser( request.GetPath().c_str(), "rb" );
  FILE * const fp = fileCloser.GetFile();

  if( NULL != fp )
  {
    result = ImageLoader::ConvertStreamToBitmap( *request.GetType(), request.GetPath(), fp, *this, bitmap );
    // Last chance to interrupt a cancelled load before it is reported back to clients
    // which have already stopped tracking it:
    InterruptionPoint(); // Note: This can throw an exception.
    if( result && bitmap )
    {
      // Construct LoadedResource and ResourcePointer for image data
      LoadedResource resource( request.GetId(), request.GetType()->id, ResourcePointer( bitmap.Get() ) );
      // Queue the loaded resource
      mResourceLoader.AddLoadedResource( resource );
    }
    else
    {
      DALI_LOG_WARNING( "Unable to decode %s\n", request.GetPath().c_str() );
    }
  }
  else
  {
    DALI_LOG_WARNING( "Failed to open file to load \"%s\"\n", request.GetPath().c_str() );
    fileNotFound = true;
  }

  if ( !bitmap )
  {
    if( fileNotFound )
    {
      FailedResource resource(request.GetId(), FailureFileNotFound  );
      mResourceLoader.AddFailedLoad(resource);
    }
    else
    {
      FailedResource resource(request.GetId(), FailureUnknown);
      mResourceLoader.AddFailedLoad(resource);
    }
  }
}

void ResourceThreadImage::DecodeImageFromMemory(void* blobBytes, size_t blobSize, const Integration::ResourceRequest& request)
{
  BitmapPtr bitmap = 0;

  DALI_ASSERT_DEBUG( blobSize > 0U );
  DALI_ASSERT_DEBUG( blobBytes != 0U );

  if( blobBytes != 0 && blobSize > 0U )
  {
    // Open a file handle on the memory buffer:
    Dali::Internal::Platform::FileCloser fileCloser( blobBytes, blobSize, "rb" );
    FILE * const fp = fileCloser.GetFile();
    if ( NULL != fp )
    {
      bool result = ImageLoader::ConvertStreamToBitmap( *request.GetType(), request.GetPath(), fp, StubbedResourceLoadingClient(), bitmap );
      if ( result && bitmap )
      {
        // Construct LoadedResource and ResourcePointer for image data
        LoadedResource resource( request.GetId(), request.GetType()->id, ResourcePointer( bitmap.Get() ) );
        // Queue the loaded resource
        mResourceLoader.AddLoadedResource( resource );
      }
      else
      {
        DALI_LOG_WARNING( "Unable to decode bitmap supplied as in-memory blob.\n" );
      }
    }
  }

  if (!bitmap)
  {
    FailedResource resource(request.GetId(), FailureUnknown);
    mResourceLoader.AddFailedLoad(resource);
  }
}

} // namespace TizenPlatform

} // namespace Dali
