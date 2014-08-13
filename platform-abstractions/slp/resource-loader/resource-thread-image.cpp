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

#include "loader-bmp.h"
#include "loader-gif.h"
#include "loader-jpeg.h"
#include "loader-png.h"
#include "loader-ico.h"
#include "loader-ktx.h"
#include "loader-wbmp.h"

#include "file-closer.h"

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

typedef bool (*LoadBitmapFunction)( FILE*, Bitmap&, ImageAttributes&, const ResourceLoadingClient& ); ///@ToDo: Make attributes a const reference?
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
    profile = lookupPtr->profile;
  }

  // Reset to the start of the file.
  if( fseek(fp, 0, SEEK_SET) )
  {
    DALI_LOG_ERROR("Error seeking to start of file\n");
  }

  return loaderFound;
}

} // namespace - empty

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
  BitmapPtr bitmap = 0;

  FILE * const fp = fopen( resourcePath.c_str(), "rb" );
  if( fp != NULL )
  {
    bool result = ConvertStreamToBitmap( resourceType, resourcePath, fp, StubbedResourceLoadingClient(), bitmap );
    if( result && bitmap )
    {
      resource.Reset(bitmap.Get());
    }
    fclose(fp);
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
    Bitmap::Profile profile;

    if ( GetBitmapLoaderFunctions( fp,
                                   GetFormatHint(filename),
                                   loaderFunction,
                                   headerFunction,
                                   profile ) )
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
  BitmapPtr bitmap = 0;

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
        Bitmap::Profile profile;

        if ( GetBitmapLoaderFunctions( fp,
                                       FORMAT_UNKNOWN,
                                       loaderFunction,
                                       headerFunction,
                                       profile ) )
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

  bool fileNotFound = false;
  BitmapPtr bitmap = 0;
  bool result = false;

  Dali::Internal::Platform::FileCloser fileCloser( request.GetPath().c_str(), "rb" );
  FILE * const fp = fileCloser.GetFile();

  if( fp != NULL )
  {
    result = ConvertStreamToBitmap( *request.GetType(), request.GetPath(), fp, *this, bitmap );
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
        bool result = ConvertStreamToBitmap( *request.GetType(), request.GetPath(), fp, StubbedResourceLoadingClient(), bitmap );

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

bool ResourceThreadImage::ConvertStreamToBitmap(const ResourceType& resourceType, std::string path, FILE * const fp, const ResourceLoadingClient& client, BitmapPtr& ptr)
{
  DALI_LOG_TRACE_METHOD(mLogFilter);
  DALI_ASSERT_DEBUG( ResourceBitmap == resourceType.id );

  bool result = false;
  BitmapPtr bitmap = 0;

  if (fp != NULL)
  {
    LoadBitmapFunction function;
    LoadBitmapHeaderFunction header;
    Bitmap::Profile profile;

    if ( GetBitmapLoaderFunctions( fp,
                                   GetFormatHint( path ),
                                   function,
                                   header,
                                   profile ) )
    {
      bitmap = Bitmap::New(profile, true);

      DALI_LOG_SET_OBJECT_STRING(bitmap, path);
      const BitmapResourceType& resType = static_cast<const BitmapResourceType&>(resourceType);
      ImageAttributes attributes  = resType.imageAttributes;

      // Check for cancellation now we have hit the filesystem, done some allocation, and burned some cycles:
      client.InterruptionPoint(); // Note: This can throw an exception.

      result = function( fp, *bitmap, attributes, client );

      if (!result)
      {
        DALI_LOG_WARNING("Unable to convert %s\n", path.c_str());
        bitmap = 0;
      }

      // Apply the requested image attributes in best-effort fashion:
      const ImageAttributes& requestedAttributes = resType.imageAttributes;
      // Cut the bitmap according to the desired width and height so that the
      // resulting bitmap has the same aspect ratio as the desired dimensions:
      if( bitmap && bitmap->GetPackedPixelsProfile() && requestedAttributes.GetScalingMode() == ImageAttributes::ScaleToFill )
      {
        const unsigned loadedWidth = bitmap->GetImageWidth();
        const unsigned loadedHeight = bitmap->GetImageHeight();
        const unsigned desiredWidth = requestedAttributes.GetWidth();
        const unsigned desiredHeight = requestedAttributes.GetHeight();

        if( desiredWidth < 1U || desiredHeight < 1U )
        {
          DALI_LOG_WARNING( "Image scaling aborted for image %s as desired dimensions too small (%u, %u)\n.", path.c_str(), desiredWidth, desiredHeight );
        }
        else if( loadedWidth != desiredWidth || loadedHeight != desiredHeight )
        {
          const Vector2 desiredDims( desiredWidth, desiredHeight );

          // Scale the desired rectangle back to fit inside the rectangle of the loaded bitmap:
          // There are two candidates (scaled by x, and scaled by y) and we choose the smallest area one.
          const float widthsRatio = loadedWidth / float(desiredWidth);
          const Vector2 scaledByWidth = desiredDims * widthsRatio;
          const float heightsRatio = loadedHeight / float(desiredHeight);
          const Vector2 scaledByHeight = desiredDims * heightsRatio;
          // Trim top and bottom if the area of the horizontally-fitted candidate is less, else trim the sides:
          const bool trimTopAndBottom = scaledByWidth.width * scaledByWidth.height < scaledByHeight.width * scaledByHeight.height;
          const Vector2 scaledDims = trimTopAndBottom ? scaledByWidth : scaledByHeight;

          // Work out how many pixels to trim from top and bottom, and left and right:
          // (We only ever do one dimension)
          const unsigned scanlinesToTrim = trimTopAndBottom ? fabsf( (scaledDims.y - loadedHeight) * 0.5f ) : 0;
          const unsigned columnsToTrim = trimTopAndBottom ? 0 : fabsf( (scaledDims.x - loadedWidth) * 0.5f );

          DALI_LOG_INFO( mLogFilter, Debug::Concise, "ImageAttributes::ScaleToFill - Bitmap, desired(%f, %f), loaded(%u,%u), cut_target(%f, %f), trimmed(%u, %u), vertical = %s.\n", desiredDims.x, desiredDims.y, loadedWidth, loadedHeight, scaledDims.x, scaledDims.y, columnsToTrim, scanlinesToTrim, trimTopAndBottom ? "true" : "false" );

          // Make a new bitmap with the central part of the loaded one if required:
          if( scanlinesToTrim > 0 || columnsToTrim > 0 ) ///@ToDo: Make this test a bit fuzzy (allow say a 5% difference).
          {
            client.InterruptionPoint(); // Note: This can throw an exception.

            const unsigned newWidth = loadedWidth - 2 * columnsToTrim;
            const unsigned newHeight = loadedHeight - 2 * scanlinesToTrim;
            BitmapPtr croppedBitmap = Bitmap::New( Bitmap::BITMAP_2D_PACKED_PIXELS, true );
            Bitmap::PackedPixelsProfile * packedView = croppedBitmap->GetPackedPixelsProfile();
            DALI_ASSERT_DEBUG( packedView );
            const Pixel::Format pixelFormat = bitmap->GetPixelFormat();
            packedView->ReserveBuffer( pixelFormat, newWidth, newHeight, newWidth, newHeight );

            const unsigned bytesPerPixel = Pixel::GetBytesPerPixel( pixelFormat );

            const PixelBuffer * const srcPixels = bitmap->GetBuffer() + scanlinesToTrim * loadedWidth * bytesPerPixel;
            PixelBuffer * const destPixels = croppedBitmap->GetBuffer();
            DALI_ASSERT_DEBUG( srcPixels && destPixels );

            // Optimize to a single memcpy if the left and right edges don't need a crop, else copy a scanline at a time:
            if( trimTopAndBottom )
            {
              memcpy( destPixels, srcPixels, newHeight * newWidth * bytesPerPixel );
            }
            else
            {
              for( unsigned y = 0; y < newHeight; ++y )
              {
                memcpy( &destPixels[y * newWidth * bytesPerPixel], &srcPixels[y * loadedWidth * bytesPerPixel + columnsToTrim * bytesPerPixel], newWidth * bytesPerPixel );
              }
            }

            // Overwrite the loaded bitmap with the cropped version:
            bitmap = croppedBitmap;
          }
        }
      }
    }
    else
    {
      DALI_LOG_WARNING("Image Decoder for %s unavailable\n", path.c_str());
    }
  }

  ptr.Reset( bitmap.Get() );
  return result;
}

} // namespace SlpPlatform

} // namespace Dali
