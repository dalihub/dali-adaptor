/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

#include <dali/internal/imaging/common/loader-gif.h>

#include <gif_lib.h>

#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/system/common/system-error-print.h>
#include <memory>

// We need to check if giflib has the new open and close API (including error parameter).
#ifdef GIFLIB_MAJOR
#define LIBGIF_VERSION_5_1_OR_ABOVE
#endif

namespace Dali
{
namespace TizenPlatform
{
namespace
{
// simple class to enforce clean-up of GIF structures
struct AutoCleanupGif
{
  AutoCleanupGif(GifFileType*& _gifInfo)
  : gifInfo(_gifInfo)
  {
  }

  ~AutoCleanupGif()
  {
    if(NULL != gifInfo)
    {
      // clean up GIF resources
#ifdef LIBGIF_VERSION_5_1_OR_ABOVE
      int errorCode = 0; //D_GIF_SUCCEEDED is 0
      DGifCloseFile(gifInfo, &errorCode);

      if(errorCode)
      {
        DALI_LOG_ERROR("GIF Loader: DGifCloseFile Error. Code: %d\n", errorCode);
      }
#else
      DGifCloseFile(gifInfo);
#endif
    }
  }

  GifFileType*& gifInfo;
};

// Used in the GIF interlace algorithm to determine the starting byte and the increment required
// for each pass.
struct InterlacePair
{
  unsigned int startingByte;
  unsigned int incrementalByte;
};

// Used in the GIF interlace algorithm to determine the order and which location to read data from
// the file.
const InterlacePair INTERLACE_PAIR_TABLE[] = {
  {0, 8}, // Starting at 0, read every 8 bytes.
  {4, 8}, // Starting at 4, read every 8 bytes.
  {2, 4}, // Starting at 2, read every 4 bytes.
  {1, 2}, // Starting at 1, read every 2 bytes.
};
const unsigned int INTERLACE_PAIR_TABLE_SIZE(sizeof(INTERLACE_PAIR_TABLE) / sizeof(InterlacePair));

/// Function used by Gif_Lib to read from the image file.
int ReadDataFromGif(GifFileType* gifInfo, GifByteType* data, int length)
{
  FILE*     fp           = reinterpret_cast<FILE*>(gifInfo->UserData);
  const int actualLength = fread(data, sizeof(GifByteType), length, fp);
  if(DALI_UNLIKELY(actualLength != length))
  {
    DALI_LOG_ERROR("Error read bytes (required : %d, actual read : %d)\n", length, actualLength);
    DALI_PRINT_SYSTEM_ERROR_LOG();
  }

  return actualLength;
}

/// Loads the GIF Header.
bool LoadGifHeader(FILE* fp, unsigned int& width, unsigned int& height, GifFileType** gifInfo)
{
  int errorCode = 0; //D_GIF_SUCCEEDED is 0

#ifdef LIBGIF_VERSION_5_1_OR_ABOVE
  *gifInfo = DGifOpen(reinterpret_cast<void*>(fp), ReadDataFromGif, &errorCode);
#else
  *gifInfo = DGifOpen(reinterpret_cast<void*>(fp), ReadDataFromGif);
#endif

  if(DALI_UNLIKELY(!(*gifInfo) || errorCode))
  {
    DALI_LOG_ERROR("GIF Loader: DGifOpen Error. Code: %d\n", errorCode);
    return false;
  }

  width  = (*gifInfo)->SWidth;
  height = (*gifInfo)->SHeight;

  // No proper size in GIF.
  if(DALI_UNLIKELY(width <= 0 || height <= 0))
  {
    return false;
  }

  return true;
}

/// Decode the GIF image.
bool DecodeImage(GifFileType* gifInfo, unsigned char* decodedData, const unsigned int width, const unsigned int height, const unsigned int bytesPerRow)
{
  if(gifInfo->Image.Interlace)
  {
    // If the image is interlaced, then use the GIF interlace algorithm to read the file appropriately.

    const InterlacePair* interlacePairPtr(INTERLACE_PAIR_TABLE);
    for(unsigned int interlacePair = 0; interlacePair < INTERLACE_PAIR_TABLE_SIZE; ++interlacePair, ++interlacePairPtr)
    {
      for(unsigned int currentByte = interlacePairPtr->startingByte; currentByte < height; currentByte += interlacePairPtr->incrementalByte)
      {
        unsigned char* row = decodedData + currentByte * bytesPerRow;
        if(DALI_UNLIKELY(DGifGetLine(gifInfo, row, width) == GIF_ERROR))
        {
          DALI_LOG_ERROR("GIF Loader: Error reading Interlaced GIF\n");
          return false;
        }
      }
    }
  }
  else
  {
    // Non-interlace does not require any erratic reading / jumping.
    unsigned char* decodedDataPtr(decodedData);

    for(unsigned int row = 0; row < height; ++row)
    {
      if(DALI_UNLIKELY(DGifGetLine(gifInfo, decodedDataPtr, width) == GIF_ERROR))
      {
        DALI_LOG_ERROR("GIF Loader: Error reading non-interlaced GIF\n");
        return false;
      }
      decodedDataPtr += bytesPerRow;
    }
  }
  return true;
}

// Retrieves the colors used in the GIF image.
GifColorType* GetImageColors(SavedImage* image, GifFileType* gifInfo)
{
  GifColorType* color(NULL);
  if(image->ImageDesc.ColorMap)
  {
    color = image->ImageDesc.ColorMap->Colors;
  }
  else
  {
    // if there is no color map for this image use the default one
    color = gifInfo->SColorMap->Colors;
  }
  return color;
}

/// Called when we want to handle IMAGE_DESC_RECORD_TYPE
bool HandleImageDescriptionRecordType(Dali::Devel::PixelBuffer& bitmap, GifFileType* gifInfo, unsigned int width, unsigned int height, bool& finished)
{
  if(DALI_UNLIKELY(DGifGetImageDesc(gifInfo) == GIF_ERROR))
  {
    DALI_LOG_ERROR("GIF Loader: Error getting Image Description\n");
    return false;
  }

  // Ensure there is at least 1 image in the GIF.
  if(DALI_UNLIKELY(gifInfo->ImageCount < 1))
  {
    DALI_LOG_ERROR("GIF Loader: No Images\n");
    return false;
  }

  Pixel::Format pixelFormat(Pixel::RGB888);

  SavedImage*         image(&gifInfo->SavedImages[gifInfo->ImageCount - 1]);
  const GifImageDesc& desc(image->ImageDesc);

  auto decodedData = new unsigned char[width * height * sizeof(GifPixelType)];

  std::unique_ptr<unsigned char[]> ptr{decodedData};

  const unsigned int bytesPerRow(width * sizeof(GifPixelType));
  const unsigned int actualWidth(desc.Width);
  const unsigned int actualHeight(desc.Height);

  // Create a buffer to store the decoded data.
  bitmap = Dali::Devel::PixelBuffer::New(actualWidth, actualHeight, pixelFormat);

  // Decode the GIF Image
  if(DALI_UNLIKELY(!DecodeImage(gifInfo, decodedData, actualWidth, actualHeight, bytesPerRow)))
  {
    return false;
  }

  // Get the colormap for the GIF
  GifColorType* color(GetImageColors(image, gifInfo));

  // If it's an animated GIF, we still only read the first image

  // Create and populate pixel buffer.
  auto pixels = bitmap.GetBuffer();
  for(unsigned int row = 0; row < actualHeight; ++row)
  {
    for(unsigned int column = 0; column < actualWidth; ++column)
    {
      unsigned char index = decodedData[row * width + column];

      pixels[0] = color[index].Red;
      pixels[1] = color[index].Green;
      pixels[2] = color[index].Blue;
      pixels += 3;
    }
  }
  finished = true;
  return true;
}

/// Called when we want to handle EXTENSION_RECORD_TYPE
bool HandleExtensionRecordType(GifFileType* gifInfo)
{
  SavedImage   image;
  GifByteType* extensionByte(NULL);

#ifdef LIBGIF_VERSION_5_1_OR_ABOVE
  ExtensionBlock extensionBlocks;
  image.ExtensionBlocks          = &extensionBlocks;
  image.ExtensionBlockCount      = 1;
  int* extensionBlockTypePointer = &image.ExtensionBlocks->Function;
#else
  image.ExtensionBlocks = NULL;
  image.ExtensionBlockCount = 0;
  int* extensionBlockTypePointer = &image.Function;
#endif

  // Not really interested in the extensions so just skip them unless there is an error.
  for(int extRetCode = DGifGetExtension(gifInfo, extensionBlockTypePointer, &extensionByte);
      extensionByte != NULL;
      extRetCode = DGifGetExtensionNext(gifInfo, &extensionByte))
  {
    if(DALI_UNLIKELY(extRetCode == GIF_ERROR))
    {
      DALI_LOG_ERROR("GIF Loader: Error reading GIF Extension record.\n");
      return false;
    }
  }

  return true;
}

} // unnamed namespace

bool LoadGifHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  GifFileType*   gifInfo = NULL;
  AutoCleanupGif autoCleanupGif(gifInfo);
  FILE* const    fp = input.file;

  return LoadGifHeader(fp, width, height, &gifInfo);
}

bool LoadBitmapFromGif(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap)
{
  FILE* const fp = input.file;
  // Load the GIF Header file.

  GifFileType* gifInfo(NULL);
  unsigned int width(0);
  unsigned int height(0);
  if(DALI_UNLIKELY(!LoadGifHeader(fp, width, height, &gifInfo)))
  {
    return false;
  }
  AutoCleanupGif autoGif(gifInfo);

  // Check each record in the GIF file.

  bool          finished(false);
  GifRecordType recordType(UNDEFINED_RECORD_TYPE);
  for(int returnCode = DGifGetRecordType(gifInfo, &recordType);
      !finished && recordType != TERMINATE_RECORD_TYPE;
      returnCode = DGifGetRecordType(gifInfo, &recordType))
  {
    if(DALI_UNLIKELY(returnCode == GIF_ERROR))
    {
      DALI_LOG_ERROR("GIF Loader: Error getting Record Type\n");
      return false;
    }

    if(IMAGE_DESC_RECORD_TYPE == recordType)
    {
      if(DALI_UNLIKELY(!HandleImageDescriptionRecordType(bitmap, gifInfo, width, height, finished)))
      {
        return false;
      }
    }
    else if(EXTENSION_RECORD_TYPE == recordType)
    {
      if(DALI_UNLIKELY(!HandleExtensionRecordType(gifInfo)))
      {
        return false;
      }
    }
  }

  return true;
}

} // namespace TizenPlatform

} // namespace Dali
