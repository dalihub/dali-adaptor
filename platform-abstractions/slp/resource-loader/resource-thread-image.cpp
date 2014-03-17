//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "resource-thread-image.h"
#include <dali/public-api/common/ref-counted-dali-vector.h>
#include <dali/integration-api/image-data.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/resource-cache.h>

#include "loader-bmp.h"
#include "loader-gif.h"
#include "loader-jpeg.h"
#include "loader-png.h"
#include "loader-ico.h"
#include "loader-ktx.h"
#include "loader-wbmp.h"

using namespace std;
using namespace Dali::Integration;
using boost::mutex;
using boost::unique_lock;

namespace Dali
{

namespace SlpPlatform
{

namespace
{

typedef bool (*LoadBitmapFunction)(FILE*, ImageAttributes&, ImageDataPtr&);
typedef bool (*LoadBitmapHeaderFunction)(FILE*, const ImageAttributes& attrs, unsigned int& width, unsigned int& height );

/**
 * Stores the magic bytes, and the loader and header functions used for each image loader.
 */
struct BitmapLoader
{
  unsigned char magicByte1;        ///< The first byte in the file should be this
  unsigned char magicByte2;        ///< The second byte in the file should be this
  LoadBitmapFunction loader;       ///< The function which decodes the file
  LoadBitmapHeaderFunction header; ///< The function which decodes the header of the file
};

/**
 * Enum for file formats, has to be in sync with BITMAP_LOADER_LOOKUP_TABLE
 */
enum FileFormats
{
  // Unknown file format
  FORMAT_UNKNOWN = -1,

  // formats that use magic bytes
  FORMAT_PNG = 0,
  FORMAT_JPEG,
  FORMAT_BMP,
  FORMAT_GIF,
  FORMAT_KTX,
  FORMAT_ICO,
  FORMAT_MAGIC_BYTE_COUNT,

  // formats after this one do not use magic bytes
  FORMAT_WBMP = FORMAT_MAGIC_BYTE_COUNT,
  FORMAT_TOTAL_COUNT
};

/**
 * A lookup table containing all the bitmap loaders with the appropriate information.
 * Has to be in sync with enum FileFormats
 */
const BitmapLoader BITMAP_LOADER_LOOKUP_TABLE[FORMAT_TOTAL_COUNT] =
{
  { Png::MAGIC_BYTE_1,  Png::MAGIC_BYTE_2,  LoadBitmapFromPng,  LoadPngHeader  },
  { Jpeg::MAGIC_BYTE_1, Jpeg::MAGIC_BYTE_2, LoadBitmapFromJpeg, LoadJpegHeader },
  { Bmp::MAGIC_BYTE_1,  Bmp::MAGIC_BYTE_2,  LoadBitmapFromBmp,  LoadBmpHeader  },
  { Gif::MAGIC_BYTE_1,  Gif::MAGIC_BYTE_2,  LoadBitmapFromGif,  LoadGifHeader  },
  { Ktx::MAGIC_BYTE_1,  Ktx::MAGIC_BYTE_2,  LoadBitmapFromKtx,  LoadKtxHeader  },
  { Ico::MAGIC_BYTE_1,  Ico::MAGIC_BYTE_2,  LoadBitmapFromIco,  LoadIcoHeader  },
  { 0x0,                0x0,                LoadBitmapFromWbmp, LoadWbmpHeader },
};

const unsigned int MAGIC_LENGTH = 2;

/**
 * This code tries to predict the file format from the filename to help with format picking.
 */
struct FormatExtension
{
  const std::string extension;
  FileFormats format;
};

const FormatExtension FORMAT_EXTENSIONS[] =
{
 { ".png",  FORMAT_PNG  },
 { ".jpg",  FORMAT_JPEG },
 { ".bmp",  FORMAT_BMP  },
 { ".gif",  FORMAT_GIF  },
 { ".ktx",  FORMAT_KTX  },
 { ".ico",  FORMAT_ICO  },
 { ".wbmp", FORMAT_WBMP }
};

const unsigned int FORMAT_EXTENSIONS_COUNT = sizeof(FORMAT_EXTENSIONS) / sizeof(FormatExtension);

FileFormats GetFormatHint( const std::string& filename )
{
  FileFormats format = FORMAT_UNKNOWN;

  for ( unsigned int i = 0; i < FORMAT_EXTENSIONS_COUNT; ++i )
  {
    unsigned int length = FORMAT_EXTENSIONS[i].extension.size();
    if ( ( filename.size() > length ) &&
         ( 0 == filename.compare( filename.size() - length, length, FORMAT_EXTENSIONS[i].extension ) ) )
    {
      format = FORMAT_EXTENSIONS[i].format;
      break;
    }
  }

  return format;
}

/**
 * Checks the magic bytes of the file first to determine which Image decoder to use to decode the
 * bitmap.
 * @param[in]   fp      The file to decode
 * @param[in]   format  Hint about what format to try first
 * @param[out]  loader  Set with the function to use to decode the image
 * @param[out]  header  Set with the function to use to decode the header
 * @return true, if we can decode the image, false otherwise
 */
bool GetBitmapLoaderFunctions( FILE *fp,
                               FileFormats format,
                               LoadBitmapFunction& loader,
                               LoadBitmapHeaderFunction& header )
{
  unsigned char magic[MAGIC_LENGTH];
  size_t read = fread(magic, sizeof(unsigned char), MAGIC_LENGTH, fp);

  // Reset to the start of the file.
  if( fseek(fp, 0, SEEK_SET) )
  {
    DALI_LOG_ERROR("Error seeking to start of file\n");
  }

  if (read != MAGIC_LENGTH)
  {
    return false;
  }

  bool loaderFound = false;
  const BitmapLoader *lookupPtr = BITMAP_LOADER_LOOKUP_TABLE;
  ImageAttributes attrs;

  // try hinted format first
  if ( format != FORMAT_UNKNOWN )
  {
    lookupPtr = BITMAP_LOADER_LOOKUP_TABLE + format;
    if ( format >= FORMAT_MAGIC_BYTE_COUNT ||
       ( lookupPtr->magicByte1 == magic[0] && lookupPtr->magicByte2 == magic[1] ) )
    {
      unsigned int width = 0;
      unsigned int height = 0;
      loaderFound = lookupPtr->header(fp, attrs, width, height );
    }
  }

  // then try to get a match with formats that have magic bytes
  if ( false == loaderFound )
  {
    for ( lookupPtr = BITMAP_LOADER_LOOKUP_TABLE;
          lookupPtr < BITMAP_LOADER_LOOKUP_TABLE + FORMAT_MAGIC_BYTE_COUNT;
          ++lookupPtr )
    {
      if ( lookupPtr->magicByte1 == magic[0] && lookupPtr->magicByte2 == magic[1] )
      {
        // to seperate ico file format and wbmp file format
        unsigned int width = 0;
        unsigned int height = 0;
        loaderFound = lookupPtr->header(fp, attrs, width, height);
      }
      if (loaderFound)
      {
        break;
      }
    }
  }

  // finally try formats that do not use magic bytes
  if ( false == loaderFound )
  {
    for ( lookupPtr = BITMAP_LOADER_LOOKUP_TABLE + FORMAT_MAGIC_BYTE_COUNT;
          lookupPtr < BITMAP_LOADER_LOOKUP_TABLE + FORMAT_TOTAL_COUNT;
          ++lookupPtr )
    {
      // to seperate ico file format and wbmp file format
      unsigned int width = 0;
      unsigned int height = 0;
      loaderFound = lookupPtr->header(fp, attrs, width, height);
      if (loaderFound)
      {
        break;
      }
    }
  }

  // if a loader was found set the outputs
  if ( loaderFound )
  {
    loader  = lookupPtr->loader;
    header  = lookupPtr->header;
  }

  // Reset to the start of the file.
  if( fseek(fp, 0, SEEK_SET) )
  {
    DALI_LOG_ERROR("Error seeking to start of file\n");
  }

  return loaderFound;
}

}

ResourceThreadImage::ResourceThreadImage(ResourceLoader& resourceLoader)
: ResourceThreadBase(resourceLoader)
{
}

ResourceThreadImage::~ResourceThreadImage()
{
}

ResourcePointer ResourceThreadImage::LoadResourceSynchronously( const Integration::ResourceType& resourceType, const std::string& resourcePath )
{
  ResourcePointer resource;
  ImageDataPtr bitmap = 0;

  FILE * const fp = fopen( resourcePath.c_str(), "rb" );
  if( fp != NULL )
  {
    bool result = ConvertStreamToBitmap( resourceType, resourcePath, fp, bitmap );
    if( result && bitmap )
    {
      resource.Reset(bitmap.Get());
    }
  }
  return resource;
}


void ResourceThreadImage::GetClosestImageSize( const std::string& filename,
                                               const ImageAttributes& attributes,
                                               Vector2 &closestSize )
{
  FILE *fp = fopen(filename.c_str(), "rb");
  if (fp != NULL)
  {
    LoadBitmapFunction loaderFunction;
    LoadBitmapHeaderFunction headerFunction;

    if ( GetBitmapLoaderFunctions( fp,
                                   GetFormatHint(filename),
                                   loaderFunction,
                                   headerFunction ) )
    {
      unsigned int width;
      unsigned int height;

      const bool read_res = headerFunction(fp, attributes, width, height);
      if(!read_res)
      {
        DALI_LOG_WARNING("Image Decoder failed to read header for %s\n", filename.c_str());
      }

      closestSize.width = (float)width;
      closestSize.height = (float)height;
    }
    else
    {
      DALI_LOG_WARNING("Image Decoder for %s unavailable\n", filename.c_str());
    }
    fclose(fp);
  }
}


void ResourceThreadImage::GetClosestImageSize( ResourcePointer resourceBuffer,
                                               const ImageAttributes& attributes,
                                               Vector2 &closestSize )
{
  // Get the blob of binary data that we need to decode:
  DALI_ASSERT_DEBUG( resourceBuffer );
  Dali::RefCountedVector<uint8_t>* const encodedBlob = reinterpret_cast<Dali::RefCountedVector<uint8_t>*>( resourceBuffer.Get() );

  if( encodedBlob != 0 )
  {
    const size_t blobSize     = encodedBlob->GetVector().Size();
    uint8_t * const blobBytes = &(encodedBlob->GetVector()[0]);
    DALI_ASSERT_DEBUG( blobSize > 0U );
    DALI_ASSERT_DEBUG( blobBytes != 0U );

    if( blobBytes != 0 && blobSize > 0U )
    {
      // Open a file handle on the memory buffer:
      FILE * const fp = fmemopen(blobBytes, blobSize, "rb");
      if ( fp != NULL )
      {
        LoadBitmapFunction loaderFunction;
        LoadBitmapHeaderFunction headerFunction;

        if ( GetBitmapLoaderFunctions( fp,
                                       FORMAT_UNKNOWN,
                                       loaderFunction,
                                       headerFunction ) )
        {
          unsigned int width;
          unsigned int height;
          const bool read_res = headerFunction(fp, attributes, width, height);
          if(!read_res)
          {
            DALI_LOG_WARNING("Image Decoder failed to read header for resourceBuffer\n");
          }

          closestSize.width = (float) width;
          closestSize.height = (float) height;
        }
        fclose(fp);
      }
    }
  }
}


//----------------- Called from separate thread (mThread) -----------------

void ResourceThreadImage::Load(const ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD( mLogFilter );
  DALI_LOG_INFO( mLogFilter, Debug::Verbose, "%s(%s)\n", __FUNCTION__, request.GetPath().c_str() );

  bool file_not_found = false;
  ImageDataPtr bitmap;
  bool result = false;

  FILE * const fp = fopen( request.GetPath().c_str(), "rb" );
  if( fp != NULL )
  {
    result = ConvertStreamToBitmap( *request.GetType(), request.GetPath(), fp, bitmap );
    HandleConversionResult( request, result, bitmap,  request.GetPath().c_str() );
  }
  else
  {
    DALI_LOG_WARNING( "Failed to open file to load \"%s\"\n", request.GetPath().c_str() );
    file_not_found = true;
  }

  if ( !bitmap )
  {
    if( file_not_found )
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

  ImageDataPtr bitmap = 0;

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
      FILE * const fp = fmemopen(blobBytes, blobSize, "rb");
      if ( fp != NULL )
      {
        bool result = ConvertStreamToBitmap( *request.GetType(), request.GetPath(), fp, bitmap );
        HandleConversionResult( request, result, bitmap, "(image data supplied as in-memory encoded buffer)" );
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
  DALI_ASSERT_DEBUG( request.GetType()->id == ResourceImageData );
  DALI_LOG_WARNING( "Image saving not supported on background resource threads." );
}


bool ResourceThreadImage::ConvertStreamToBitmap(const ResourceType& resourceType, std::string path, FILE * const fp, ImageDataPtr& ptr)
{
  DALI_LOG_TRACE_METHOD(mLogFilter);
  DALI_ASSERT_DEBUG( ResourceImageData == resourceType.id );

  bool result = false;
  ImageDataPtr bitmap = 0;

  if (fp != NULL)
  {
    LoadBitmapFunction function;
    LoadBitmapHeaderFunction header;

    if ( GetBitmapLoaderFunctions( fp,
                                   GetFormatHint( path ),
                                   function,
                                   header ) )
    {
      const ImageResourceType& resType = static_cast<const ImageResourceType&>(resourceType);
      ImageAttributes attributes  = resType.imageAttributes;

      result = function(fp, attributes, bitmap);
      DALI_LOG_SET_OBJECT_STRING(bitmap, path);

      if (!result)
      {
        DALI_LOG_WARNING("Unable to convert %s\n", path.c_str());
        bitmap = 0;
      }
    }
    else
    {
      DALI_LOG_WARNING("Image Decoder for %s unavailable\n", path.c_str());
    }
    fclose(fp); ///! Not exception safe, but an exception on a resource thread will bring the process down anyway.
  }

  ptr.Reset( bitmap.Get() );
  return result;
}

namespace
{
bool IsAlphaChannelUsed(const uint8_t * const pixelBuffer, const unsigned width, const unsigned height, const Pixel::Format pixelFormat)
{
  bool alphaChannelUsed = false;

  if(pixelBuffer != NULL)
  {
    const uint8_t* row = pixelBuffer;

    int byte; int bits;
    Pixel::GetAlphaOffsetAndMask(pixelFormat, byte, bits);
    const unsigned bytesPerPixel = Pixel::GetBytesPerPixel( pixelFormat );
    const unsigned stride       = width * bytesPerPixel;

    for(size_t j=0; j < height; j++)
    {
      const uint8_t* pixels = row;
      for(unsigned i=0; i < width; ++i)
      {
        if((pixels[byte] & bits) != bits)
        {
          alphaChannelUsed = true;
          j = height; // break out of outer loop
          break;
        }
        pixels += bytesPerPixel;
      }
      row += stride;
    }
  }
  return alphaChannelUsed;
}
}

void ResourceThreadImage::HandleConversionResult( const Integration::ResourceRequest& request, bool result, Integration::ImageDataPtr imageData, const char * const msg )
{
  if( result && imageData )
  {
    // Scan the pixels of the ImageData to see if its alpha channel is used:
    if( Pixel::HasAlpha( imageData->pixelFormat ) )
    {
      const uint8_t* const pixelBuffer = imageData->GetBuffer();
      if( pixelBuffer )
      {
        const bool alphaUsed = IsAlphaChannelUsed( pixelBuffer, imageData->imageWidth, imageData->imageHeight, imageData->pixelFormat );
        imageData->SetAlphaUsed( alphaUsed );
      }
    }

    // Construct LoadedResource and ResourcePointer for image data
    LoadedResource resource( request.GetId(), request.GetType()->id, Integration::ResourcePointer( imageData.Get() ) );
    // Queue the loaded resource
    mResourceLoader.AddLoadedResource( resource );
  }
  else
  {
    DALI_LOG_WARNING( "Unable to decode: %s\n",  msg);
  }
}

} // namespace SlpPlatform

} // namespace Dali
