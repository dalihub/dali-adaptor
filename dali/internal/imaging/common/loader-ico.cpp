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
 */

/*
 * Derived from Enlightenment file evas_image_load_ico.c[1]  which is licensed
 * under the BSD 2-clause license[2] reproduced below.
 *
 * [1][http://web.archive.org/web/20141201151111/http://git.enlightenment.org/core/efl.git/tree/src/modules/evas/loaders/ico/evas_image_load_ico.c]
 * [2][http://web.archive.org/web/20140717012400/https://git.enlightenment.org/core/efl.git/about/]
 *
 * Copyright (C) 2002-2012 Carsten Haitzler, Dan Sinclair, Mike Blumenkrantz,
 * Samsung Electronics and various contributors (see AUTHORS)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// HEADER
#include <dali/internal/imaging/common/loader-ico.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/system/common/system-error-print.h>

namespace Dali
{
namespace TizenPlatform
{
namespace
{
// Reserved 2 bytes + type 2 bytes + count 2 bytes + count * 16 bytes
const unsigned char ICO_FILE_HEADER = 22;
// Info header 40 bytes = size 4 bytes + width 4 bytes + height 4 bytes + planes 2 bytes + bitcount 2 bytes
// + compression 4 bytes + imagesize 4 bytes + xpixelsPerM 4 bytes + ypixelsPerM 4 bytes + colorsUsed 4 bytes + colorImportant 4 bytes
// besides, there are rgba color data = numberOfColors * 4 bytes
const unsigned char ICO_IMAGE_INFO_HEADER = 40;

typedef unsigned char DATA8;
#define A_VAL(p) (reinterpret_cast<DATA8*>(p)[3])

#define RGB_JOIN(r, g, b) \
  (((r) << 16) + ((g) << 8) + (b))

#define ARGB_JOIN(a, r, g, b) \
  (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

bool read_ushort(const unsigned char* const& map, size_t length, size_t* position, unsigned short* ret)
{
  unsigned char b[2];

  if(DALI_UNLIKELY(*position + 2 > length))
  {
    return false;
  }
  b[0] = map[(*position)++];
  b[1] = map[(*position)++];
  *ret = (b[1] << 8) | b[0];
  return true;
}

bool read_uint(const unsigned char* const& map, size_t length, size_t* position, unsigned int* ret)
{
  unsigned char b[4];
  unsigned int  i;

  if(DALI_UNLIKELY(*position + 4 > length))
  {
    return false;
  }
  for(i = 0; i < 4; i++)
  {
    b[i] = map[(*position)++];
  }
  *ret = ARGB_JOIN(b[3], b[2], b[1], b[0]);
  return true;
}

bool read_uchar(const unsigned char* const& map, size_t length, size_t* position, unsigned char* ret)
{
  if(DALI_UNLIKELY(*position + 1 > length))
  {
    return false;
  }
  *ret = map[(*position)++];
  return true;
}

bool read_mem(const unsigned char* const& map, size_t length, size_t* position, void* buffer, int size)
{
  if(DALI_UNLIKELY(*position + size > length))
  {
    return false;
  }
  memcpy(buffer, map + *position, size);
  *position += size;
  return true;
}

enum
{
  SMALLEST,
  BIGGEST,
  SMALLER,
  BIGGER
};

enum
{
  ICON   = 1,
  CURSOR = 2
};

struct IcoData
{
  int          pdelta;
  int          w, h;
  int          cols;
  int          bpp, planes;
  int          hot_x, hot_y;
  unsigned int bmoffset, bmsize;
};

bool LoadIcoHeaderHelper(FILE*                        fp,
                         IcoData&                     chosen,
                         Dali::Vector<unsigned char>& map,
                         unsigned int&                fsize)
{
  memset(&chosen, 0, sizeof(chosen));

  if(DALI_UNLIKELY(fp == NULL))
  {
    DALI_LOG_ERROR("Error loading bitmap\n");
    return false;
  }
  size_t         position = 0;
  unsigned short word;
  unsigned char  byte;

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_END)))
  {
    DALI_LOG_ERROR("Error seeking ICO data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  long positionIndicator = ftell(fp);
  fsize                  = 0u;

  if(positionIndicator > -1L)
  {
    fsize = static_cast<unsigned int>(positionIndicator);
  }

  if(DALI_UNLIKELY(0u == fsize))
  {
    DALI_LOG_ERROR("Error ICO data size is zero!\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking ICO data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fsize < (ICO_FILE_HEADER + ICO_IMAGE_INFO_HEADER))) //6 + 16 + 40
  {
    DALI_LOG_ERROR("Error ICO data size is too small! (%ld < %u)!\n", fsize, static_cast<uint32_t>(ICO_FILE_HEADER + ICO_IMAGE_INFO_HEADER));
    return false;
  }
  map.ResizeUninitialized(fsize);
  if(DALI_UNLIKELY(fread(&map[0], 1, fsize, fp) != fsize))
  {
    DALI_LOG_ERROR("image file read opeation error!\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  const std::uint8_t* const inputBufferPtr = &map[0];

  int            search = BIGGEST;
  unsigned short reserved, type, count;
  if(DALI_UNLIKELY(!read_ushort(inputBufferPtr, fsize, &position, &reserved)))
  {
    DALI_LOG_ERROR("Error ICO header.reserved decode failed!\n");
    return false;
  }
  if(DALI_UNLIKELY(!read_ushort(inputBufferPtr, fsize, &position, &type)))
  {
    DALI_LOG_ERROR("Error ICO header.type decode failed!\n");
    return false;
  }
  if(DALI_UNLIKELY(!read_ushort(inputBufferPtr, fsize, &position, &count)))
  {
    DALI_LOG_ERROR("Error ICO header.count decode failed!\n");
    return false;
  }
  if(DALI_UNLIKELY(!((reserved == 0) &&
                     ((type == ICON) || (type == CURSOR)) && (count != 0))))
  {
    DALI_LOG_ERROR("Error ICO header is invalid! (reserved : %hu, type : %hu, count : %hu)\n", reserved, type, count);
    return false;
  }
  search           = BIGGEST;
  chosen.pdelta    = 0;
  bool have_choice = false;

  for(unsigned short i = 0; i < count; i++)
  {
    unsigned char tw = 0, th = 0, tcols = 0;
    if(DALI_UNLIKELY(!read_uchar(inputBufferPtr, fsize, &position, &tw)))
    {
      return false;
    }
    int w = tw;
    if(w <= 0)
    {
      w = 256;
    }
    if(DALI_UNLIKELY(!read_uchar(inputBufferPtr, fsize, &position, &th)))
    {
      return false;
    }
    int h = th;
    if(h <= 0)
    {
      h = 256;
    }
    if(DALI_UNLIKELY(!read_uchar(inputBufferPtr, fsize, &position, &tcols)))
    {
      return false;
    }
    int cols = tcols;
    if(DALI_UNLIKELY(!read_uchar(inputBufferPtr, fsize, &position, &byte)))
    {
      return false;
    }
    if(DALI_UNLIKELY(!read_ushort(inputBufferPtr, fsize, &position, &word)))
    {
      return false;
    }
    int planes = 0;
    if(type == 1)
    {
      planes = word;
    }
    //else hot_x = word;
    if(DALI_UNLIKELY(!read_ushort(inputBufferPtr, fsize, &position, &word)))
    {
      return false;
    }
    int bpp = 0;
    if(type == 1)
    {
      bpp = word;
    }

    // 0 colors means 256 for paletized modes.
    // Note: We must not do this conversion for bpp greater than 8, as there is no palette.
    if(bpp <= 8 && cols == 0)
    {
      cols = 256;
    }

    //else hot_y = word;
    unsigned int bmoffset, bmsize;
    if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &bmsize)))
    {
      return false;
    }
    if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &bmoffset)))
    {
      return false;
    }
    if(DALI_UNLIKELY((bmsize <= 0) || (bmoffset <= 0) || (bmoffset >= fsize)))
    {
      return false;
    }
    if(search == BIGGEST)
    {
      int pdelta = w * h;
      if((!have_choice) ||
         ((pdelta >= chosen.pdelta) &&
          (((bpp >= 3) && (bpp >= chosen.bpp)) ||
           ((bpp < 3) && (cols >= chosen.cols)))))
      {
        have_choice     = true;
        chosen.pdelta   = pdelta;
        chosen.w        = w;
        chosen.h        = h;
        chosen.cols     = cols;
        chosen.bpp      = bpp;
        chosen.planes   = planes;
        chosen.bmsize   = bmsize;
        chosen.bmoffset = bmoffset;
      }
    }
  }

  if(DALI_UNLIKELY(chosen.bmoffset == 0))
  {
    DALI_LOG_ERROR("Error ICO data is invalid!\n");
    return false;
  }

  return true;
}

/**
 * @brief Handle the different bits per pixel
 * @param[in] bitcount The bit count
 * @param[in] inputBufferPtr The map to use
 * @param[in/out] pix A reference to the pointer to the pix buffer
 * @param[in/out] outputBufferPtr A reference to the surface buffer
 * @param[in] width The width
 * @param[in] height The height
 * @param[in] fsize The file size
 * @param[in/out] position The position in the file
 * @param[/outin] pixbuf A reference to the pixbuf
 * @param[in] stride The stride to use
 * @param[in] palette The palette
 */
bool HandleBitsPerPixel(
  const unsigned int                bitcount,
  const std::uint8_t* const&        inputBufferPtr,
  unsigned int*&                    pix,
  std::uint32_t* const&             outputBufferPtr,
  const unsigned int                width,
  const unsigned int                height,
  const unsigned int                fsize,
  size_t&                           position,
  const unsigned int                stride,
  const Dali::Vector<unsigned int>& palette)
{
  // Pixbuf only ever contains one scanline worth of data.
  Dali::Vector<std::uint8_t> pixbuf;
  pixbuf.ResizeUninitialized(stride);
  std::uint8_t* lineBufferPtr = &pixbuf[0];

  // Note: Switch is in order of most common format first.
  switch(bitcount)
  {
    case 32:
    {
      const std::uint8_t* p = inputBufferPtr + position;
      pix                   = outputBufferPtr + ((height - 1) * width);

      for(unsigned int i = 0; i < height; i++)
      {
        for(unsigned int j = 0; j < width; j++)
        {
          *pix++ = ARGB_JOIN(p[3], p[0], p[1], p[2]);
          p += 4;
        }
        // Move the output up 1 line (we subtract 2 lines because we moved forward one line while copying).
        pix -= (width * 2);
      }
      break;
    }

    case 24:
    {
      for(unsigned int i = 0; i < height; i++)
      {
        pix = outputBufferPtr + ((height - 1 - i) * width);
        if(DALI_UNLIKELY(!read_mem(inputBufferPtr, fsize, &position, lineBufferPtr, stride)))
        {
          return false;
        }
        const std::uint8_t* p = lineBufferPtr;
        for(unsigned int j = 0; j < width; j++)
        {
          *pix++ = ARGB_JOIN(0xff, p[0], p[1], p[2]);
          p += 3;
        }
      }
      break;
    }

    case 8:
    {
      for(unsigned int i = 0; i < height; i++)
      {
        pix = outputBufferPtr + ((height - 1 - i) * width);
        if(DALI_UNLIKELY(!read_mem(inputBufferPtr, fsize, &position, lineBufferPtr, stride)))
        {
          return false;
        }
        const std::uint8_t* p = lineBufferPtr;
        for(unsigned int j = 0; j < width; j++)
        {
          *pix++ = palette[*p++];
        }
      }
      break;
    }

    case 4:
    {
      for(unsigned int i = 0; i < height; i++)
      {
        pix = outputBufferPtr + ((height - 1 - i) * width);
        if(DALI_UNLIKELY(!read_mem(inputBufferPtr, fsize, &position, lineBufferPtr, stride)))
        {
          return false;
        }
        const std::uint8_t* p = lineBufferPtr;
        for(unsigned int j = 0; j < width; j++)
        {
          if(j & 0x1)
          {
            *pix = palette[*p & 0x0f];
            p++;
          }
          else
          {
            *pix = palette[*p >> 4];
          }
          pix++;
        }
      }
      break;
    }

    case 1:
    {
      const std::uint32_t bytesPerWidth          = width / 8;
      const std::uint32_t bytesRemainingPerWidth = width & 7;
      for(unsigned int i = 0; i < height; i++)
      {
        pix = outputBufferPtr + ((height - 1 - i) * width);
        if(DALI_UNLIKELY(!read_mem(inputBufferPtr, fsize, &position, lineBufferPtr, stride)))
        {
          return false;
        }

        const std::uint8_t* p = lineBufferPtr;
        for(unsigned int j = 0; j < bytesPerWidth; ++j)
        {
          *pix++ = palette[*p >> 7];
          *pix++ = palette[*p >> 6 & 0x01];
          *pix++ = palette[*p >> 5 & 0x01];
          *pix++ = palette[*p >> 4 & 0x01];
          *pix++ = palette[*p >> 3 & 0x01];
          *pix++ = palette[*p >> 2 & 0x01];
          *pix++ = palette[*p >> 1 & 0x01];
          *pix++ = palette[*p >> 0 & 0x01];

          ++p;
        }
        if(bytesRemainingPerWidth > 0)
        {
          for(std::uint32_t j = 0; j < bytesRemainingPerWidth; ++j)
          {
            *pix++ = palette[(*p >> (7 - j)) & 0x01];
          }
          ++p;
        }
      }
      break;
    }

    default:
    {
      DALI_LOG_ERROR("Image file contains unsupported bits-per-pixel %d\n", bitcount);
      return false;
    }
  }

  return true;
}

/**
 * @brief Apply the mask if required
 * @param[in] inputBufferPtr The map to use
 * @param[in] fsize The file size
 * @param[in/out] position The position in the file
 * @param[in] bitStride The stride
 * @param[in] width The width
 * @param[in] height The height
 * @param[in/out] pix A reference to the pointer to the pix buffer
 * @param[in/out] outputBufferPtr A reference to the surface buffer
 */
bool ApplyMask(
  const std::uint8_t* const& inputBufferPtr,
  const unsigned int         fsize,
  size_t&                    position,
  const unsigned int         bitStride,
  const unsigned int         width,
  const unsigned int         height,
  unsigned int*&             pix,
  std::uint32_t* const&      outputBufferPtr)
{
  Dali::Vector<std::uint8_t> maskbuf;
  maskbuf.ResizeUninitialized(bitStride);
  std::uint8_t* lineBufferPtr = &maskbuf[0];

  // Apply mask.
  // Precalc to save time in the loops.
  unsigned int bytesPerWidth          = width / 8;
  unsigned int bytesRemainingPerWidth = width - (bytesPerWidth << 3);

  // Loop for each line of the image.
  for(unsigned int i = 0; i < height; ++i)
  {
    pix = outputBufferPtr + ((height - 1 - i) * width);
    if(DALI_UNLIKELY(!read_mem(inputBufferPtr, fsize, &position, lineBufferPtr, bitStride)))
    {
      return false;
    }
    const std::uint8_t* m = lineBufferPtr;

    // Do chunks of 8 pixels first so mask operations can be unrolled.
    for(unsigned int j = 0; j < bytesPerWidth; ++j)
    {
      // Unrolled 8 bits of the mask to avoid many conditions and branches.
      A_VAL(pix++) = (*m & (1 << 7)) ? 0x00 : 0xff;
      A_VAL(pix++) = (*m & (1 << 6)) ? 0x00 : 0xff;
      A_VAL(pix++) = (*m & (1 << 5)) ? 0x00 : 0xff;
      A_VAL(pix++) = (*m & (1 << 4)) ? 0x00 : 0xff;
      A_VAL(pix++) = (*m & (1 << 3)) ? 0x00 : 0xff;
      A_VAL(pix++) = (*m & (1 << 2)) ? 0x00 : 0xff;
      A_VAL(pix++) = (*m & (1 << 1)) ? 0x00 : 0xff;
      A_VAL(pix++) = (*m & (1 << 0)) ? 0x00 : 0xff;
      m++;
    }

    // Handle any remaining width ( < 8 ) or images that are < 8 wide.
    if(bytesRemainingPerWidth > 0)
    {
      for(unsigned int j = 0; j < bytesRemainingPerWidth; ++j)
      {
        // Note: Although we are doing less that a bytes worth of mask, we still always start on the first bit.
        // If the image is smaller than 8 pixels wide, each mask will still start on a new byte.
        A_VAL(pix++) = (*m & (1 << (7 - j))) ? 0x00 : 0xff;
      }
      m++;
    }
  }

  return true;
}

} //unnamed namespace

bool LoadIcoHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  IcoData                     chosen;
  Dali::Vector<unsigned char> map;
  unsigned int                fsize;
  FILE* const                 fp = input.file;

  if(DALI_UNLIKELY(false == LoadIcoHeaderHelper(fp, chosen, map, fsize)))
  {
    return false;
  }

  width  = chosen.w;
  height = chosen.h;

  return true;
}

bool LoadBitmapFromIco(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap)
{
  IcoData                     chosen;
  Dali::Vector<unsigned char> map;
  unsigned int                fsize;
  FILE* const                 fp = input.file;

  if(DALI_UNLIKELY(false == LoadIcoHeaderHelper(fp, chosen, map, fsize)))
  {
    return false;
  }

  unsigned int   dword;
  unsigned short word;

  int           diff_size = 0;
  unsigned int* pix;

  size_t position = chosen.bmoffset; //22 == position

  unsigned int w    = chosen.w;
  unsigned int h    = chosen.h;
  unsigned int cols = chosen.cols;

  const std::uint8_t* const inputBufferPtr = &map[0];

  // read bmp header time... let's do some checking
  if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &dword)))
  {
    return false; // headersize - dont care
  }
  if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &dword)))
  {
    return false; // width
  }
  if(dword > 0)
  {
    if(dword != w)
    {
      w         = dword;
      diff_size = 1;
    }
  }
  if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &dword)))
  {
    return false; // height
  }
  if(dword > 0)
  {
    if(dword != (h * 2))
    {
      h         = dword / 2;
      diff_size = 1;
    }
  }
  if(diff_size)
  {
    DALI_LOG_ERROR("Broken ICO file!\n");
  }

  if(DALI_UNLIKELY(!read_ushort(inputBufferPtr, fsize, &position, &word)))
  {
    return false; // planes
  }
  //planes2 = word;
  if(DALI_UNLIKELY(!read_ushort(inputBufferPtr, fsize, &position, &word)))
  {
    return false; // bitcount
  }
  unsigned int bitcount = word;
  if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &dword)))
  {
    return false; // compression
  }
  //compression = dword;
  if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &dword)))
  {
    return false; // imagesize
  }
  //imagesize = dword;
  if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &dword)))
  {
    return false; // z pixels per m
  }
  if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &dword)))
  {
    return false; // y pizels per m
  }
  if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &dword)))
  {
    return false; // colors used
  }
  //colorsused = dword;
  if(DALI_UNLIKELY(!read_uint(inputBufferPtr, fsize, &position, &dword)))
  {
    return false; // colors important
  }

  Dali::Vector<unsigned int> pal;
  pal.Resize(256 * 4);
  for(unsigned int i = 0; i < cols; i++)
  {
    unsigned char a, r, g, b;

    if(DALI_UNLIKELY(!read_uchar(inputBufferPtr, fsize, &position, &b)))
    {
      return false;
    }
    if(DALI_UNLIKELY(!read_uchar(inputBufferPtr, fsize, &position, &g)))
    {
      return false;
    }
    if(DALI_UNLIKELY(!read_uchar(inputBufferPtr, fsize, &position, &r)))
    {
      return false;
    }
    if(DALI_UNLIKELY(!read_uchar(inputBufferPtr, fsize, &position, &a)))
    {
      return false;
    }
    pal[i] = ARGB_JOIN(0xff, b, g, r);
  }

  Dali::Vector<std::uint32_t> surface;

  // This is the reference way of calculating the total number of bytes necessary to store one row of pixels.
  unsigned int stride    = (((bitcount * w) + 31) / 32) * 4;
  unsigned int bitStride = ((w + 31) / 32) * 4;
  // Set up the surface as soon as we have the width and height.
  surface.ResizeUninitialized(w * h);

  std::uint32_t* const outputBufferPtr = &surface[0];

  // Handle different bits-per-pixel.
  if(DALI_UNLIKELY(!HandleBitsPerPixel(bitcount, inputBufferPtr, pix, outputBufferPtr, w, h, fsize, position, stride, pal)))
  {
    return false;
  }

  // From the spec: If bpp is less than 32, there will be a 1bpp mask bitmap also.
  if(bitcount < 32)
  {
    if(DALI_UNLIKELY(!ApplyMask(inputBufferPtr, fsize, position, bitStride, w, h, pix, outputBufferPtr)))
    {
      // Return false if not able to apply mask when the bpp is less than 32
      return false;
    }
  }

  bitmap      = Dali::Devel::PixelBuffer::New(w, h, Pixel::Format::RGBA8888);
  auto pixels = bitmap.GetBuffer();
  memcpy(pixels, outputBufferPtr, w * h * 4);

  return true;
}

} // namespace TizenPlatform

} // namespace Dali
