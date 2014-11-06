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

#include "resource-thread-distance-field.h"
#include "dali/public-api/images/distance-field.h"

#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>
#include <stdint.h>

#include "image-loaders/loader-bmp.h"
#include "image-loaders/loader-gif.h"
#include "image-loaders/loader-jpeg.h"
#include "image-loaders/loader-png.h"

using namespace std;
using namespace Dali::Integration;

namespace Dali
{

namespace SlpPlatform
{

namespace
{

typedef bool (*LoadBitmapFunction)(FILE*, Bitmap&, ImageAttributes&, const ResourceLoadingClient& );
typedef bool (*LoadBitmapHeaderFunction)(FILE*, const ImageAttributes&, unsigned int&, unsigned int&);

/*
 * Extract the luminance channel L from a RGBF image.
   Luminance is calculated from the sRGB model using a D65 white point, using the Rec.709 formula :
   L = ( 0.2126 * r ) + ( 0.7152 * g ) + ( 0.0722 * b )

   Reference :
   A Standard Default Color Space for the Internet - sRGB.
   [online] http://www.w3.org/Graphics/Color/sRGB
*/

#define LUMA_REC709(r, g, b) (0.2126f * r + 0.7152f * g + 0.0722f * b)
#define GREY8(r, g, b) (uint8_t)LUMA_REC709(r, g, b)

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
 * A lookup table containing all the bitmap loaders with the appropriate information.
 * The grey-scale converter only supports 8-bit channels as input, the image needs to be rgb8 or rgba8 format.
 */
const BitmapLoader BITMAP_LOADER_LOOKUP_TABLE[] =
{
  { Png::MAGIC_BYTE_1,  Png::MAGIC_BYTE_2,  LoadBitmapFromPng,  LoadPngHeader  },
  { Jpeg::MAGIC_BYTE_1, Jpeg::MAGIC_BYTE_2, LoadBitmapFromJpeg, LoadJpegHeader },
  { Bmp::MAGIC_BYTE_1,  Bmp::MAGIC_BYTE_2,  LoadBitmapFromBmp,  LoadBmpHeader  },
  { Gif::MAGIC_BYTE_1,  Gif::MAGIC_BYTE_2,  LoadBitmapFromGif,  LoadGifHeader  },
};

const unsigned int BITMAP_LOADER_COUNT = sizeof(BITMAP_LOADER_LOOKUP_TABLE) / sizeof(BitmapLoader);
const unsigned int MAGIC_LENGTH = 2;

/**
 * Checks the magic bytes of the file first to determine which Image decoder to use to decode the
 * bitmap.
 * @param[in]   fp      The file to decode
 * @param[out]  loader  Set with the function to use to decode the image
 * @param[out]  header  Set with the function to use to decode the header
 * @return true, if we can decode the image, false otherwise
 */
bool GetBitmapLoaderFunctions(FILE *fp, LoadBitmapFunction& loader, LoadBitmapHeaderFunction& header)
{
  unsigned char magic[MAGIC_LENGTH];
  size_t read = fread(magic, sizeof(unsigned char), MAGIC_LENGTH, fp);

  if (read != MAGIC_LENGTH)
  {
    return false;
  }

  bool loaderFound = false;

  const BitmapLoader *lookupPtr = BITMAP_LOADER_LOOKUP_TABLE;
  for (unsigned int i = 0; i < BITMAP_LOADER_COUNT; ++i, ++lookupPtr)
  {
    if (lookupPtr->magicByte1 == magic[0] && lookupPtr->magicByte2 == magic[1])
    {
      loader = lookupPtr->loader;
      header = lookupPtr->header;
      loaderFound = true;
      break;
    }
  }

  // Reset to the start of the file.
  if( fseek(fp, 0, SEEK_SET) )
  {
    DALI_LOG_ERROR("Error seeking to start of file\n");
  }

  return loaderFound;
}

}

ResourceThreadDistanceField::ResourceThreadDistanceField(ResourceLoader& resourceLoader)
: ResourceThreadBase(resourceLoader)
{
}

ResourceThreadDistanceField::~ResourceThreadDistanceField()
{
}

//----------------- Called from separate thread (mThread) -----------------

void ResourceThreadDistanceField::Load(const ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD(mLogFilter);
  DALI_ASSERT_DEBUG(request.GetType()->id == ResourceBitmap);

  // TODO - down-scaling to requested size

  DALI_LOG_INFO(mLogFilter, Debug::Verbose, "%s(%s)\n", __FUNCTION__, request.GetPath().c_str());

  bool result = false;
  bool file_not_found = false;

  BitmapPtr bitmap = Bitmap::New( Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD );

  DALI_LOG_SET_OBJECT_STRING(bitmap, request.GetPath());
  BitmapResourceType& resType = static_cast<BitmapResourceType&>(*(request.GetType()));
  ImageAttributes& attributes  = resType.imageAttributes;

  FILE *fp = fopen(request.GetPath().c_str(), "rb");
  if (fp != NULL)
  {
    // Only png, jpg, bmp and gif files are supported
    LoadBitmapFunction function = NULL;
    LoadBitmapHeaderFunction header = NULL;

    if (GetBitmapLoaderFunctions(fp, function, header))
    {
      result = function(fp, *bitmap, attributes, *this);

      if (result)
      {
        if ((bitmap->GetPixelFormat() == Pixel::RGBA8888) || (bitmap->GetPixelFormat() == Pixel::RGB888))
        {
          // create a bitmap, so we can get its buffersize - to avoid a slow copy.
          BitmapPtr destBitmap = Bitmap::New( Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD );
          destBitmap->GetPackedPixelsProfile()->ReserveBuffer(Pixel::A8, attributes.GetWidth(), attributes.GetHeight());

          uint8_t* srcPixels = bitmap->GetBuffer();
          uint32_t dest = 0;
          uint8_t destPixels[destBitmap->GetBufferSize()];
          memset(destPixels, 0, destBitmap->GetBufferSize());

          switch(bitmap->GetPixelFormat())
          {
            case Pixel::RGB888:
            {
              for(std::size_t i = 0; i < bitmap->GetBufferSize(); i+=3)
              {
                destPixels[dest++] = GREY8( srcPixels[i], srcPixels[i+1], srcPixels[i+2]);
              }

              break;
            }

            case Pixel::RGBA8888:
            {
              for(std::size_t i = 0; i < bitmap->GetBufferSize(); i+=4)
              {
                uint8_t a = srcPixels[i+3];
                // transparent pixels must have an alpha value of 0
                if (a > 0x0)
                {
                  destPixels[dest]= GREY8( srcPixels[i], srcPixels[i+1], srcPixels[i+2]);
                }

                ++dest;
              }

              break;
            }

            default:
              break;
          }

          // now we have an 8 bit luminance map in workbuffer, time to convert it to distance map.
          Size imageSize(destBitmap->GetPackedPixelsProfile()->GetBufferWidth(), destBitmap->GetPackedPixelsProfile()->GetBufferHeight());
          GenerateDistanceFieldMap( destPixels,
                                    imageSize,
                                    destBitmap->GetBuffer(),
                                    imageSize,
                                    attributes.GetFieldBorder(),
                                    imageSize,
                                    true );
          bitmap = destBitmap;
        }
      }
      else
      {
        DALI_LOG_WARNING("Unable to decode %s\n", request.GetPath().c_str());
      }
    }
    else
    {
      DALI_LOG_WARNING("Image Decoder for %s unavailable\n", request.GetPath().c_str());
    }

    fclose(fp);
  }
  else
  {
    DALI_LOG_WARNING("Failed to load \"%s\"\n", request.GetPath().c_str());
    file_not_found = true;
  }

  if (result)
  {
    // Construct LoadedResource and ResourcePointer for image data
    LoadedResource resource( request.GetId(), request.GetType()->id, ResourcePointer(bitmap.Get()) );

    // Queue the loaded resource
    mResourceLoader.AddLoadedResource(resource);
  }
  else
  {
    // add to the failed queue
    FailedResource resource(request.GetId(), file_not_found ? FailureFileNotFound : FailureUnknown);
    mResourceLoader.AddFailedLoad(resource);
  }
}

void ResourceThreadDistanceField::Save(const Integration::ResourceRequest& request)
{
  DALI_LOG_TRACE_METHOD(mLogFilter);
  DALI_ASSERT_DEBUG(request.GetType()->id == ResourceBitmap);
}

} // namespace SlpPlatform

} // namespace Dali
