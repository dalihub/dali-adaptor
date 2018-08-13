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
#include <cstring>
#include <dali/public-api/common/dali-vector.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

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

typedef unsigned char  DATA8;
#define A_VAL(p) (reinterpret_cast< DATA8 * >( p )[3])

#define RGB_JOIN(r,g,b) \
                (((r) << 16) + ((g) << 8) + (b))

#define ARGB_JOIN(a,r,g,b) \
                (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

bool read_ushort(unsigned char *map, size_t length, size_t *position, unsigned short *ret)
{
  unsigned char b[2];

  if (*position + 2 > length)
  {
    return false;
  }
  b[0] = map[(*position)++];
  b[1] = map[(*position)++];
  *ret = (b[1] << 8) | b[0];
  return true;
}

bool read_uint(unsigned char *map, size_t length, size_t *position, unsigned int *ret)
{
  unsigned char b[4];
  unsigned int i;

  if (*position + 4 > length)
  {
    return false;
  }
  for (i = 0; i < 4; i++)
  {
    b[i] = map[(*position)++];
  }
  *ret = ARGB_JOIN(b[3], b[2], b[1], b[0]);
  return true;
}

bool read_uchar(unsigned char *map, size_t length, size_t *position, unsigned char *ret)
{
  if (*position + 1 > length)
  {
    return false;
  }
  *ret = map[(*position)++];
  return true;
}

bool read_mem(unsigned char *map, size_t length, size_t *position, void *buffer, int size)
{
  if (*position + size > length)
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
  ICON = 1,
  CURSOR = 2
};

struct IcoData
{
  int pdelta;
  int w, h;
  int cols;
  int bpp, planes;
  int hot_x, hot_y;
  unsigned int bmoffset, bmsize;
};

bool LoadIcoHeaderHelper( FILE* fp,
                          IcoData& chosen,
                          Dali::Vector<unsigned char>& map,
                          unsigned int& fsize )
{
  memset( &chosen, 0, sizeof(chosen) );

  if(fp == NULL)
  {
    DALI_LOG_ERROR("Error loading bitmap\n");
    return false;
  }
  size_t position = 0;
  unsigned short word;
  unsigned char byte;

  if( fseek(fp,0,SEEK_END) )
  {
    DALI_LOG_ERROR("Error seeking ICO data\n");
    return false;
  }

  long positionIndicator = ftell(fp);
  fsize = 0u;

  if( positionIndicator > -1L )
  {
    fsize = static_cast<unsigned int>(positionIndicator);
  }

  if( 0u == fsize )
  {
    return false;
  }

  if( fseek(fp, 0, SEEK_SET) )
  {
    DALI_LOG_ERROR("Error seeking ICO data\n");
    return false;
  }

  if (fsize < (ICO_FILE_HEADER + ICO_IMAGE_INFO_HEADER)) //6 + 16 + 40
  {
    return false;
  }
  map.Resize(fsize);

  if(fread(&map[0], 1, fsize, fp) != fsize)
  {
    DALI_LOG_WARNING("image file read opeation error!\n");
    return false;
  }

  int search = BIGGEST;
  unsigned short reserved, type, count;
  if (!read_ushort(&map[0], fsize, &position, &reserved))
  {
    return false;
  }
  if (!read_ushort(&map[0], fsize, &position, &type))
  {
    return false;
  }
  if (!read_ushort(&map[0], fsize, &position, &count))
  {
    return false;
  }
  if (!((reserved == 0) &&
       ((type == ICON) || (type == CURSOR)) && (count != 0)))
  {
    return false;
  }
  search = BIGGEST;
  chosen.pdelta = 0;
  bool have_choice = false;

  for (unsigned short i = 0; i < count; i++)
  {
    unsigned char tw = 0, th = 0, tcols = 0;
    if (!read_uchar(&map[0], fsize, &position, &tw))
    {
      return false;
    }
    int w = tw;
    if (w <= 0)
    {
      w = 256;
    }
    if (!read_uchar(&map[0], fsize, &position, &th))
    {
      return false;

    }
    int h = th;
    if (h <= 0)
    {
      h = 256;
    }
    if (!read_uchar(&map[0], fsize, &position, &tcols))
    {
      return false;
    }
    int cols = tcols;
    if (!read_uchar(&map[0], fsize, &position, &byte))
    {
      return false;
    }
    if (!read_ushort(&map[0], fsize, &position, &word))
    {
      return false;
    }
    int planes=0;
    if (type == 1)
    {
      planes = word;
    }
    //else hot_x = word;
    if (!read_ushort(&map[0], fsize, &position, &word))
    {
      return false;
    }
    int bpp=0;
    if (type == 1)
    {
      bpp = word;
    }

    // 0 colors means 256 for paletized modes.
    // Note: We must not do this conversion for bpp greater than 8, as there is no palette.
    if( bpp <= 8 && cols == 0 )
    {
      cols = 256;
    }

    //else hot_y = word;
    unsigned int bmoffset, bmsize;
    if (!read_uint(&map[0], fsize, &position, &bmsize))
    {
      return false;
    }
    if (!read_uint(&map[0], fsize, &position, &bmoffset))
    {
      return false;
    }
    if ((bmsize <= 0) || (bmoffset <= 0) || (bmoffset >= fsize))
    {
      return false;
    }
    if (search == BIGGEST)
    {
      int pdelta = w * h;
      if ((!have_choice) ||
       ((pdelta >= chosen.pdelta) &&
           (((bpp >= 3) && (bpp >= chosen.bpp)) ||
               ((bpp < 3) && (cols >= chosen.cols)))))
      {
        have_choice = true;
        chosen.pdelta = pdelta;
        chosen.w = w;
        chosen.h = h;
        chosen.cols = cols;
        chosen.bpp = bpp;
        chosen.planes = planes;
        chosen.bmsize = bmsize;
        chosen.bmoffset = bmoffset;
      }
    }
  }

  if (chosen.bmoffset == 0)
  {
    return false;
  }

  return true;
}

}//unnamed namespace

bool LoadIcoHeader( const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height )
{
  IcoData chosen;
  Dali::Vector<unsigned char> map;
  unsigned int fsize;
  FILE* const fp = input.file;

  if ( false == LoadIcoHeaderHelper(fp, chosen, map, fsize) )
  {
    return false;
  }

  width = chosen.w;
  height = chosen.h;

  return true;
}

bool LoadBitmapFromIco( const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap )
{
  IcoData chosen;
  Dali::Vector<unsigned char> map;
  unsigned int fsize;
  FILE* const fp = input.file;

  if ( false == LoadIcoHeaderHelper(fp, chosen, map, fsize) )
  {
    return false;
  }

  Dali::Vector<unsigned int> pal;
  Dali::Vector<unsigned int> surface;
  Dali::Vector<unsigned char> maskbuf;
  Dali::Vector<unsigned char> pixbuf;
  pal.Resize(256 * 4);

  unsigned int dword;
  unsigned short word;

  int diff_size = 0;
  unsigned int* pix;

  size_t position = chosen.bmoffset;//22 == position

  unsigned int w = chosen.w;
  unsigned int h = chosen.h;
  unsigned int cols = chosen.cols;

  // read bmp header time... let's do some checking
  if (!read_uint(&map[0], fsize, &position, &dword))
  {
    return false; // headersize - dont care
  }
  if (!read_uint(&map[0], fsize, &position, &dword))
  {
    return false; // width
  }
  if (dword > 0)
  {
    if (dword != w)
    {
      w = dword;
      diff_size = 1;
    }
  }
  if (!read_uint(&map[0], fsize, &position, &dword))
  {
    return false; // height
  }
  if (dword > 0)
  {
    if (dword != (h * 2))
    {
      h = dword / 2;
      diff_size = 1;
    }
  }
  if (diff_size)
  {
    DALI_LOG_WARNING("Broken ICO file!\n");
  }

  // Set up the surface as soon as we have the width and height, so we have a black image if there are any further errors.
  surface.Resize( w * h * 4 );
  memset( &surface[0], 0, w * h * 4 );

  if (!read_ushort(&map[0], fsize, &position, &word))
  {
    return false; // planes
  }
  //planes2 = word;
  if (!read_ushort(&map[0], fsize, &position, &word))
  {
    return false; // bitcount
  }
  unsigned int bitcount = word;
  if (!read_uint(&map[0], fsize, &position, &dword))
  {
    return false; // compression
  }
  //compression = dword;
  if (!read_uint(&map[0], fsize, &position, &dword))
  {
    return false; // imagesize
  }
  //imagesize = dword;
  if (!read_uint(&map[0], fsize, &position, &dword))
  {
    return false; // z pixels per m
  }
  if (!read_uint(&map[0], fsize, &position, &dword))
  {
    return false; // y pizels per m
  }
  if (!read_uint(&map[0], fsize, &position, &dword))
  {
    return false; // colors used
  }
  //colorsused = dword;
  if (!read_uint(&map[0], fsize, &position, &dword))
  {
    return false; // colors important
  }

  for( unsigned int i = 0; i < cols ; i ++ )
  {
    unsigned char a, r, g, b;

    if (!read_uchar(&map[0], fsize, &position, &b))
    {
      return false;
    }
    if (!read_uchar(&map[0], fsize, &position, &g))
    {
      return false;
    }
    if (!read_uchar(&map[0], fsize, &position, &r))
    {
      return false;
    }
    if (!read_uchar(&map[0], fsize, &position, &a))
    {
      return false;
    }
    pal[i] = ARGB_JOIN( 0xff, b, g, r );
  }

  // This is the reference way of calculating the total number of bytes necessary to store one row of pixels.
  unsigned int stride = ( ( ( bitcount * w ) + 31 ) / 32 ) * 4;
  unsigned int bitStride = ( ( w + 31 ) / 32 ) * 4;

  // Pixbuf only ever contains one scanline worth of data.
  pixbuf.Resize( stride );
  maskbuf.Resize( bitStride * h );

  // Handle different bits-per-pixel.
  // Note: Switch is in order of most common format first.
  switch( bitcount )
  {
    case 32:
    {
      unsigned char* p = &map[position];
      pix = &surface[0] + ( ( h - 1 ) * w );

      for( unsigned int i = 0; i < h; i++ )
      {
        for( unsigned int j = 0; j < w; j++ )
        {
          *pix++ = ARGB_JOIN( p[3], p[0], p[1], p[2] );
          p += 4;
        }
        // Move the output up 1 line (we subtract 2 lines because we moved forward one line while copying).
        pix -= ( w * 2 );
      }
      break;
    }

    case 24:
    {
      for( unsigned int i = 0; i < h; i++ )
      {
        pix = &surface[0] + ( ( h - 1 - i ) * w );
        if( !read_mem( &map[0], fsize, &position, &pixbuf[0], stride ) )
        {
          return false;
        }
        unsigned char* p = &pixbuf[0];
        for( unsigned int j = 0; j < w; j++ )
        {
          *pix++ = ARGB_JOIN( 0xff, p[0], p[1], p[2] );
          p += 3;
        }
      }
      break;
    }

    case 8:
    {
      for( unsigned int i = 0; i < h; i++ )
      {
        pix = &surface[0] + ( ( h - 1 - i ) * w );
        if( !read_mem( &map[0], fsize, &position, &pixbuf[0], stride ) )
        {
          return false;
        }
        unsigned char* p = &pixbuf[0];
        for( unsigned int j = 0; j < w; j++ )
        {
          *pix++ = pal[*p++];
        }
      }
      break;
    }

    case 4:
    {
      for( unsigned int i = 0; i < h; i++ )
      {
        pix = &surface[0] + ( ( h - 1 - i ) * w );
        if( !read_mem( &map[0], fsize, &position, &pixbuf[0], stride ) )
        {
          return false;
        }
        unsigned char* p = &pixbuf[0];
        for( unsigned int j = 0; j < w; j++ )
        {
          if( j & 0x1 )
          {
            *pix = pal[*p & 0x0f];
            p++;
          }
          else
          {
            *pix = pal[*p >> 4];
          }
          pix++;
        }
      }
      break;
    }

    case 1:
    {
      for( unsigned int i = 0; i < h; i++ )
      {
        pix = &surface[0] + ( ( h - 1 - i ) * w );
        if( !read_mem( &map[0], fsize, &position, &pixbuf[0], stride ) )
        {
          return false;
        }
        unsigned char* p = &pixbuf[0];

        for( unsigned int j = 0; j < w; j += 8 )
        {
          *pix++ = pal[ *p >> 7 ];
          *pix++ = pal[ *p >> 6 & 0x01 ];
          *pix++ = pal[ *p >> 5 & 0x01 ];
          *pix++ = pal[ *p >> 4 & 0x01 ];
          *pix++ = pal[ *p >> 3 & 0x01 ];
          *pix++ = pal[ *p >> 2 & 0x01 ];
          *pix++ = pal[ *p >> 1 & 0x01 ];
          *pix++ = pal[ *p >> 0 & 0x01 ];

          p++;
        }
      }
      break;
    }

    default:
    {
      DALI_LOG_WARNING( "Image file contains unsupported bits-per-pixel %d\n", bitcount );
      return false;
    }
  }

  // From the spec: If bpp is less than 32, there will be a 1bpp mask bitmap also.
  if( bitcount < 32 )
  {
    if( !read_mem( &map[0], fsize, &position, &maskbuf[0], bitStride * h ) )
    {
      return false;
    }

    // Apply mask.
    // Precalc to save time in the loops.
    unsigned int bytesPerWidth = w / 8;
    unsigned int bytesRemainingPerWidth = w - ( bytesPerWidth << 3 );

    // Loop for each line of the image.
    for( unsigned int i = 0; i < h; ++i )
    {
      unsigned char *m = &maskbuf[0] + ( bitStride * i );
      pix = &surface[0] + ( ( h - 1 - i ) * w );

      // Do chunks of 8 pixels first so mask operations can be unrolled.
      for( unsigned int j = 0; j < bytesPerWidth; ++j )
      {
        // Unrolled 8 bits of the mask to avoid many conditions and branches.
        A_VAL( pix++ ) = ( *m & ( 1 << 7 ) ) ? 0x00 : 0xff;
        A_VAL( pix++ ) = ( *m & ( 1 << 6 ) ) ? 0x00 : 0xff;
        A_VAL( pix++ ) = ( *m & ( 1 << 5 ) ) ? 0x00 : 0xff;
        A_VAL( pix++ ) = ( *m & ( 1 << 4 ) ) ? 0x00 : 0xff;
        A_VAL( pix++ ) = ( *m & ( 1 << 3 ) ) ? 0x00 : 0xff;
        A_VAL( pix++ ) = ( *m & ( 1 << 2 ) ) ? 0x00 : 0xff;
        A_VAL( pix++ ) = ( *m & ( 1 << 1 ) ) ? 0x00 : 0xff;
        A_VAL( pix++ ) = ( *m & ( 1 << 0 ) ) ? 0x00 : 0xff;
        m++;
      }

      // Handle any remaining width ( < 8 ) or images that are < 8 wide.
      if( bytesRemainingPerWidth > 0 )
      {
        for( unsigned int j = 0; j < bytesRemainingPerWidth; ++j )
        {
          // Note: Although we are doing less that a bytes worth of mask, we still always start on the first bit.
          // If the image is smaller than 8 pixels wide, each mask will still start on a new byte.
          A_VAL( pix++ ) = ( *m & ( 1 << ( 7 - j ) ) ) ? 0x00 : 0xff;
        }
        m++;
      }
    }
  }

  bitmap = Dali::Devel::PixelBuffer::New(w, h, Pixel::Format::RGBA8888);
  auto pixels = bitmap.GetBuffer();
  memcpy( pixels, &surface[0], w * h * 4 );

  return true;
}

}

}
