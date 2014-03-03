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
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>

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

typedef bool (*LoadBitmapFunction)(FILE*, Bitmap&, ImageAttributes&);
typedef bool (*LoadBitmapHeaderFunction)(FILE*, unsigned int&, unsigned int&);

/**
 * Stores the magic bytes, and the loader and header functions used for each image loader.
 */
struct BitmapLoader
{
  unsigned char magicByte1;        ///< The first byte in the file should be this
  unsigned char magicByte2;        ///< The second byte in the file should be this
  LoadBitmapFunction loader;       ///< The function which decodes the file
  LoadBitmapHeaderFunction header; ///< The function which decodes the header of the file
  Bitmap::Profile profile;         ///< The kind of bitmap to be created
                                   ///  (addressable packed pixels or an opaque compressed blob).
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
  { Png::MAGIC_BYTE_1,  Png::MAGIC_BYTE_2,  LoadBitmapFromPng,  LoadPngHeader,  Bitmap::BITMAP_2D_PACKED_PIXELS },
  { Jpeg::MAGIC_BYTE_1, Jpeg::MAGIC_BYTE_2, LoadBitmapFromJpeg, LoadJpegHeader, Bitmap::BITMAP_2D_PACKED_PIXELS },
  { Bmp::MAGIC_BYTE_1,  Bmp::MAGIC_BYTE_2,  LoadBitmapFromBmp,  LoadBmpHeader,  Bitmap::BITMAP_2D_PACKED_PIXELS },
  { Gif::MAGIC_BYTE_1,  Gif::MAGIC_BYTE_2,  LoadBitmapFromGif,  LoadGifHeader,  Bitmap::BITMAP_2D_PACKED_PIXELS },
  { Ktx::MAGIC_BYTE_1,  Ktx::MAGIC_BYTE_2,  LoadBitmapFromKtx,  LoadKtxHeader,  Bitmap::BITMAP_COMPRESSED       },
  { Ico::MAGIC_BYTE_1,  Ico::MAGIC_BYTE_2,  LoadBitmapFromIco,  LoadIcoHeader,  Bitmap::BITMAP_2D_PACKED_PIXELS },
  { 0x0,                0x0,                LoadBitmapFromWbmp, LoadWbmpHeader, Bitmap::BITMAP_2D_PACKED_PIXELS },
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
 * @param[out]  profile The kind of bitmap to hold the bits loaded for the bitmap.
 * @return true, if we can decode the image, false otherwise
 */
bool GetBitmapLoaderFunctions( FILE *fp,
                               FileFormats format,
                               LoadBitmapFunction& loader,
                               LoadBitmapHeaderFunction& header,
                               Bitmap::Profile& profile )
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

  // try hinted format first
  if ( format != FORMAT_UNKNOWN )
  {
    lookupPtr = BITMAP_LOADER_LOOKUP_TABLE + format;
    if ( format >= FORMAT_MAGIC_BYTE_COUNT ||
         ( lookupPtr->magicByte1 == magic[0] && lookupPtr->magicByte2 == magic[1] ) )
    {
      unsigned int width = 0;
      unsigned int height = 0;
      loaderFound = lookupPtr->header(fp, width, height);
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
        loaderFound = lookupPtr->header(fp, width, height);
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
      loaderFound = lookupPtr->header(fp, width, height);
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
    profile = lookupPtr->profile;
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

void ResourceThreadImage::LoadImageMetadata(const std::string fileName, Vector2 &size)
{
  FILE *fp = fopen(fileName.c_str(), "rb");
  if (fp != NULL)
  {
    unsigned int width, height;
    width = height = 0;

    LoadBitmapFunction loaderFunction;
    LoadBitmapHeaderFunction headerFunction;
    Bitmap::Profile profile;

    if ( GetBitmapLoaderFunctions( fp,
                                   GetFormatHint(fileName),
                                   loaderFunction,
                                   headerFunction,
                                   profile ) )
    {
      const bool read_res = headerFunction(fp, width, height);
      if(!read_res){
        DALI_LOG_WARNING("Image Decoder failed to read header for %s\n", fileName.c_str());
      }

      size.x = (float)width;
      size.y = (float)height;
    }
    else
    {
      DALI_LOG_WARNING("Image Decoder for %s unavailable\n", fileName.c_str());
    }
    fclose(fp);
  }
}

//----------------- Called from separate thread (mThread) -----------------

void ResourceThreadImage::FailedLoadOrDecode( const bool foundFile, const ResourceRequest& request )
{
    FailedResource resource(request.GetId(), !foundFile ? FailureFileNotFound : FailureUnknown);
    mResourceLoader.AddFailedLoad(resource);
}

void ResourceThreadImage::Load(const ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD( mLogFilter );
  DALI_ASSERT_DEBUG( request.GetType() && ResourceBitmap == request.GetType()->id );
  DALI_LOG_INFO( mLogFilter, Debug::Verbose, "%s(%s)\n", __FUNCTION__, request.GetPath().c_str() );

  bool file_not_found = false;
  BitmapPtr bitmap = 0;

  FILE * const fp = fopen( request.GetPath().c_str(), "rb" );
  if( fp != NULL )
  {
    bitmap = LoadAndDecodeCommon( request, fp );
    if( !bitmap )
    {
      DALI_LOG_WARNING( "Unable to decode %s\n", request.GetPath().c_str() );
    }
  }
  else
  {
    DALI_LOG_WARNING( "Failed to open file to load \"%s\"\n", request.GetPath().c_str() );
    file_not_found = true;
  }

  // Log the load if it worked:
  if ( bitmap )
  {
    const unsigned int width  = bitmap->GetImageWidth();
    const unsigned int height = bitmap->GetImageHeight();
    const unsigned int bufSize   = bitmap->GetBufferSize();
    Pixel::Format pixelFormat = bitmap->GetPixelFormat();
    const int formatVal = pixelFormat;

    DALI_LOG_RESOURCE("[LOAD] Resource id: %d - image loaded from file %s to Bitmap %p - size %u bytes (logical width x logical height = %ux%u, and pixel format = %d)\n", request.GetId(), request.GetPath().c_str(), bitmap.Get(), bufSize, width, height, formatVal);
  }
  else
  {
    FailedLoadOrDecode( !file_not_found, request );
  }
}

void ResourceThreadImage::Decode(const Integration::ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD( mLogFilter );
  DALI_ASSERT_DEBUG( request.GetType() && ResourceBitmap == request.GetType()->id );
  DALI_LOG_INFO(mLogFilter, Debug::Verbose, "%s(%s)\n", __FUNCTION__, request.GetPath().c_str());

  BitmapPtr bitmap = 0;

  // Get the blob of binary data that we need to decode:
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
        bitmap = LoadAndDecodeCommon(request, fp);
      }
      if ( !bitmap )
      {
        DALI_LOG_WARNING( "Unable to decode bitmap supplied as in-memory blob.\n" );
      }
    }
  }

  // Log the decode if it worked:
  if (bitmap)
  {
    const unsigned int width  = bitmap->GetImageWidth();
    const unsigned int height = bitmap->GetImageHeight();
    const unsigned int bufSize   = bitmap->GetBufferSize();
    Pixel::Format pixelFormat = bitmap->GetPixelFormat();
    const int formatVal = pixelFormat;

    DALI_LOG_RESOURCE("[DECODE] Resource id: %d - image loadedto Bitmap %p - size %u bytes (logical width x logical height = %ux%u, and pixel format = %d)\n", request.GetId(), bitmap.Get(), bufSize, width, height, formatVal);
  }
  else
  {
    FailedLoadOrDecode( true, request );
  }
}

BitmapPtr ResourceThreadImage::LoadAndDecodeCommon(const ResourceRequest& request,
    /** File to read resource from. It will be closed before the function returns. */
    FILE * const fp)
{
  DALI_LOG_TRACE_METHOD(mLogFilter);
  DALI_ASSERT_DEBUG( request.GetType() && ResourceBitmap == request.GetType()->id );

  bool result = false;
  BitmapPtr bitmap = 0;

  if (fp != NULL)
  {
    // Only png, jpg, bmp, gif, and compressed-data-containing ktx files are supported.
    LoadBitmapFunction function;
    LoadBitmapHeaderFunction header;
    Bitmap::Profile profile;

    if ( GetBitmapLoaderFunctions( fp,
                                   GetFormatHint( request.GetPath() ),
                                   function,
                                   header,
                                   profile ) )
    {
      bitmap = Bitmap::New(profile, true);

      DALI_LOG_SET_OBJECT_STRING(bitmap, request.GetPath());
      BitmapResourceType& resType = static_cast<BitmapResourceType&>(*(request.GetType()));
      ImageAttributes& attributes  = resType.imageAttributes;

      result = function(fp, *bitmap, attributes);

      if (!result)
      {
        DALI_LOG_WARNING("Unable to decode %s\n", request.GetPath().c_str());
        bitmap = 0;
      }
    }
    else
    {
      DALI_LOG_WARNING("Image Decoder for %s unavailable\n", request.GetPath().c_str());
    }
    fclose(fp); ///! Not exception safe, but an exception on a resource thread will bring the process down anyway.
  }

  if ( result )
  {
    // Construct LoadedResource and ResourcePointer for image data
    LoadedResource resource( request.GetId(), request.GetType()->id, ResourcePointer( bitmap.Get() ) );
    // Queue the loaded resource
    mResourceLoader.AddLoadedResource( resource );
  }
  return bitmap;
}

void ResourceThreadImage::Save(const Integration::ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD( mLogFilter );
  DALI_ASSERT_DEBUG( request.GetType()->id == ResourceBitmap );
  DALI_LOG_WARNING( "Image saving not supported on background resource threads." );
}


} // namespace SlpPlatform

} // namespace Dali
