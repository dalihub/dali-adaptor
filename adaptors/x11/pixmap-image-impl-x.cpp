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

// CLASS HEADER
#include "pixmap-image-impl.h"

// EXTERNAL INCLUDES
#include <Ecore_X.h>
#include <X11/Xutil.h>
#include <dali/integration-api/debug.h>
#include <render-surface.h>

// INTERNAL INCLUDES
#include <gl/egl-image-extensions.h>
#include <gl/egl-factory.h>
#include <adaptor-impl.h>

// Allow this to be encoded and saved:
#include <platform-abstractions/slp/resource-loader/resource-loader.h>
#include <platform-abstractions/slp/resource-loader/loader-jpeg.h>
#include <platform-abstractions/slp/resource-loader/loader-png.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
using Dali::Integration::PixelBuffer;

// Pieces needed to save compressed images (temporary location while plumbing):
namespace
{

  /**
   * Free an allocated XImage on destruction.
   */
  struct XImageJanitor
  {
    XImageJanitor( XImage* const  pXImage ) : mXImage( pXImage )
    {
      DALI_ASSERT_DEBUG(pXImage != 0 && "Null pointer to XImage.");
    }

    ~XImageJanitor()
    {
      if( mXImage )
      {
        if( !XDestroyImage(mXImage) )
        {
          DALI_ASSERT_DEBUG("XImage deallocation failure");
        }
      }
    }
    XImage* const  mXImage;
  private:
    XImageJanitor( const XImageJanitor& rhs );
    XImageJanitor& operator = ( const XImageJanitor& rhs );
  };

  /**
   * Simple function to tell intended image file format from filename
   */
  FileFormat GetFormatFromFileName( const std::string& filename )
  {
    if (filename.length() < 5)
    {
      DALI_LOG_WARNING("Invalid (short) filename.");
    }
    FileFormat format(INVALID_FORMAT);

    const std::size_t filenameSize = filename.length();

    if(filenameSize >= 4){ // Avoid throwing out_of_range or failing silently if exceptions are turned-off on the compare(). (http://www.cplusplus.com/reference/string/string/compare/)
      if( !filename.compare( filenameSize - 4, 4, ".jpg" )
       || !filename.compare( filenameSize - 4, 4, ".JPG" ) )
      {
        format = JPG_FORMAT;
      }
      else if( !filename.compare( filenameSize - 4, 4, ".png" )
            || !filename.compare( filenameSize - 4, 4, ".PNG" ) )
      {
        format = PNG_FORMAT;
      }
      else if( !filename.compare( filenameSize - 4, 4, ".bmp" )
            || !filename.compare( filenameSize - 4, 4, ".BMP" ) )
      {
        format = BMP_FORMAT;
      }
      else if( !filename.compare( filenameSize - 4, 4, ".gif" )
            || !filename.compare( filenameSize - 4, 4, ".GIF" ) )
      {
        format = GIF_FORMAT;
      }
      else if( !filename.compare( filenameSize - 4, 4, ".ico" )
            || !filename.compare( filenameSize - 4, 4, ".ICO" ) )
      {
        format = ICO_FORMAT;
      }
      else if(filenameSize >= 5){
        if( !filename.compare( filenameSize - 5, 5, ".jpeg" )
         || !filename.compare( filenameSize - 5, 5, ".JPEG" ) )
        {
          format = JPG_FORMAT;
        }
      }
    }

    return format;
  }

  bool EncodeToFormat( const PixelBuffer* pixelBuffer, std::vector< unsigned char >& encodedPixels, FileFormat formatEncoding, std::size_t width, std::size_t height, Pixel::Format pixelFormat )
  {
    switch( formatEncoding )
    {
      case JPG_FORMAT:
      {
        return SlpPlatform::EncodeToJpeg( pixelBuffer, encodedPixels, width, height, pixelFormat );
        break;
      }
      case PNG_FORMAT:
      {
        return SlpPlatform::EncodeToPng( pixelBuffer, encodedPixels, width, height, pixelFormat );
        break;
      }
      default:
      {
        DALI_LOG_ERROR("Format not supported for image encoding (supported formats are PNG and JPEG)");
        break;
      }
    }
    return false;
  }

  bool EncodeToFile(const PixelBuffer * const pixelBuffer, const std::string& filename, const Pixel::Format pixelFormat, const std::size_t width, const std::size_t height)
  {
    DALI_ASSERT_DEBUG(pixelBuffer != 0 && filename.size() > 4 && width > 0 && height > 0);
    std::vector< unsigned char > pixbufEncoded;
    const FileFormat format = GetFormatFromFileName( filename );
    const bool encodeResult = EncodeToFormat( pixelBuffer, pixbufEncoded, format, width, height, pixelFormat );
    if(!encodeResult)
    {
      DALI_LOG_ERROR("Encoding pixels failed");
      return false;
    }
    return SlpPlatform::ResourceLoader::SaveFile( filename, pixbufEncoded );
  }
}

PixmapImage* PixmapImage::New(unsigned int width, unsigned int height, Dali::PixmapImage::ColorDepth depth, Dali::Adaptor& adaptor,  Any pixmap )
{
  PixmapImage* image = new PixmapImage( width, height, depth, adaptor, pixmap );
  DALI_ASSERT_DEBUG( image && "PixmapImage allocation failed." );

  // 2nd phase construction
  if(image) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

PixmapImage::PixmapImage(unsigned int width, unsigned int height, Dali::PixmapImage::ColorDepth depth, Dali::Adaptor& adaptor, Any pixmap)
: mWidth(width),
  mHeight(height),
  mOwnPixmap(true),
  mPixmap(0),
  mDisplay(NULL),
  mPixelFormat(Pixel::RGB888),
  mColorDepth(depth),
  mAdaptor(Internal::Adaptor::Adaptor::GetImplementation(adaptor)),
  mEglImageKHR(NULL)
{
  // assign the pixmap
  mPixmap = GetPixmapFromAny(pixmap);
}

void PixmapImage::Initialize()
{
  // Get render-surface being used by Dali
  Dali::RenderSurface& surface = mAdaptor.GetSurface();

  // get the X11 display pointer and store it
  // it is used by eglCreateImageKHR, and XFreePixmap
  // Any other display (x-connection) will fail in eglCreateImageKHR
  Any display = surface.GetDisplay();

  // the dali display pointer is needed
  mDisplay = AnyCast<Ecore_X_Display*>(display);

  // if pixmap has been created outside of X11 Image we can return
  if (mPixmap)
  {
    // we don't own the pixmap
    mOwnPixmap = false;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
    return;
  }

  // get the pixel depth
  int depth = GetPixelDepth(mColorDepth);

  // set the pixel format
  SetPixelFormat(depth);

  // Get the X-Renderable for which the pixmap is created on
  Any renderableSurface =  surface.GetSurface();

  // if Dali is using a Pixmap or Window to render to it doesn't matter because they have the same
  // underlying type of unsigned long
  Ecore_X_Window daliWindow = AnyCast< Ecore_X_Window >(renderableSurface);

  mPixmap = ecore_x_pixmap_new(daliWindow, mWidth, mHeight, depth);
  ecore_x_sync();
}

PixmapImage::~PixmapImage()
{
  // Lost the opportunity to call GlExtensionDestroy() if Adaptor is destroyed first
  if( Adaptor::IsAvailable() )
  {
    // GlExtensionDestroy() called from GLCleanup on the render thread. Checking this is done here.
    // (mEglImageKHR is now read/written from different threads although ref counted destruction
    //  should mean this isnt concurrent)
    DALI_ASSERT_ALWAYS( NULL == mEglImageKHR && "NativeImage GL resources have not been properly cleaned up" );
  }

  if (mOwnPixmap && mPixmap)
  {
    ecore_x_pixmap_free(mPixmap);
  }
}

Any PixmapImage::GetPixmap(Dali::PixmapImage::PixmapAPI api) const
{
  if (api == Dali::PixmapImage::ECORE_X11)
  {
    // return ecore x11 type
    return Any(mPixmap);
  }
  else
  {
    // return x11 type after casting it
    Pixmap pixmap=  static_cast<Pixmap>(mPixmap);
    return Any(pixmap);
  }
}

Any PixmapImage::GetDisplay() const
{
  // return ecore x11 type
  return Any(mDisplay);
}

bool PixmapImage::GetPixels(std::vector<unsigned char>& pixbuf, unsigned& width, unsigned& height, Pixel::Format& pixelFormat) const
{
  DALI_ASSERT_DEBUG(sizeof(unsigned) == 4);
  bool success = false;
  width  = mWidth;
  height = mHeight;

  XImageJanitor xImageJanitor( XGetImage(static_cast<Display*>(mDisplay),
                               mPixmap,
                               0, 0, // x,y of subregion to extract.
                               width, height, // of suregion to extract.
                               0xFFFFFFFF,
                               ZPixmap));
  XImage* const  pXImage = xImageJanitor.mXImage;
  DALI_ASSERT_DEBUG(pXImage && "XImage (from pixmap) could not be retrieved from the server");
  if(!pXImage)
  {
    DALI_LOG_ERROR("Could not retrieve Ximage.");
  }
  else
  {
    switch(pXImage->depth)
    {
      // Note, depth is a logical value. On target the framebuffer is still 32bpp
      // (see pXImage->bits_per_pixel) so we go through XGetPixel() and swizzle.
      // Note, this could be the default, fallback case for all depths if *pXImage
      // didn't have blank RGB masks (X bug), but we have to hardcode the masks and
      // shifts instead.
      case 24:
      {
        pixelFormat = Pixel::RGB888;
        pixbuf.resize(width*height*3);
        unsigned char* bufPtr = &pixbuf[0];

        for(unsigned y = height-1; y < height; --y)
        {
          for(unsigned x = 0; x < width; ++x, bufPtr+=3)
          {
            const unsigned pixel = XGetPixel(pXImage,x,y);

            // store as RGB
            const unsigned blue  =  pixel & 0xFFU;
            const unsigned green = (pixel >> 8)  & 0xFFU;
            const unsigned red   = (pixel >> 16) & 0xFFU;

            *bufPtr = red;
            *(bufPtr+1) = green;
            *(bufPtr+2) = blue;
          }
        }
        success = true;
        break;
      }
      case 32:
      {
        if(pXImage->data)
        {
          // Sweep through the image, doing a vertical flip, but handling each scanline as
          // an inlined intrinsic/builtin memcpy (should be fast):
          pixbuf.resize(width*height*4);
          unsigned * bufPtr = reinterpret_cast<unsigned *>(&pixbuf[0]);
          const unsigned xDataLineSkip = pXImage->bytes_per_line;
          const size_t copy_count = width * 4;
          pixelFormat = Pixel::BGRA8888;

          for(unsigned y = height-1; y < height; --y, bufPtr += width)
          {
            const char * const in = pXImage->data + xDataLineSkip * y;

            // Copy a whole scanline at a time:
            DALI_ASSERT_DEBUG( size_t( bufPtr ) >= size_t( &pixbuf[0] ));
            DALI_ASSERT_DEBUG( reinterpret_cast<size_t>( bufPtr ) + copy_count <= reinterpret_cast<size_t>( &pixbuf[pixbuf.size()] ) );
            DALI_ASSERT_DEBUG( in >= pXImage->data );
            DALI_ASSERT_DEBUG( in + copy_count <= pXImage->data + xDataLineSkip * height );
            __builtin_memcpy( bufPtr, in, copy_count );
          }
          success = true;
        }
        else
        {
          DALI_LOG_ERROR("XImage has null data pointer.");
        }
        break;
      }
      // Make a case for 16 bit modes especially to remember that the only reason we don't support them is a bug in X:
      case 16:
      {
        DALI_ASSERT_DEBUG(pXImage->red_mask && pXImage->green_mask && pXImage->blue_mask && "No image masks mean 16 bit modes are not possible.");
        ///! If the above assert doesn't fail in a debug build, the X bug may have been fixed, so revisit this function.
        ///! No break, fall through to the general unsupported format warning below.
      }
      default:
      {
        DALI_LOG_WARNING("Pixmap has unsupported bit-depth for getting pixels: %u", pXImage->depth);
      }
    }
  }
  if(!success)
  {
    DALI_LOG_ERROR("Failed to get pixels from PixmapImage.");
    pixbuf.resize(0);
    width = 0;
    height = 0;
  }
  return success;
}

bool PixmapImage::EncodeToFile(const std::string& filename) const
{
  std::vector< unsigned char > pixbuf;
  unsigned int width(0), height(0);
  Pixel::Format pixelFormat;

  if(GetPixels(pixbuf, width, height, pixelFormat))
  {
    return Dali::Internal::Adaptor::EncodeToFile(&pixbuf[0], filename, pixelFormat, width, height);
  }
  return false;
}

bool PixmapImage::GlExtensionCreate()
{
  // if the image existed previously delete it.
  if (mEglImageKHR != NULL)
  {
    GlExtensionDestroy();
  }

  EglImageExtensions* eglImageExtensions = GetEglImageExtensions();

  // casting from an unsigned int to a void *, which should then be cast back
  // to an unsigned int in the driver.
  EGLClientBuffer eglBuffer = reinterpret_cast< EGLClientBuffer > (mPixmap);

  mEglImageKHR = eglImageExtensions->CreateImageKHR( eglBuffer );

  return mEglImageKHR != NULL;
}

void PixmapImage::GlExtensionDestroy()
{
  EglImageExtensions* eglImageExtensions = GetEglImageExtensions();

  eglImageExtensions->DestroyImageKHR(mEglImageKHR);

  mEglImageKHR = NULL;
}

unsigned int PixmapImage::TargetTexture()
{
  EglImageExtensions* eglImageExtensions = GetEglImageExtensions();

  eglImageExtensions->TargetTextureKHR(mEglImageKHR);

  return 0;
}

int PixmapImage::GetPixelDepth(Dali::PixmapImage::ColorDepth depth) const
{
  switch (depth)
  {
    case Dali::PixmapImage::COLOR_DEPTH_DEFAULT:
    {
      // Get the default screen depth
      return ecore_x_default_depth_get(ecore_x_display_get(), ecore_x_default_screen_get());
    }
    case Dali::PixmapImage::COLOR_DEPTH_8:
    {
      return 8;
    }
    case Dali::PixmapImage::COLOR_DEPTH_16:
    {
      return 16;
    }
    case Dali::PixmapImage::COLOR_DEPTH_24:
    {
      return 24;
    }
    case Dali::PixmapImage::COLOR_DEPTH_32:
    {
      return 32;
    }
    default:
    {
      DALI_ASSERT_DEBUG(0 && "unknown color enum");
      return 0;
    }
  }
}

void PixmapImage::SetPixelFormat(int depth)
{
  // store the pixel format based on the depth
  switch (depth)
  {
    case 8:
    {
      mPixelFormat = Pixel::A8;
      break;
    }
    case 16:
    {
      mPixelFormat = Pixel::RGB565;
      break;
    }
    case 32:
    {
      mPixelFormat = Pixel::RGBA8888;
      break;
    }
    case 24:
    default:
    {
      mPixelFormat = Pixel::RGB888;
      break;
    }
  }
}

Ecore_X_Pixmap PixmapImage::GetPixmapFromAny(Any pixmap) const
{
  if (pixmap.Empty())
  {
    return 0;
  }

  // see if it is of type x11 pixmap
  if (pixmap.GetType() == typeid (Pixmap))
  {
    // get the x pixmap type
    Pixmap xpixmap = AnyCast<Pixmap>(pixmap);

    // cast it to a ecore pixmap type
    return static_cast<Ecore_X_Pixmap>(xpixmap);
  }
  else
  {
    return AnyCast<Ecore_X_Pixmap>(pixmap);
  }
}

void PixmapImage::GetPixmapDetails()
{
  int x, y;

  // get the width, height and depth
  ecore_x_pixmap_geometry_get(mPixmap, &x, &y, (int*)&mWidth, (int*)&mHeight);

  // set the pixel format
  SetPixelFormat(ecore_x_pixmap_depth_get(mPixmap));
}

EglImageExtensions* PixmapImage::GetEglImageExtensions() const
{
  EglFactory& factory = mAdaptor.GetEGLFactory();
  EglImageExtensions* egl = factory.GetImageExtensions();
  DALI_ASSERT_DEBUG( egl && "EGL Image Extensions not initialized" );
  return egl;
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
