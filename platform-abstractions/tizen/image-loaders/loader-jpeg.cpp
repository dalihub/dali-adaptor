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

#include "loader-jpeg.h"

#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>

#include <dali/integration-api/debug.h>
#include <dali/integration-api/bitmap.h>
#include <dali/public-api/math/math-utils.h>
#include <dali/public-api/math/vector2.h>

#include "platform-capabilities.h"
#include <cstring>
#include <libexif/exif-data.h>
#include <libexif/exif-tag.h>

namespace Dali
{
using Integration::Bitmap;

namespace TizenPlatform
{

namespace
{

typedef enum {
  JPGFORM_NONE = 1,  /* no transformation 0th-Row = top & 0th-Column = left */
  JPGFORM_FLIP_H ,   /* horizontal flip       0th-Row = top & 0th-Column = right */
  JPGFORM_FLIP_V,    /* vertical flip     0th-Row = bottom & 0th-Column = right*/
  JPGFORM_TRANSPOSE, /* transpose across UL-to-LR axis   0th-Row = bottom & 0th-Column = left*/
  JPGFORM_TRANSVERSE,/* transpose across UR-to-LL axis   0th-Row = left      & 0th-Column = top*/
  JPGFORM_ROT_90 ,   /* 90-degree clockwise rotation  0th-Row = right  & 0th-Column = top*/
  JPGFORM_ROT_180,   /* 180-degree rotation  0th-Row = right  & 0th-Column = bottom*/
  JPGFORM_ROT_270    /* 270-degree clockwise (or 90 ccw)  0th-Row = left  & 0th-Column = bottom*/
} JPGFORM_CODE;


typedef struct {
   char R;
   char G;
   char B;
}RGB888Type;
typedef struct {
   char R;
   char G;
   char B;
   char A;
}RGBA8888Type;
typedef struct
{
  char RG;
  char GB;
}RGB565Type;
typedef struct
{
  char gray;
}L8Type;

// simple class to enforce clean-up of JPEG structures
struct auto_jpg
{
  auto_jpg(struct jpeg_decompress_struct& _cinfo)
  : cinfo(_cinfo),
    compression_started(false)
  {
  }

  ~auto_jpg()
  {
    // clean up JPG resources
    if( compression_started )
    {
      // finish decompression if started
      jpeg_finish_decompress(&cinfo);
    }

    jpeg_destroy_decompress (&cinfo);
  }

  bool start_decompress()
  {
    // store flag indicating compression has started
    compression_started = jpeg_start_decompress(&cinfo);
    return compression_started;
  }

  void abort_decompress()
  {
    if( compression_started )
    {
      // abort decompression if started
      jpeg_abort_decompress(&cinfo);
      // clear compression started flag
      compression_started = false;
    }
  }

  struct jpeg_decompress_struct& cinfo;
  bool compression_started;
}; // struct auto_jpg;

static void JpegFatalErrorHandler (j_common_ptr cinfo)
{
  /* LibJpeg causes an assert if this happens but we do not want that */
}

static void JpegOutputMessageHandler (j_common_ptr cinfo)
{
  /* Stop libjpeg from printing to stderr - Do Nothing */
}

bool LoadJpegHeader(FILE *fp, unsigned int &width, unsigned int &height, jpeg_decompress_struct &cinfo, jpeg_error_mgr &jerr)
{
    cinfo.err = jpeg_std_error(&jerr);

    jerr.output_message = JpegOutputMessageHandler;
    jerr.error_exit = JpegFatalErrorHandler;

    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, fp);

    // Check header to see if it is  JPEG file
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
    {
      return false;
    }

    width = (float)cinfo.image_width;
    height = (float)cinfo.image_height;

    return true;
}

// Simple struct to ensure xif data is deleted
struct ExifAutoPtr
{
  ExifAutoPtr( ExifData* data)
  :mData( data )
  {}

  ~ExifAutoPtr()
  {
    exif_data_free( mData);
  }
  ExifData *mData;
};


} // unnamed namespace

bool JpegRotate90(unsigned char *buffer, int width, int height, int bpp);
bool JpegRotate180(unsigned char *buffer, int width, int height, int bpp);
bool JpegRotate270(unsigned char *buffer, int width, int height, int bpp);
bool LoadJpegHeader(FILE *fp, unsigned int &width, unsigned int &height)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  auto_jpg autoJpg(cinfo);

  return LoadJpegHeader(fp, width, height, cinfo, jerr);
}

bool LoadBitmapFromJpeg(FILE *fp, Bitmap& bitmap, ImageAttributes& attributes)
{
  JPGFORM_CODE transform = JPGFORM_NONE;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  auto_jpg autoJpg(cinfo);

  if( fseek(fp,0,SEEK_END) )
  {
    DALI_LOG_ERROR("Error seeking to end of file\n");
    return false;
  }

  long positionIndicator = ftell(fp);
  unsigned int jpegbufsize( 0u );
  if( positionIndicator > -1L )
  {
    jpegbufsize = static_cast<unsigned int>(positionIndicator);
  }

  if( 0u == jpegbufsize )
  {
    return false;
  }

  if( fseek(fp, 0, SEEK_SET) )
  {
    DALI_LOG_ERROR("Error seeking to end of file\n");
    return false;
  }

  std::vector<unsigned char> jpegbuf(jpegbufsize);
  unsigned char *jpegbufPtr = &jpegbuf[0];
  if(fread(jpegbufPtr, 1, jpegbufsize, fp) != jpegbufsize)
  {
    return false;
  }

  if( fseek(fp, 0, SEEK_SET) )
  {
    DALI_LOG_ERROR("Error seeking to end of file\n");
    return false;
  }

  ExifAutoPtr exifData( exif_data_new_from_data(jpegbufPtr, jpegbufsize) );
  ExifEntry *entry;
  ExifTag tag = EXIF_TAG_ORIENTATION;
  entry = exif_data_get_entry(exifData.mData, tag);
  int orientation = 0;
  if(entry)
  {
    orientation = exif_get_short(entry->data, exif_data_get_byte_order(entry->parent->parent));
    switch(orientation)
    {
    case 1:
      transform = JPGFORM_NONE;
      break;
    case 2:
      transform = JPGFORM_FLIP_H;
      break;
    case 3:
      transform = JPGFORM_FLIP_V;
      break;
    case 4:
      transform = JPGFORM_TRANSPOSE;
      break;
    case 5:
      transform = JPGFORM_TRANSVERSE;
      break;
    case 6:
      transform = JPGFORM_ROT_90;
      break;
    case 7:
      transform = JPGFORM_ROT_180;
      break;
    case 8:
      transform = JPGFORM_ROT_270;
      break;
    default:
      break;
    }
  }

  // Load the header info
  unsigned int width, height;
  if (!LoadJpegHeader(fp, width, height, cinfo, jerr))
  {
      return false;
  }

  // set scaling if image request is not zero (which means full image)
  // and requested size is smaller than actual size
  if( ( attributes.GetWidth() > 0 )&&( attributes.GetHeight() > 0 ) &&
      ( attributes.GetWidth()  < cinfo.image_width )           &&
      ( attributes.GetHeight() < cinfo.image_height ))
  {
    // jpeg only supports scaling by 1/2, 1/4, 1/8, 1/16
    // calculate width and height scale between (fitted) request and original
    Size req = attributes.GetSize();
    Size orig((float)cinfo.image_width, (float)cinfo.image_height);
    Size fitted = FitInside(req, orig);
    int widthScale = cinfo.image_width / fitted.width;
    int heightScale = cinfo.image_height / fitted.height;
    // pick the smallest one of those as we want to scale as close as possible
    int scale = std::min( widthScale, heightScale );
    // if the scale is not power of two make it
    if( !IsPowerOfTwo( scale ) )
    {
      scale = NextPowerOfTwo( scale );
    }
    // finally set the scale 1 / scale
    cinfo.scale_num = 1;
    cinfo.scale_denom = scale;
  }

  Pixel::Format pixelFormat = Pixel::RGBA8888;
  if (!autoJpg.start_decompress())
  {
   // @todo renable this log LOG_WARNING("error during jpeg_start_decompress\n");
    return false;
  }

  int bytes_per_pixel   = cinfo.out_color_components;

  // JPEG doesn't support transparency
  if (bytes_per_pixel == 3)
  {
    pixelFormat = Pixel::RGB888;
  }
  else if (bytes_per_pixel == 2)
  {
    pixelFormat = Pixel::RGB565;
  }
  else if (bytes_per_pixel == 1)
  {
    pixelFormat = Pixel::L8;
  }
  else
  {
    DALI_LOG_WARNING ("Unsupported jpeg format\n");
    autoJpg.abort_decompress();
    return false;
  }

  width                      = cinfo.output_width;
  height                     = cinfo.output_height;
  unsigned int  bufferWidth  = GetTextureDimension( width );
  unsigned int  bufferHeight = GetTextureDimension( height );
  unsigned int  stride       = bufferWidth * bytes_per_pixel;

  unsigned char *pixels      = NULL;
  unsigned char *lptr        = NULL;

  /// @todo support more scaling types
  bool fitInside = attributes.GetScalingMode() == Dali::ImageAttributes::ShrinkToFit &&
                   attributes.GetWidth() != 0 &&
                   attributes.GetHeight() != 0 &&
                  (attributes.GetWidth() < cinfo.output_width || attributes.GetHeight() < cinfo.output_height);
  bool crop = attributes.GetScalingMode() == Dali::ImageAttributes::ScaleToFill;

  if (fitInside || crop)
  {
    // create temporary decompression buffer
    pixels = new unsigned char[stride*bufferHeight];
    lptr   = pixels;
  }
  else
  {
    // decode the whole image into bitmap buffer
    switch(transform)
    {
    case JPGFORM_NONE:
    case JPGFORM_FLIP_H:
    case JPGFORM_FLIP_V:
    case JPGFORM_TRANSPOSE:
    case JPGFORM_TRANSVERSE:
    case JPGFORM_ROT_180:
      pixels = bitmap.ReserveBuffer(pixelFormat, width, height, bufferWidth, bufferHeight);
      break;
    case JPGFORM_ROT_270:
    case JPGFORM_ROT_90:
      pixels = bitmap.ReserveBuffer(pixelFormat, height,width, bufferHeight,bufferWidth);
    default:
      break;
    }
    lptr   = pixels;
  }

  // decode jpeg
  while (cinfo.output_scanline < cinfo.output_height)
  {
    lptr = pixels + (stride * cinfo.output_scanline);
    jpeg_read_scanlines(&cinfo, &lptr, 1);
  }
  DALI_ASSERT_DEBUG(pixels);

  switch(transform)
  {
  case JPGFORM_NONE:
  case JPGFORM_FLIP_H:
  case JPGFORM_FLIP_V:
  case JPGFORM_TRANSPOSE:
  case JPGFORM_TRANSVERSE:
    break;
  case JPGFORM_ROT_180:
    JpegRotate180(pixels, bufferWidth, bufferHeight, bytes_per_pixel);
    break;
  case JPGFORM_ROT_270:
    JpegRotate270(pixels, bufferWidth, bufferHeight, bytes_per_pixel);
    break;
  case JPGFORM_ROT_90:
    JpegRotate90(pixels, bufferWidth, bufferHeight, bytes_per_pixel);
    break;
  default:
    break;
  }
  // fit image completely inside requested size (maintaining aspect ratio)?
  if (fitInside)
  {
    Size req = attributes.GetSize();
    Size orig((float)cinfo.output_width, (float)cinfo.output_height);
    switch(transform)
    {
    case JPGFORM_NONE:
    case JPGFORM_FLIP_H:
    case JPGFORM_FLIP_V:
    case JPGFORM_TRANSPOSE:
    case JPGFORM_TRANSVERSE:
    case JPGFORM_ROT_180:
      break;
    case JPGFORM_ROT_270:
    case JPGFORM_ROT_90:
      orig = Vector2 ((float)cinfo.output_height, (float)cinfo.output_width);
      stride = bufferHeight * bytes_per_pixel;
      break;
    default:
      break;
    }
    // calculate actual width, height
    req = FitInside(req, orig);

    attributes.SetSize((int) req.width, (int) req.height);
    attributes.SetPixelFormat(pixelFormat);

    bufferWidth  = GetTextureDimension( attributes.GetWidth() );
    bufferHeight = GetTextureDimension( attributes.GetHeight() );

    // scaled buffer's stride
    int lstride = bufferWidth*bytes_per_pixel;

    // allocate bitmap buffer using requested size
    unsigned char *bitmapBuffer = bitmap.ReserveBuffer(pixelFormat, attributes.GetWidth(), attributes.GetHeight(), bufferWidth, bufferHeight);

    // scale original image to fit requested size
    float xRatio = (float)cinfo.output_width / (float)attributes.GetWidth();
    float yRatio = (float)cinfo.output_height / (float)attributes.GetHeight();
    switch(transform)
    {
    case JPGFORM_NONE:
    case JPGFORM_FLIP_H:
    case JPGFORM_FLIP_V:
    case JPGFORM_TRANSPOSE:
    case JPGFORM_TRANSVERSE:
    case JPGFORM_ROT_180:
      break;
    case JPGFORM_ROT_270:
    case JPGFORM_ROT_90:
      xRatio = (float)cinfo.output_height / (float)attributes.GetWidth();
      yRatio = (float)cinfo.output_width / (float)attributes.GetHeight();
      break;
    default:
      break;
    }
    // @todo Add a quality setting to ImageAttributes and use suitable shrink algorithm
    unsigned char *bufptr = bitmapBuffer;
    for (unsigned int y = 0; y < attributes.GetHeight(); ++y)
    {
      unsigned char* dest = bufptr;
      lptr = pixels + (stride * (unsigned int)(floorf(yRatio * y)));
      for (unsigned int x = 0; x < attributes.GetWidth(); ++x)
      {
        unsigned char* src = lptr + (unsigned int)(floorf(xRatio * x)) * bytes_per_pixel;
        *dest++ = *src++;
        if (bytes_per_pixel >= 2)
        {
          *dest++ = *src++;
          if (bytes_per_pixel >= 3)
          {
            *dest++ = *src++;
          }
        }
      }
      bufptr += lstride;
    }

    delete[] pixels;
  }
  // copy part of the buffer to bitmap?
  else if (crop)
  {
    Size req = attributes.GetSize();
    const Size orig ((float)cinfo.output_width, (float)cinfo.output_height);

    // calculate actual width, height
    req = FitScaleToFill(req, orig);

    // modify attributes with result
    attributes.SetSize((int) req.width, (int) req.height);
    attributes.SetPixelFormat(pixelFormat);

    bufferWidth  = GetTextureDimension( attributes.GetWidth() );
    bufferHeight = GetTextureDimension( attributes.GetHeight() );

    // cropped buffer's stride
    int lstride = bufferWidth*bytes_per_pixel;

    // calculate offsets
    int x_offset = ((cinfo.output_width  - attributes.GetWidth())  / 2) * bytes_per_pixel;
    int y_offset = ((cinfo.output_height - attributes.GetHeight()) / 2) * stride;

    // allocate bitmap buffer using requested size
    unsigned char *bitmapBuffer = bitmap.ReserveBuffer(pixelFormat, attributes.GetWidth(), attributes.GetHeight(), bufferWidth, bufferHeight);

    // crop center of original image to fit requested size
    unsigned char *bufptr = bitmapBuffer;
    lptr = pixels+y_offset+x_offset;
    for (unsigned int i = 0; i < attributes.GetHeight(); ++i)
    {
      memcpy (bufptr, lptr, attributes.GetWidth()*bytes_per_pixel);
      bufptr += lstride;
      lptr += stride;
    }

    delete[] pixels;
  }
  else
  {
    // set the attributes
    switch(transform)
    {
    case JPGFORM_NONE:
    case JPGFORM_FLIP_H:
    case JPGFORM_FLIP_V:
    case JPGFORM_TRANSPOSE:
    case JPGFORM_TRANSVERSE:
    case JPGFORM_ROT_180:
      attributes.SetSize(width, height);
      break;
    case JPGFORM_ROT_270:
    case JPGFORM_ROT_90:
      attributes.SetSize(height, width);
      break;
    default:
      break;
    }

    attributes.SetPixelFormat(pixelFormat);
  }

  return true;
}

bool JpegRotate180(unsigned char *buffer, int width, int height, int bpp)
{
  int  ix, iw, ih, hw = 0;
  iw = width;
  ih = height;
  hw = iw * ih;
  ix = hw;

  switch(bpp)
  {
  case 4:
  {
    RGBA8888Type *from, *to;
    RGBA8888Type tmp;
    from = ((RGBA8888Type * )buffer) + hw - 1;
    to = (RGBA8888Type *)buffer;
    for(; --ix >= (hw / 2); )
    {
      tmp = *to;
      *to = *from;
      *from = tmp;
      to ++;
      from --;
    }
    break;
  }
  case 3:
  {
    RGB888Type *from, *to;
    RGB888Type tmp;
    from = ((RGB888Type * )buffer) + hw - 1;
    to = (RGB888Type *)buffer;
    for(; --ix >= (hw / 2); )
    {
      tmp = *to;
      *to = *from;
      *from = tmp;
      to ++;
      from --;
    }
    break;
  }
  case 2:
  {
    RGB565Type *from, *to;
    RGB565Type tmp;
    from = ((RGB565Type * )buffer) + hw - 1;
    to = (RGB565Type *)buffer;
    for(; --ix >= (hw / 2); )
    {
      tmp = *to;
      *to = *from;
      *from = tmp;
      to ++;
      from --;
    }
    break;
  }
  case 1:
  {
    L8Type *from, *to;
    L8Type tmp;
    from = ((L8Type * )buffer) + hw - 1;
    to = (L8Type *)buffer;
    for(; --ix >= (hw / 2); )
    {
      tmp = *to;
      *to = *from;
      *from = tmp;
      to ++;
      from --;
    }
    break;
  }
  default:
    break;
  }

  return true;
}

bool JpegRotate270(unsigned char *buffer, int width, int height, int bpp)
{
  int  w, iw, ih, hw = 0;
  int ix, iy = 0;

  iw = width;
  ih = height;
  std::vector<unsigned char> data(width * height * bpp);
  unsigned char *dataPtr = &data[0];
  memcpy(dataPtr, buffer, width * height * bpp);
  w = ih;
  ih = iw;
  iw = w;
  hw = iw * ih;

  switch(bpp)
  {
  case 4:
  {
    RGBA8888Type *from, *to;
    to = ((RGBA8888Type * )buffer) + hw  - iw;
    w = -w;
    hw =  hw + 1;
    from = (RGBA8888Type *)dataPtr;
    for(ix = iw; -- ix >= 0;)
    {
      for(iy = ih; -- iy >= 0;)
      {
        *to = *from;
        from += 1;
        to += w;
      }
      to += hw;
    }
    break;
  }
  case 3:
  {
    RGB888Type *from, *to;
    to = ((RGB888Type * )buffer) + hw  - iw;
    w = -w;
    hw =  hw + 1;
    from = (RGB888Type *)dataPtr;
    for(ix = iw; -- ix >= 0;)
    {
      for(iy = ih; -- iy >= 0;)
      {
        *to = *from;
        from += 1;
        to += w;
      }
      to += hw;
    }
    break;
  }
  case 2:
  {
    RGB565Type *from, *to;
    to = ((RGB565Type * )buffer) + hw  - iw;
    w = -w;
    hw =  hw + 1;
    from = (RGB565Type *)dataPtr;
    for(ix = iw; -- ix >= 0;)
    {
      for(iy = ih; -- iy >= 0;)
      {
        *to = *from;
        from += 1;
        to += w;
      }
      to += hw;
    }
    break;
  }
  case 1:
  {
    L8Type *from, *to;
    to = ((L8Type * )buffer) + hw  - iw;
    w = -w;
    hw =  hw + 1;
    from = (L8Type *)dataPtr;
    for(ix = iw; -- ix >= 0;)
    {
      for(iy = ih; -- iy >= 0;)
      {
        *to = *from;
        from += 1;
        to += w;
      }
      to += hw;
    }
    break;
  }
  default:
    break;
  }

  return true;
}

bool JpegRotate90(unsigned char *buffer, int width, int height, int bpp)
{
  int  w, iw, ih, hw = 0;
  int ix, iy = 0;
  iw = width;
  ih = height;
  std::vector<unsigned char> data(width * height * bpp);
  unsigned char *dataPtr = &data[0];
  memcpy(dataPtr, buffer, width * height * bpp);
  w = ih;
  ih = iw;
  iw = w;
  hw = iw * ih;
  hw = - hw - 1;
  switch(bpp)
  {
  case 4:
  {
    RGBA8888Type *from, *to;
    to = ((RGBA8888Type * )buffer) + iw  - 1;
    from = (RGBA8888Type *)dataPtr;
    for(ix = iw; -- ix >= 0;)
    {
      for(iy = ih; -- iy >= 0;)
      {
        *to = *from;
        from += 1;
        to += iw;
      }
      to += hw;
    }
    break;
  }
  case 3:
  {
    RGB888Type *from, *to;
    to = ((RGB888Type * )buffer) + iw  - 1;
    from = (RGB888Type *)dataPtr;
    for(ix = iw; -- ix >= 0;)
    {
      for(iy = ih; -- iy >= 0;)
      {
        *to = *from;
        from += 1;
        to += iw;
      }
      to += hw;
    }
    break;
  }
  case 2:
  {
    RGB565Type *from, *to;
    to = ((RGB565Type * )buffer) + iw  - 1;
    from = (RGB565Type *)dataPtr;
    for(ix = iw; -- ix >= 0;)
    {
      for(iy = ih; -- iy >= 0;)
      {
        *to = *from;
        from += 1;
        to += iw;
      }
      to += hw;
    }
    break;
  }
  case 1:
  {
    L8Type *from, *to;
    to = ((L8Type * )buffer) + iw  - 1;
    from = (L8Type *)dataPtr;
    for(ix = iw; -- ix >= 0;)
    {
      for(iy = ih; -- iy >= 0;)
      {
        *to = *from;
        from += 1;
        to += iw;
      }
      to += hw;
    }
    break;
  }
  default:
    break;
  }

  return true;
}

} // namespace TizenPlatform

} // namespace Dali

