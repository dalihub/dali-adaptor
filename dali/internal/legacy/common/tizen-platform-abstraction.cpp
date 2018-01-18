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

// CLASS HEADER
#include <dali/internal/legacy/common/tizen-platform-abstraction.h>

// EXTERNAL INCLUDES
#include <dirent.h>
#include <fstream>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/resource-types.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/image-loader.h>
#include <dali/internal/system/common/file-reader.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{

namespace TizenPlatform
{

TizenPlatformAbstraction::TizenPlatformAbstraction()
: mDataStoragePath( "" )
{
}

TizenPlatformAbstraction::~TizenPlatformAbstraction()
{
}

ImageDimensions TizenPlatformAbstraction::GetClosestImageSize( const std::string& filename,
                                                               ImageDimensions size,
                                                               FittingMode::Type fittingMode,
                                                               SamplingMode::Type samplingMode,
                                                               bool orientationCorrection )
{
  return ImageLoader::GetClosestImageSize( filename, size, fittingMode, samplingMode, orientationCorrection );
}

ImageDimensions TizenPlatformAbstraction::GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                                                               ImageDimensions size,
                                                               FittingMode::Type fittingMode,
                                                               SamplingMode::Type samplingMode,
                                                               bool orientationCorrection )
{
  return ImageLoader::GetClosestImageSize( resourceBuffer, size, fittingMode, samplingMode, orientationCorrection );
}

Integration::ResourcePointer TizenPlatformAbstraction::LoadImageSynchronously(const Integration::BitmapResourceType& resource, const std::string& resourcePath)
{
  return ImageLoader::LoadImageSynchronously( resource, resourcePath );
}

Integration::BitmapPtr TizenPlatformAbstraction::DecodeBuffer( const Integration::BitmapResourceType& resource, uint8_t * buffer, size_t size )
{
  Integration::BitmapPtr resultBitmap;
  Dali::Devel::PixelBuffer bitmap;

  Dali::Internal::Platform::FileReader fileReader( buffer, size );
  FILE * const fp = fileReader.GetFile();
  if( fp )
  {
    bool result = ImageLoader::ConvertStreamToBitmap( resource, "", fp, bitmap );
    if ( !result || !bitmap )
    {
      bitmap.Reset();
      DALI_LOG_WARNING( "Unable to decode bitmap supplied as in-memory blob.\n" );
    }
    else
    {
      Integration::Bitmap::Profile profile{Integration::Bitmap::Profile::BITMAP_2D_PACKED_PIXELS};

      // For backward compatibility the Bitmap must be created
      auto retval = Integration::Bitmap::New(profile, Dali::ResourcePolicy::OWNED_DISCARD);

      retval->GetPackedPixelsProfile()->ReserveBuffer(
              bitmap.GetPixelFormat(),
              bitmap.GetWidth(),
              bitmap.GetHeight(),
              bitmap.GetWidth(),
              bitmap.GetHeight()
            );

      auto& impl = Dali::GetImplementation(bitmap);

      std::copy( impl.GetBuffer(), impl.GetBuffer()+impl.GetBufferSize(), retval->GetBuffer());
      resultBitmap.Reset(retval);
    }
  }

  return resultBitmap;
}

bool TizenPlatformAbstraction::LoadShaderBinaryFile( const std::string& filename, Dali::Vector< unsigned char >& buffer ) const
{
  bool result = false;

#ifdef SHADERBIN_CACHE_ENABLED
  std::string path;

  // First check the system location where shaders are stored at install time:
  path = DALI_SHADERBIN_DIR;
  path += filename;
  result = LoadFile( path, buffer );

  // Fallback to the cache of shaders stored after previous runtime compilations:
  // On desktop this looks in the current working directory that the app was launched from.
  if( mResourceLoader && result == false )
  {
    path = mDataStoragePath;
    path += filename;
    result = LoadFile( path, buffer );
  }
#endif

  return result;
}

bool TizenPlatformAbstraction::SaveShaderBinaryFile( const std::string& filename, const unsigned char * buffer, unsigned int numBytes ) const
{
  bool result = false;

#ifdef SHADERBIN_CACHE_ENABLED

  // Use the cache of shaders stored after previous runtime compilations:
  // On desktop this looks in the current working directory that the app was launched from.
  std::string path = mDataStoragePath;
  path += filename;
  result = SaveFile( path, buffer, numBytes );

#endif

  return result;
}

void TizenPlatformAbstraction::SetDataStoragePath( const std::string& path )
{
  mDataStoragePath = path;
}

TizenPlatformAbstraction* CreatePlatformAbstraction()
{
  return new TizenPlatformAbstraction();
}

bool SaveFile( const std::string& filename, const unsigned char * buffer, unsigned int numBytes )
{
  DALI_ASSERT_DEBUG( 0 != filename.length());

  bool result = false;

  std::filebuf buf;
  buf.open(filename.c_str(), std::ios::out | std::ios_base::trunc | std::ios::binary);
  if( buf.is_open() )
  {
    std::ostream stream(&buf);

    // determine size of buffer
    int length = static_cast<int>(numBytes);

    // write contents of buffer to the file
    stream.write(reinterpret_cast<const char*>(buffer), length);

    if( !stream.bad() )
    {
      result = true;
    }
  }

  return result;
}

}  // namespace TizenPlatform

}  // namespace Dali
