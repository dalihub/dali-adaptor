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
#include "portable/file-closer.h"
#include "image-loaders/image-loader.h"

using namespace std;
using namespace Dali::Integration;

namespace Dali
{

namespace SlpPlatform
{

ResourceThreadImage::ResourceThreadImage(ResourceLoader& resourceLoader)
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

  bool fileNotFound = false;
  BitmapPtr bitmap = 0;
  bool result = false;

  Dali::Internal::Platform::FileCloser fileCloser( request.GetPath().c_str(), "rb" );
  FILE * const fp = fileCloser.GetFile();

  if( fp != NULL )
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

void ResourceThreadImage::Decode(const ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD( mLogFilter );
  DALI_LOG_INFO(mLogFilter, Debug::Verbose, "%s(%s)\n", __FUNCTION__, request.GetPath().c_str());

  BitmapPtr bitmap = 0;

  // Get the blob of binary data that we need to decode:
  DALI_ASSERT_DEBUG( request.GetResource() );

  DALI_ASSERT_DEBUG( 0 != dynamic_cast<Dali::RefCountedVector<uint8_t>*>( request.GetResource().Get() ) && "Only blobs of binary data can be decoded." );
  Dali::RefCountedVector<uint8_t>* const encodedBlob = reinterpret_cast<Dali::RefCountedVector<uint8_t>*>( request.GetResource().Get() );

  if( encodedBlob != 0 )
  {
    const size_t blobSize     = encodedBlob->GetVector().Size();
    uint8_t * const blobBytes = &(encodedBlob->GetVector()[0]);
    DALI_ASSERT_DEBUG( blobSize > 0U );
    DALI_ASSERT_DEBUG( blobBytes != 0U );

    if( blobBytes != 0 && blobSize > 0U )
    {
      // Open a file handle on the memory buffer:
      Dali::Internal::Platform::FileCloser fileCloser( blobBytes, blobSize, "rb" );
      FILE * const fp = fileCloser.GetFile();
      if ( fp != NULL )
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
  }

  if (!bitmap)
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


} // namespace SlpPlatform

} // namespace Dali
