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

#include <dali/internal/imaging/common/loader-bmp.h>

#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/system/common/system-error-print.h>
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali
{
namespace TizenPlatform
{
namespace
{
const unsigned int FileHeaderOffsetOfBF32V4  = 0x7A;
const unsigned int MaskForBFRGB565           = 0x80;
const unsigned int FileHeaderOffsetOfRGB24V5 = 0x8A;

enum BmpFormat
{
  BMP_RGB1 = 14,     //BI_RGB & bpp =1
  BMP_RGB4,          //BI_RGB & bpp = 4
  BMP_RGB8,          //BI_RGB & bpp = 8
  BMP_RGB555,        //BI_RGB & bpp = 16
  BMP_BITFIELDS555,  //BI_BITFIELDS & 16bit & R:G:B = 5:5:5
  BMP_BITFIELDS32,   //BI_BITFIELDS & 32bit & R:G:B:A = 8:8:8:8
  BMP_RLE8,          //BI_RLE8
  BMP_RLE4,          //BI_RLE4
  BMP_BITFIELDS32V4, //BI_BITFIELDS & 32bit
  BMP_RGB24V5,       //BI_RGB & bpp = 24 & bmp version5
  BMP_NOTEXIST
};

struct BmpFileHeader
{
  unsigned short signature;    // Bitmap file signature
  unsigned int   fileSize;     // Bitmap file size in bytes
  unsigned short reserved1;    // Reserved bits
  unsigned short reserved2;    // Reserved bits
  unsigned int   offset;       // Offset from BMP file header to BMP bits
} __attribute__((__packed__)); // Stops the structure from being aligned to every 4 bytes

struct BmpInfoHeader
{
  unsigned int   infoHeaderSize;  // Specifies the number of bytes required by the info header
  unsigned int   width;           // The Image Width
  int            height;          // The Image Height (negative value represents image data is flipped)
  unsigned short planes;          // The number of color planes, must be 1
  unsigned short bitsPerPixel;    // The bits per pixel
  unsigned int   compression;     // The type of compression used by the image
  unsigned int   imageSize;       // The size of the image in bytes
  unsigned int   xPixelsPerMeter; // The number of pixels per meter in x axis
  unsigned int   yPixelsPerMeter; // The number of pixels per meter in y axis
  unsigned int   numberOfColors;  // The number of colors in the color table
  unsigned int   importantColors; // The important color count
} __attribute__((__packed__));    // Stops the structure from being aligned to every 4 bytes

/**
 * Template function to read from the file directly into our structure.
 * @param[in]  fp     The file to read from
 * @param[out] header The structure we want to store our information in
 * @return true, if read successful, false otherwise
 */
template<typename T>
inline bool ReadHeader(FILE* fp, T& header)
{
  const unsigned int readLength = sizeof(T);

  // Load the information directly into our structure
  if(DALI_UNLIKELY(fread(&header, 1, readLength, fp) != readLength))
  {
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  return true;
}

bool LoadBmpHeader(FILE* fp, unsigned int& width, unsigned int& height, BmpFileHeader& fileHeader, BmpInfoHeader& infoHeader)
{
  if(DALI_UNLIKELY(!ReadHeader(fp, fileHeader)))
  {
    DALI_LOG_ERROR("File header read failed\n");
    return false;
  }

  if(DALI_UNLIKELY(!ReadHeader(fp, infoHeader)))
  {
    DALI_LOG_ERROR("Info header read failed\n");
    return false;
  }

  width  = infoHeader.width;
  height = abs(infoHeader.height);

  if(DALI_UNLIKELY(infoHeader.width == 0))
  {
    DALI_LOG_ERROR("Invalid header size\n");
    return false;
  }

  return true;
}

/**
 * function to decode format BI_RGB & bpp = 24 & bmp version5.
 * @param[in]  fp      The file to read from
 * @param[out] pixels  The pointer that  we want to store bmp data  in
 * @param[in]  width   bmp width
 * @param[in]  height  bmp height
 * @param[in]  offset  offset from bmp header to bmp image data
 * @param[in]  topDown indicate image data is read from bottom or from top
 * @param[in]  padding padded to a u_int32 boundary for each line
 * @return true, if decode successful, false otherwise
 */
bool DecodeRGB24V5(FILE*          fp,
                   unsigned char* pixels,
                   unsigned int   width,
                   unsigned int   height,
                   unsigned int   offset,
                   bool           topDown,
                   unsigned int   rowStride,
                   unsigned int   padding)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_RGB24V5 format\n");
    return false;
  }
  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_RGB24V5 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  for(std::uint32_t yPos = 0; yPos < height; ++yPos)
  {
    std::uint8_t* pixelsPtr = NULL;
    if(topDown)
    {
      pixelsPtr = pixels + (yPos * rowStride);
    }
    else
    {
      pixelsPtr = pixels + (((height - 1) - yPos) * rowStride);
    }
    if(DALI_UNLIKELY(fread(pixelsPtr, 1, rowStride, fp) != rowStride))
    {
      DALI_LOG_ERROR("Error reading the BMP image\n");
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }
    for(std::uint32_t i = 0; i < rowStride; i += 3)
    {
      std::uint8_t temp = pixelsPtr[i];
      pixelsPtr[i]      = pixelsPtr[i + 2];
      pixelsPtr[i + 2]  = temp;
    }

    if(padding)
    {
      // move past the padding.
      if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(padding)), SEEK_CUR)))
      {
        DALI_LOG_ERROR("Error moving past BMP_RGB24V5 padding\n");
        DALI_PRINT_SYSTEM_ERROR_LOG();
      }
    }
  }
  return true;
}

/**
 * function to decode format BI_BITFIELDS & bpp = 32 & bmp version4.
 * @param[in]  fp        The file to read from
 * @param[out] pixels    The pointer that  we want to store bmp data  in
 * @param[in]  width     bmp width
 * @param[in]  height    bmp height
 * @param[in]  offset    offset from bmp header to bmp image data
 * @param[in]  topDown   indicate image data is read from bottom or from top
 * @param[in]  rowStride bits span for each line
 * @param[in]  padding   padded to a u_int32 boundary for each line
 * @return true, if decode successful, false otherwise
 */
bool DecodeBF32V4(FILE*          fp,
                  unsigned char* pixels,
                  unsigned int   width,
                  unsigned int   height,
                  unsigned int   offset,
                  bool           topDown,
                  unsigned int   rowStride,
                  unsigned int   padding)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_BITFIELDS32V4 format\n");
    return false;
  }
  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_BITFIELDS32V4 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  for(std::uint32_t yPos = 0; yPos < height; ++yPos)
  {
    std::uint8_t* pixelsPtr = NULL;
    if(topDown)
    {
      pixelsPtr = pixels + (yPos * rowStride);
    }
    else
    {
      pixelsPtr = pixels + (((height - 1) - yPos) * rowStride);
    }
    if(DALI_UNLIKELY(fread(pixelsPtr, 1, rowStride, fp) != rowStride))
    {
      DALI_LOG_ERROR("Error reading the BMP image\n");
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }
    for(std::uint32_t i = 0; i < rowStride; i += 4)
    {
      std::uint8_t temp = pixelsPtr[i];
      pixelsPtr[i]      = pixelsPtr[i + 2];
      pixelsPtr[i + 2]  = temp;
    }
    if(padding)
    {
      // move past the padding.
      if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(padding)), SEEK_CUR)))
      {
        DALI_LOG_ERROR("Error moving past BMP_BITFIELDS32V4 padding\n");
        DALI_PRINT_SYSTEM_ERROR_LOG();
      }
    }
  }
  return true;
}

/**
 * function to decode format BI_BITFIELDS & bpp = 32
 * @param[in]  fp        The file to read from
 * @param[out] pixels    The pointer that  we want to store bmp data  in
 * @param[in]  width     bmp width
 * @param[in]  height    bmp height
 * @param[in]  offset    offset from bmp header to bmp image data
 * @param[in]  topDown   indicate image data is read from bottom or from top
 * @param[in]  rowStride bits span for each line
 * @param[in]  padding   padded to a u_int32 boundary for each line
 * @return true, if decode successful, false otherwise
 */
bool DecodeBF32(FILE*          fp,
                unsigned char* pixels,
                unsigned int   width,
                unsigned int   height,
                unsigned int   offset,
                bool           topDown,
                unsigned int   rowStride,
                unsigned int   padding)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_BITFIELDS32 format\n");
    return false;
  }
  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_BITFIELDS32 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  for(std::uint32_t yPos = 0; yPos < height; ++yPos)
  {
    std::uint8_t* pixelsPtr;
    if(topDown)
    {
      // the data in the file is top down, and we store the data top down
      pixelsPtr = pixels + (yPos * rowStride);
    }
    else
    {
      // the data in the file is bottom up, and we store the data top down
      pixelsPtr = pixels + (((height - 1) - yPos) * rowStride);
    }

    if(DALI_UNLIKELY(fread(pixelsPtr, 1, rowStride, fp) != rowStride))
    {
      DALI_LOG_ERROR("Error reading the BMP image\n");
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }
    for(std::uint32_t i = 0; i < rowStride; i += 4)
    {
      std::uint8_t temp = pixelsPtr[i];
      pixelsPtr[i]      = pixelsPtr[i + 2];
      pixelsPtr[i + 2]  = temp;
    }

    if(padding)
    {
      // move past the padding.
      if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(padding)), SEEK_CUR)))
      {
        DALI_LOG_ERROR("Error moving past BMP_BITFIELDS32 padding\n");
        DALI_PRINT_SYSTEM_ERROR_LOG();
      }
    }
  }
  return true;
}

/**
 * function to decode format BI_BITFIELDS & bpp = 16 & R:G:B = 5:6:5
 * @param[in]  fp      The file to read from
 * @param[out] pixels  The pointer that  we want to store bmp data  in
 * @param[in]  width   bmp width
 * @param[in]  height  bmp height
 * @param[in]  offset  offset from bmp header to bmp image data
 * @param[in]  topDown indicate image data is read from bottom or from top
 * @return true, if decode successful, false otherwise
 */
bool DecodeBF565(FILE*          fp,
                 unsigned char* pixels,
                 unsigned int   width,
                 unsigned int   height,
                 unsigned int   offset,
                 bool           topDown)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding RGB565 format\n");
    return false;
  }
  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking RGB565 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  width                   = ((width & 3) != 0) ? width + 4 - (width & 3) : width;
  std::uint32_t rowStride = width * 2;

  for(std::uint32_t i = 0; i < height; ++i)
  {
    std::uint8_t* pixelsPtr = NULL;
    if(topDown)
    {
      // the data in the file is top down, and we store the data top down
      pixelsPtr = pixels + (i * rowStride);
    }
    else
    {
      // the data in the file is bottom up, and we store the data top down
      pixelsPtr = pixels + (((height - 1) - i) * rowStride);
    }
    if(DALI_UNLIKELY(fread(pixelsPtr, 1, rowStride, fp) != rowStride))
    {
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }
  }

  return true;
}

/**
 * function to decode format BI_BITFIELDS & bpp = 16 & R:G:B = 5:5:5
 * @param[in]  fp      The file to read from
 * @param[out] pixels  The pointer that  we want to store bmp data  in
 * @param[in]  width   bmp width
 * @param[in]  height  bmp height
 * @param[in]  offset  offset from bmp header to bmp image data
 * @param[in]  topDown indicate image data is read from bottom or from top
 * @return true, if decode successful, false otherwise
 */
bool DecodeBF555(FILE*          fp,
                 unsigned char* pixels,
                 unsigned int   width,
                 unsigned int   height,
                 unsigned int   offset,
                 bool           topDown)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_BITFIELDS555 format\n");
    return false;
  }

  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_BITFIELDS555 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  width = ((width & 3) != 0) ? width + 4 - (width & 3) : width;

  std::vector<std::uint8_t> raw(width * height * 2);
  std::uint32_t             rawStride = width * 2;
  std::uint32_t             rowStride = width * 3;

  std::uint8_t* rawPtr = NULL;
  for(std::uint32_t j = 0; j < height; ++j)
  {
    rawPtr = &raw[0] + (j * rawStride);
    if(DALI_UNLIKELY(fread(rawPtr, 1, rawStride, fp) != rawStride))
    {
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }
  }

  for(std::uint32_t yPos = 0; yPos < height; ++yPos)
  {
    std::uint8_t* pixelsPtr = NULL;
    if(topDown)
    {
      // the data in the file is top down, and we store the data top down
      pixelsPtr = pixels + (yPos * rowStride);
    }
    else
    {
      // the data in the file is bottom up, and we store the data top down
      pixelsPtr = pixels + (((height - 1) - yPos) * rowStride);
    }

    for(std::uint32_t k = 0; k < width; ++k)
    {
      std::uint32_t index  = yPos * rawStride + 2 * k;
      pixelsPtr[3 * k]     = ((raw[index + 1] >> 2) & 0x1F) * 0xFF / 0x1F;
      pixelsPtr[3 * k + 1] = (((raw[index + 1] & 0x03) << 3) | (raw[index] >> 5)) * 0xFF / 0x1F;
      pixelsPtr[3 * k + 2] = (raw[index] & 0x1F) * 0xFF / 0x1F;
    }
  }
  return true;
}

/**
 * function to decode format BI_RGB & bpp = 16 & R:G:B = 5:5:5
 * @param[in]  fp      The file to read from
 * @param[out] pixels  The pointer that  we want to store bmp data  in
 * @param[in]  width   bmp width
 * @param[in]  height  bmp height
 * @param[in]  offset  offset from bmp header to bmp image data
 * @param[in]  topDown indicate image data is read from bottom or from top
 * @return true, if decode successful, false otherwise
 */
bool DecodeRGB555(FILE*          fp,
                  unsigned char* pixels,
                  unsigned int   width,
                  unsigned int   height,
                  unsigned int   offset,
                  bool           topDown)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_RGB555 format\n");
    return false;
  }
  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_RGB555 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  width = ((width & 3) != 0) ? width + 4 - (width & 3) : width;
  std::vector<std::uint8_t> raw(width * height * 2);
  std::uint32_t             rawStride = width * 2;
  std::uint32_t             rowStride = width * 3;

  std::uint8_t* rawPtr = NULL;
  for(std::uint32_t j = 0; j < height; ++j)
  {
    rawPtr = &raw[0] + (j * rawStride);
    if(DALI_UNLIKELY(fread(rawPtr, 1, rawStride, fp) != rawStride))
    {
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }
  }
  for(std::uint32_t i = 0; i < height; ++i)
  {
    std::uint8_t* pixelsPtr = NULL;
    if(topDown)
    {
      // the data in the file is top down, and we store the data top down
      pixelsPtr = pixels + (i * rowStride);
    }
    else
    {
      // the data in the file is bottom up, and we store the data top down
      pixelsPtr = pixels + (((height - 1) - i) * rowStride);
    }
    for(std::uint32_t k = 0; k < width; ++k)
    {
      std::uint32_t index  = i * rawStride + 2 * k;
      pixelsPtr[3 * k]     = ((raw[index + 1] >> 2) & 0x1F) * 0xFF / 0x1F;
      pixelsPtr[3 * k + 1] = (((raw[index + 1] & 0x03) << 3) | (raw[index] >> 5)) * 0xFF / 0x1F;
      pixelsPtr[3 * k + 2] = (raw[index] & 0x1F) * 0xFF / 0x1F;
    }
  }
  return true;
}

/**
 * function to decode format BI_RGB & bpp = 1
 * @param[in]  fp      The file to read from
 * @param[out] pixels  The pointer that  we want to store bmp data  in
 * @param[in]  width   bmp width
 * @param[in]  height  bmp height
 * @param[in]  offset  offset from bmp header to bmp palette data
 * @param[in]  topDown indicate image data is read from bottom or from top
 * @return true, if decode successful, false otherwise
 */
bool DecodeRGB1(FILE*          fp,
                unsigned char* pixels,
                unsigned int   width,
                unsigned int   height,
                unsigned int   offset,
                bool           topDown)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_RGB1 format\n");
    return false;
  }
  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_RGB1 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  std::uint8_t              colorTable[8] = {0};
  std::uint8_t              cmd;
  std::uint32_t             fillw = ((width & 63) != 0) ? width + 64 - (width & 63) : width;
  std::vector<std::uint8_t> colorIndex(fillw * height);
  std::uint32_t             rowStride = fillw * 3; // RGB

  if(DALI_UNLIKELY(fread(colorTable, 1, 8, fp) != 8))
  {
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  for(std::uint32_t i = 0; i < fillw * height; i += 8)
  {
    if(DALI_UNLIKELY(fread(&cmd, 1, 1, fp) != 1))
    {
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }

    colorIndex[i]     = (cmd >> 7) & 0x01;
    colorIndex[i + 1] = (cmd >> 6) & 0x01;
    colorIndex[i + 2] = (cmd >> 5) & 0x01;
    colorIndex[i + 3] = (cmd >> 4) & 0x01;
    colorIndex[i + 4] = (cmd >> 3) & 0x01;
    colorIndex[i + 5] = (cmd >> 2) & 0x01;
    colorIndex[i + 6] = (cmd >> 1) & 0x01;
    colorIndex[i + 7] = (cmd & 0x01);
  }

  for(std::uint32_t index = 0; index < height; ++index)
  {
    std::uint8_t* pixelsPtr = NULL;
    if(topDown)
    {
      // the data in the file is top down, and we store the data top down
      pixelsPtr = pixels + (index * rowStride);
    }
    else
    {
      // the data in the file is bottom up, and we store the data top down
      pixelsPtr = pixels + (((height - 1) - index) * rowStride);
    }
    for(std::uint32_t j = 0; j < fillw; ++j)
    {
      std::uint32_t ctIndex = 0;
      if((fillw * index + j) < (fillw * height))
      {
        ctIndex = colorIndex[fillw * index + j];
      }
      else
      {
        break;
      }
      // temp solution for PLM bug P130411-5268, there is one mono bmp that cause DecodeRGB1 API crash.
      if(((3 * j + 2) < height * fillw * 3) && (ctIndex < 2))
      {
        pixelsPtr[3 * j]     = colorTable[4 * ctIndex + 2];
        pixelsPtr[3 * j + 1] = colorTable[4 * ctIndex + 1];
        pixelsPtr[3 * j + 2] = colorTable[4 * ctIndex];
      }
    }
  }
  return true;
}

/**
 * function to decode format BI_RGB & bpp = 4
 * @param[in]  fp      The file to read from
 * @param[out] pixels  The pointer that  we want to store bmp data  in
 * @param[in]  width   bmp width
 * @param[in]  height  bmp height
 * @param[in]  offset  offset from bmp header to bmp palette data
 * @param[in]  topDown indicate image data is read from bottom or from top
 * @return true, if decode successful, false otherwise
 */
bool DecodeRGB4(FILE*          fp,
                unsigned char* pixels,
                unsigned int   width,
                unsigned int   height,
                unsigned int   offset,
                bool           topDown)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_RGB4 format\n");
    return false;
  }
  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_RGB4 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  std::uint8_t              colorTable[64];
  std::uint8_t              cmd;
  std::uint32_t             fillw = ((width & 3) != 0) ? width + 4 - (width & 3) : width;
  std::vector<std::uint8_t> colorIndex(fillw * height);
  std::uint32_t             rowStride = fillw * 3;

  if(DALI_UNLIKELY(fread(colorTable, 1, 64, fp) != 64))
  {
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  for(std::uint32_t i = 0; i < fillw * height; i += 2)
  {
    if(DALI_UNLIKELY(fread(&cmd, 1, 1, fp) != 1))
    {
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }

    colorIndex[i]     = cmd >> 4;
    colorIndex[i + 1] = cmd & (0x0F);
  }
  std::uint32_t ctIndex = 0;

  for(std::uint32_t index = 0; index < height; ++index)
  {
    std::uint8_t* pixelsPtr = NULL;
    if(topDown)
    {
      // the data in the file is top down, and we store the data top down
      pixelsPtr = pixels + (index * rowStride);
    }
    else
    {
      // the data in the file is bottom up, and we store the data top down
      pixelsPtr = pixels + (((height - 1) - index) * rowStride);
    }
    for(std::uint32_t j = 0; j < fillw; ++j)
    {
      ctIndex                = colorIndex[fillw * index + j];
      pixelsPtr[3 * j]       = colorTable[4 * ctIndex + 2];
      pixelsPtr[(3 * j + 1)] = colorTable[4 * ctIndex + 1];
      pixelsPtr[(3 * j + 2)] = colorTable[4 * ctIndex];
    }
  }

  return true;
}

/**
 * function to decode format BI_RGB & bpp = 8
 * @param[in]  fp      The file to read from
 * @param[out] pixels  The pointer that  we want to store bmp data  in
 * @param[in]  width   bmp width
 * @param[in]  height  bmp height
 * @param[in]  offset  offset from bmp header to bmp palette data
 * @param[in]  topDown indicate image data is read from bottom or from top
 * @return true, if decode successful, false otherwise
 */
bool DecodeRGB8(FILE*          fp,
                unsigned char* pixels,
                unsigned int   width,
                unsigned int   height,
                unsigned int   offset,
                bool           topDown)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_RGB8 format\n");
    return false;
  }
  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_RGB8 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  std::vector<std::uint8_t> colorTable(1024);
  width = ((width & 3) != 0) ? width + 4 - (width & 3) : width;
  std::vector<std::uint8_t> colorIndex(width * height);
  std::uint32_t             rowStride = width * 3; //RGB8->RGB24

  if(DALI_UNLIKELY(fread(&colorTable[0], 1, 1024, fp) != 1024))
  {
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }
  if(DALI_UNLIKELY(fread(&colorIndex[0], 1, width * height, fp) != width * height))
  {
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }
  std::uint8_t ctIndex = 0;
  for(std::uint32_t index = 0; index < height; ++index)
  {
    std::uint8_t* pixelsPtr = NULL;
    if(topDown)
    {
      // the data in the file is top down, and we store the data top down
      pixelsPtr = pixels + (index * rowStride);
    }
    else
    {
      // the data in the file is bottom up, and we store the data top down
      pixelsPtr = pixels + (((height - 1) - index) * rowStride);
    }
    for(std::uint32_t j = 0; j < width; ++j)
    {
      ctIndex                = colorIndex[width * index + j];
      pixelsPtr[3 * j]       = colorTable[4 * ctIndex + 2];
      pixelsPtr[(3 * j + 1)] = colorTable[4 * ctIndex + 1];
      pixelsPtr[(3 * j + 2)] = colorTable[4 * ctIndex];
    }
  }
  return true;
}

/**
 * function to decode format BI_RLE4 & bpp = 4
 * @param[in]  fp      The file to read from
 * @param[out] pixels  The pointer that  we want to store bmp data  in
 * @param[in]  width   bmp width
 * @param[in]  height  bmp height
 * @param[in]  offset  offset from bmp header to bmp palette data
 * @param[in]  topDown indicate image data is read from bottom or from top
 * @return true, if decode successful, false otherwise
 */
bool DecodeRLE4(FILE*          fp,
                unsigned char* pixels,
                unsigned int   width,
                unsigned int   height,
                unsigned int   offset,
                bool           topDown)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_RLE4 format\n");
    return false;
  }
  std::uint8_t* pixelsPtr = pixels;
  width                   = ((width & 3) != 0) ? width + 4 - (width & 3) : width;
  std::uint8_t              cmd[2];
  std::uint32_t             cmdStride = 2;
  std::uint8_t              colorTable[64];
  std::vector<std::uint8_t> colorIndex(width * height >> 1);
  std::vector<std::uint8_t> run;
  std::uint32_t             x  = 0;
  std::uint32_t             y  = 0;
  std::uint32_t             dx = 0;
  std::uint32_t             dy = 0;
  width += (width & 1);
  width = width >> 1;

  bool finish = false;

  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_RLE4 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fread(colorTable, 1, 64, fp) != 64))
  {
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  while((x >> 1) + y * width < width * height)
  {
    if(finish)
    {
      break;
    }
    if(DALI_UNLIKELY(fread(cmd, 1, cmdStride, fp) != cmdStride))
    {
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }
    if(cmd[0] == 0) // ESCAPE
    {
      switch(cmd[1])
      {
        case 1: //end of bitmap
          finish = true;
          break;
        case 0: // end of line
          x = 0;
          y++;
          break;
        case 2: // delta
          if(DALI_UNLIKELY(fread(cmd, 1, cmdStride, fp) != cmdStride))
          {
            DALI_LOG_ERROR("Error reading the BMP image\n");
            DALI_PRINT_SYSTEM_ERROR_LOG();
            return false;
          }
          dx = cmd[0] & (0xFF);
          dy = cmd[1] & (0xFF);
          x += dx;
          y += dy;
          break;
        default:
          // decode a literal run
          std::uint32_t length = cmd[1] & (0xFF);
          //size of run, which is word aligned
          std::uint32_t bytesize = length;
          bytesize += (bytesize & 1);
          bytesize >>= 1;
          bytesize += (bytesize & 1);
          run.resize(bytesize);
          if(DALI_UNLIKELY(fread(&run[0], 1, bytesize, fp) != bytesize))
          {
            DALI_LOG_ERROR("Error reading the BMP image\n");
            DALI_PRINT_SYSTEM_ERROR_LOG();
            return false;
          }
          if((x & 1) == 0)
          {
            length += (length & 1);
            length >>= 1;
            for(std::uint32_t i = 0; i < length; ++i)
            {
              colorIndex[(x >> 1) + width * (height - y - 1) + i] = run[i];
            }
          }
          else
          {
            for(std::uint32_t i = 0; i < length; ++i)
            {
              if((i & 1) == 0) //copy high to low
              {
                colorIndex[((x + i) >> 1) + width * (height - y - 1)] |= ((run[i >> 1] & 0xF0) >> 4);
              }
              else //copy low to high
              {
                colorIndex[((x + i) >> 1) + width * (height - y - 1)] |= ((run[i >> 1] & 0x0F) << 4);
              }
            }
          }
          x += cmd[1] & (0xFF);
          break;
      }
    }
    else
    {
      std::uint32_t length = cmd[0] & (0xFF);
      if((x & 1) == 0)
      {
        length += (length & 1);
        length >>= 1;
        for(std::uint32_t i = 0; i < length; ++i)
        {
          colorIndex[(height - y - 1) * width + i + (x >> 1)] = cmd[1];
        }
      }
      else
      {
        for(std::uint32_t i = 0; i < length; ++i)
        {
          if((i & 1) == 0)
          {
            colorIndex[((x + i) >> 1) + width * (height - y - 1)] |= ((cmd[1] & 0xF0) >> 4);
          }
          else
          {
            colorIndex[((x + i) >> 1) + width * (height - y - 1)] |= ((cmd[1] & 0x0F) << 4);
          }
        }
      }
      x += cmd[0] & (0xFF);
    }
  }

  std::uint32_t ctIndexHigh = 0;
  std::uint32_t ctIndexLow  = 0;
  for(std::uint32_t index = 0; index < (width * height); ++index)
  {
    ctIndexHigh              = colorIndex[index] >> 4;
    ctIndexLow               = colorIndex[index] & (0x0F);
    pixelsPtr[6 * index]     = colorTable[4 * ctIndexHigh + 2];
    pixelsPtr[6 * index + 1] = colorTable[4 * ctIndexHigh + 1];
    pixelsPtr[6 * index + 2] = colorTable[4 * ctIndexHigh];
    pixelsPtr[6 * index + 3] = colorTable[4 * ctIndexLow + 2];
    pixelsPtr[6 * index + 4] = colorTable[4 * ctIndexLow + 1];
    pixelsPtr[6 * index + 5] = colorTable[4 * ctIndexLow];
  }
  return true;
}

/**
 * function to decode format BI_RLE8 & bpp = 8
 * @param[in]  fp      The file to read from
 * @param[out] pixels  The pointer that  we want to store bmp data  in
 * @param[in]  width   bmp width
 * @param[in]  height  bmp height
 * @param[in]  offset  offset from bmp header to bmp palette data
 * @param[in]  topDown indicate image data is read from bottom or from top
 * @return true, if decode successful, false otherwise
 */
bool DecodeRLE8(FILE*          fp,
                unsigned char* pixels,
                unsigned int   width,
                unsigned int   height,
                unsigned int   offset,
                bool           topDown)
{
  if(DALI_UNLIKELY(fp == NULL || pixels == NULL))
  {
    DALI_LOG_ERROR("Error decoding BMP_RLE8 format\n");
    return false;
  }
  std::uint8_t* pixelsPtr = pixels;
  std::uint32_t x         = 0;
  std::uint32_t y         = 0;
  std::uint32_t cmdStride = 2;

  width = ((width & 3) != 0) ? width + 4 - (width & 3) : width;
  std::vector<std::uint8_t> colorTable(1024);
  std::uint8_t              cmd[2];
  std::vector<std::uint8_t> colorIndex(width * height);

  if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(offset)), SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking BMP_RLE8 data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fread(&colorTable[0], 1, 1024, fp) != 1024))
  {
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  std::uint32_t             dx         = 0;
  std::uint32_t             dy         = 0;
  bool                      finish     = false;
  std::uint32_t             length     = 0;
  std::uint32_t             copylength = 0;
  std::vector<std::uint8_t> run;
  while((x + y * width) < width * height)
  {
    if(DALI_UNLIKELY(finish))
    {
      break;
    }
    if(DALI_UNLIKELY(fread(cmd, 1, cmdStride, fp) != cmdStride))
    {
      DALI_PRINT_SYSTEM_ERROR_LOG();
      return false;
    }

    if(cmd[0] == 0) //ESCAPE
    {
      switch(cmd[1])
      {
        case 1: // end of bitmap
          finish = true;
          break;
        case 0: // end of line
          x = 0;
          y++;
          break;
        case 2: // delta
          if(DALI_UNLIKELY(fread(cmd, 1, cmdStride, fp) != cmdStride))
          {
            DALI_LOG_ERROR("Error reading the BMP image\n");
            DALI_PRINT_SYSTEM_ERROR_LOG();
            return false;
          }
          dx = cmd[0] & (0xFF);
          dy = cmd[1] & (0xFF);
          x += dx;
          y += dy;
          break;
        default:
          //decode a literal run
          length     = cmd[1] & (0xFF);
          copylength = length;
          //absolute mode must be word-aligned
          length += (length & 1);
          run.resize(length);
          if(DALI_UNLIKELY(fread(&run[0], 1, length, fp) != length))
          {
            DALI_LOG_ERROR("Error reading the BMP image\n");
            DALI_PRINT_SYSTEM_ERROR_LOG();
            return false;
          }

          for(std::uint32_t i = 0; i < length; ++i)
          {
            colorIndex[x + width * (height - y - 1) + i] = run[i];
          }
          x += copylength;
          break;
      }
    } // end if cmd[0] ==
    else
    {
      length = cmd[0] & (0xFF);
      for(std::uint32_t i = 0; i < length; ++i)
      {
        colorIndex[(height - y - 1) * width + x] = cmd[1];
        x++;
      }
    }
  }
  std::uint32_t ctIndex = 0;
  for(std::uint32_t index = 0; index < width * height; ++index)
  {
    ctIndex                  = colorIndex[index];
    pixelsPtr[3 * index]     = colorTable[4 * ctIndex + 2];
    pixelsPtr[3 * index + 1] = colorTable[4 * ctIndex + 1];
    pixelsPtr[3 * index + 2] = colorTable[4 * ctIndex];
  }
  return true;
}

} // unnamed namespace

bool LoadBmpHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  BmpFileHeader fileHeader;
  BmpInfoHeader infoHeader;

  bool ret = LoadBmpHeader(input.file, width, height, fileHeader, infoHeader);

  return ret;
}

bool LoadBitmapFromBmp(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap)
{
  //DALI_ASSERT_DEBUG( bitmap.GetPackedPixelsProfile() != 0 && "Need a packed pixel bitmap to load into." );
  FILE* const fp = input.file;
  if(DALI_UNLIKELY(fp == NULL))
  {
    DALI_LOG_ERROR("Error loading bitmap\n");
    return false;
  }
  BmpFormat     customizedFormat = BMP_NOTEXIST;
  BmpFileHeader fileHeader;
  BmpInfoHeader infoHeader;

  // Load the header info
  unsigned int width, height;

  if(DALI_UNLIKELY(!LoadBmpHeader(fp, width, height, fileHeader, infoHeader)))
  {
    return false;
  }

  Pixel::Format pixelFormat = Pixel::RGB888;
  switch(infoHeader.compression)
  {
    case 0:
    {
      switch(infoHeader.bitsPerPixel)
      {
        case 32:
        {
          pixelFormat = Pixel::RGBA8888;
          break;
        }

        case 24:
        {
          if(fileHeader.offset == FileHeaderOffsetOfRGB24V5) //0x8A
          {
            customizedFormat = BMP_RGB24V5;
          }
          else
          {
            pixelFormat = Pixel::RGB888;
          }
          break;
        }

        case 16:
        {
          customizedFormat = BMP_RGB555;
          break;
        }

        case 8:
        {
          customizedFormat = BMP_RGB8;
          break;
        }

        case 4: // RGB4
        {
          customizedFormat = BMP_RGB4;
          break;
        }

        case 1: //RGB1
        {
          customizedFormat = BMP_RGB1;
          break;
        }

        default:
        {
          DALI_LOG_ERROR("%d bits per pixel not supported for BMP files\n", infoHeader.bitsPerPixel);
          return false;
        }
      }
      break;
    }
    case 1: //// RLE8
    {
      if(infoHeader.bitsPerPixel == 8)
      {
        customizedFormat = BMP_RLE8;
      }
      break;
    }
    case 2: // RLE4
    {
      if(infoHeader.bitsPerPixel == 4)
      {
        customizedFormat = BMP_RLE4;
      }
      break;
    }
    case 3: // // BI_BITFIELDS
    {
      if(infoHeader.bitsPerPixel == 16)
      {
        if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(14 + infoHeader.infoHeaderSize + 1)), SEEK_SET)))
        {
          DALI_PRINT_SYSTEM_ERROR_LOG();
          return false;
        }

        char mask;
        if(DALI_UNLIKELY(fread(&mask, 1, 1, fp) != 1))
        {
          DALI_PRINT_SYSTEM_ERROR_LOG();
          return false;
        }

        if((mask & 0x80) == MaskForBFRGB565) // mask is 0xF8
        {
          pixelFormat = Pixel::RGB565;
        }
        else if((mask & 0x80) == 0) // mask is 0x 7C
        {
          customizedFormat = BMP_BITFIELDS555;
        }
        else
        {
          return false;
        }
      }
      else if(infoHeader.bitsPerPixel == 32)
      {
        if(fileHeader.offset == FileHeaderOffsetOfBF32V4) // 0x7A
        {
          customizedFormat = BMP_BITFIELDS32V4;
        }
        else
        {
          customizedFormat = BMP_BITFIELDS32;
        }
      }
      break;
    }
    default:
    {
      DALI_LOG_ERROR("Compression not supported for BMP files\n");
      return false;
    }
  }

  bool topDown = false;

  // if height is negative, bitmap data is top down
  if(infoHeader.height < 0)
  {
    infoHeader.height = abs(infoHeader.height);
    height            = infoHeader.height;
    topDown           = true;
  }

  unsigned int rowStride = infoHeader.width * (infoHeader.bitsPerPixel >> 3);

  // bitmaps row stride is padded to 4 bytes
  unsigned int padding = (rowStride % 4);
  if(padding)
  {
    padding = 4 - padding;
  }

  int  imageW         = infoHeader.width;
  int  pixelBufferW   = infoHeader.width;
  int  pixelBufferH   = infoHeader.height;
  auto newPixelFormat = Pixel::Format::INVALID;

  switch(customizedFormat)
  {
    case BMP_RLE8:
    case BMP_RGB8:
    case BMP_RGB4:
    case BMP_RLE4:
    case BMP_RGB555:
    case BMP_BITFIELDS555:
    {
      pixelBufferW   = ((imageW & 3) != 0) ? imageW + 4 - (imageW & 3) : imageW;
      pixelBufferH   = abs(infoHeader.height);
      newPixelFormat = Pixel::RGB888;
      break;
    }
    case BMP_RGB1:
    {
      pixelBufferW   = ((imageW & 63) != 0) ? imageW + 64 - (imageW & 63) : imageW;
      pixelBufferH   = abs(infoHeader.height);
      newPixelFormat = Pixel::RGB888;
      break;
    }
    case BMP_BITFIELDS32:
    case BMP_BITFIELDS32V4:
    {
      pixelBufferH   = abs(infoHeader.height);
      newPixelFormat = Pixel::RGBA8888;
      break;
    }
    case BMP_RGB24V5:
    {
      newPixelFormat = Pixel::RGB888;
      break;
    }
    default:
    {
      if(pixelFormat == Pixel::RGB565)
      {
        pixelBufferW   = ((imageW & 3) != 0) ? imageW + 4 - (imageW & 3) : imageW;
        pixelBufferH   = abs(infoHeader.height);
        newPixelFormat = Pixel::RGB565;
      }
      else
      {
        pixelBufferW   = infoHeader.width;
        pixelBufferH   = infoHeader.height;
        newPixelFormat = pixelFormat;
      }
      break;
    }
  }

  bitmap      = Dali::Devel::PixelBuffer::New(pixelBufferW, pixelBufferH, newPixelFormat);
  auto pixels = bitmap.GetBuffer();

  // Read the raw bitmap data
  decltype(pixels) pixelsIterator = nullptr;

  bool decodeResult(false);
  switch(customizedFormat)
  {
    case BMP_RGB1:
    {
      decodeResult = DecodeRGB1(fp, pixels, infoHeader.width, abs(infoHeader.height), 14 + infoHeader.infoHeaderSize, topDown);
      break;
    }
    case BMP_RGB4:
    {
      decodeResult = DecodeRGB4(fp, pixels, infoHeader.width, abs(infoHeader.height), 14 + infoHeader.infoHeaderSize, topDown);
      break;
    }
    case BMP_RLE4:
    {
      decodeResult = DecodeRLE4(fp, pixels, infoHeader.width, abs(infoHeader.height), 14 + infoHeader.infoHeaderSize, topDown);
      break;
    }
    case BMP_BITFIELDS32:
    {
      decodeResult = DecodeBF32(fp, pixels, infoHeader.width, abs(infoHeader.height), fileHeader.offset, topDown, rowStride, padding);
      break;
    }
    case BMP_BITFIELDS555:
    {
      decodeResult = DecodeBF555(fp, pixels, infoHeader.width, abs(infoHeader.height), fileHeader.offset, topDown);
      break;
    }
    case BMP_RGB555:
    {
      decodeResult = DecodeRGB555(fp, pixels, infoHeader.width, abs(infoHeader.height), fileHeader.offset, topDown);
      break;
    }
    case BMP_RGB8:
    {
      decodeResult = DecodeRGB8(fp, pixels, infoHeader.width, abs(infoHeader.height), 14 + infoHeader.infoHeaderSize, topDown);
      break;
    }
    case BMP_RLE8:
    {
      decodeResult = DecodeRLE8(fp, pixels, infoHeader.width, abs(infoHeader.height), 14 + infoHeader.infoHeaderSize, topDown);
      break;
    }
    case BMP_RGB24V5:
    {
      decodeResult = DecodeRGB24V5(fp, pixels, infoHeader.width, abs(infoHeader.height), fileHeader.offset, topDown, rowStride, padding);
      break;
    }
    case BMP_BITFIELDS32V4:
    {
      decodeResult = DecodeBF32V4(fp, pixels, infoHeader.width, abs(infoHeader.height), fileHeader.offset, topDown, rowStride, padding);
      break;
    }
    default:
    {
      if(pixelFormat == Pixel::RGB565)
      {
        decodeResult = DecodeBF565(fp, pixels, infoHeader.width, abs(infoHeader.height), fileHeader.offset, topDown);
      }
      else
      {
        for(unsigned int yPos = 0; yPos < height; yPos++)
        {
          if(topDown)
          {
            // the data in the file is top down, and we store the data top down
            pixelsIterator = pixels + (yPos * rowStride);
          }
          else
          {
            // the data in the file is bottom up, and we store the data top down
            pixelsIterator = pixels + (((height - 1) - yPos) * rowStride);
          }

          if(DALI_UNLIKELY(fread(pixelsIterator, 1, rowStride, fp) != rowStride))
          {
            DALI_LOG_ERROR("Error reading the BMP image\n");
            DALI_PRINT_SYSTEM_ERROR_LOG();
            break;
          }

          // If 32 bit mode then swap Blue and Red pixels. And Alpha pixels must be ignored.
          // Reference : https://users.cs.fiu.edu/~czhang/teaching/cop4225/project_files/bitmap_format.htm
          // ... if the compression field of the bitmap is set to bi_rgb, ... the high byte in each dword is not used.
          // RGB8888 format doesn't seem to be supported by graphics-api
          if(infoHeader.bitsPerPixel == 32)
          {
            for(unsigned int i = 0; i < rowStride; i += 4)
            {
              uint8_t temp          = pixelsIterator[i];
              pixelsIterator[i]     = pixelsIterator[i + 2];
              pixelsIterator[i + 2] = temp;
              pixelsIterator[i + 3] = 255u;
            }
          }

          // If 24 bit mode then swap Blue and Red pixels
          // BGR888 doesn't seem to be supported by dali-core
          if(infoHeader.bitsPerPixel == 24)
          {
            for(unsigned int i = 0; i < rowStride; i += 3)
            {
              uint8_t temp          = pixelsIterator[i];
              pixelsIterator[i]     = pixelsIterator[i + 2];
              pixelsIterator[i + 2] = temp;
            }
          }

          if(padding)
          {
            if(DALI_UNLIKELY(fseek(fp, static_cast<long>(static_cast<size_t>(padding)), SEEK_CUR))) // move past the padding.
            {
              DALI_LOG_ERROR("Error moving past BMP padding\n");
              DALI_PRINT_SYSTEM_ERROR_LOG();
            }
          }
        }
        decodeResult = true;
      }
      break;
    }
  } // switch

  if(DALI_UNLIKELY(!decodeResult))
  {
    DALI_LOG_ERROR("Decoding failed\n");
    return false;
  }

  return true;
}

} // namespace TizenPlatform

} // namespace Dali
