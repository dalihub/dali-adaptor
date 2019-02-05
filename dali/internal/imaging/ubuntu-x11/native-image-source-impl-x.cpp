/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/ubuntu-x11/native-image-source-impl-x.h>

// EXTERNAL INCLUDES
#include <Ecore_X.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/devel-api/adaptor-framework/bitmap-saver.h>
#include <dali/graphics/graphics-interface.h>
#include <dali/integration-api/render-surface.h>


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
          DALI_LOG_ERROR("XImage deallocation failure");
        }
      }
    }
    XImage* const  mXImage;
  private:
    XImageJanitor( const XImageJanitor& rhs );
    XImageJanitor& operator = ( const XImageJanitor& rhs );
  };
}

NativeImageSourceX* NativeImageSourceX::New(unsigned int width, unsigned int height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
{
  NativeImageSourceX* image = new NativeImageSourceX( width, height, depth, nativeImageSource );
  DALI_ASSERT_DEBUG( image && "NativeImageSource allocation failed." );

  // 2nd phase construction
  if(image) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

NativeImageSourceX::NativeImageSourceX( unsigned int width, unsigned int height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
: mWidth( width ),
  mHeight( height ),
  mOwnPixmap( true ),
  mPixmap( 0 ),
  mBlendingRequired( false ),
  mColorDepth( depth ),
  mEglImageKHR( NULL ),
  mEglImageExtensions( NULL )
{
  DALI_ASSERT_ALWAYS( Adaptor::IsAvailable() );

#if 0
  GraphicsInterface* graphics = &( Adaptor::GetImplementation( Adaptor::Get() ).GetGraphicsInterface() );
  auto eglGraphics = static_cast<EglGraphics *>(graphics);

  mEglImageExtensions = eglGraphics->GetImageExtensions();
  DALI_ASSERT_DEBUG( mEglImageExtensions );
#endif

  // assign the pixmap
  mPixmap = GetPixmapFromAny(nativeImageSource);
}

void NativeImageSourceX::Initialize()
{
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

  // set whether blending is required according to pixel format based on the depth
  /* default pixel format is RGB888
     If depth = 8, Pixel::A8;
     If depth = 16, Pixel::RGB565;
     If depth = 32, Pixel::RGBA8888 */
  mBlendingRequired = ( depth == 32 || depth == 8 );

  mPixmap = ecore_x_pixmap_new( 0, mWidth, mHeight, depth );
  ecore_x_sync();
}

NativeImageSourceX::~NativeImageSourceX()
{
  if (mOwnPixmap && mPixmap)
  {
    ecore_x_pixmap_free(mPixmap);
  }
}

Any NativeImageSourceX::GetNativeImageSource() const
{
  // return ecore x11 type
  return Any(mPixmap);
}

bool NativeImageSourceX::GetPixels(std::vector<unsigned char>& pixbuf, unsigned& width, unsigned& height, Pixel::Format& pixelFormat) const
{
  DALI_ASSERT_DEBUG(sizeof(unsigned) == 4);
  bool success = false;
  width  = mWidth;
  height = mHeight;

  // Open a display connection
  Display* displayConnection = XOpenDisplay( 0 );

  XImageJanitor xImageJanitor( XGetImage( displayConnection,
                                          mPixmap,
                                          0, 0, // x,y of subregion to extract.
                                          width, height, // of subregion to extract.
                                          0xFFFFFFFF,
                                          ZPixmap ) );
  XImage* const  pXImage = xImageJanitor.mXImage;
  DALI_ASSERT_DEBUG(pXImage && "XImage (from pixmap) could not be retrieved from the server");
  if(!pXImage)
  {
    DALI_LOG_ERROR("Could not retrieve Ximage.\n");
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
          const size_t copy_count = static_cast< size_t >( width ) * 4;
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
          DALI_LOG_ERROR("XImage has null data pointer.\n");
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
        DALI_LOG_WARNING("Pixmap has unsupported bit-depth for getting pixels: %u\n", pXImage->depth);
      }
    }
  }
  if(!success)
  {
    DALI_LOG_ERROR("Failed to get pixels from NativeImageSource.\n");
    pixbuf.resize(0);
    width = 0;
    height = 0;
  }

  // Close the display connection
  XCloseDisplay( displayConnection );

  return success;
}

bool NativeImageSourceX::EncodeToFile(const std::string& filename) const
{
  std::vector< unsigned char > pixbuf;
  unsigned int width(0), height(0);
  Pixel::Format pixelFormat;

  if(GetPixels(pixbuf, width, height, pixelFormat))
  {
    return Dali::EncodeToFile(&pixbuf[0], filename, pixelFormat, width, height);
  }
  return false;
}

void NativeImageSourceX::SetSource( Any source )
{
  mPixmap = GetPixmapFromAny( source );

  if (mPixmap)
  {
    // we don't own the pixmap
    mOwnPixmap = false;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
  }
}

bool NativeImageSourceX::IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth )
{
  return true;
}

bool NativeImageSourceX::GlExtensionCreate()
{
#if 0
  // if the image existed previously delete it.
  if (mEglImageKHR != NULL)
  {
    GlExtensionDestroy();
  }

  // casting from an unsigned int to a void *, which should then be cast back
  // to an unsigned int in the driver.
  EGLClientBuffer eglBuffer = reinterpret_cast< EGLClientBuffer > (mPixmap);

  mEglImageKHR = mEglImageExtensions->CreateImageKHR( eglBuffer );

#endif
  return mEglImageKHR != NULL;
}

void NativeImageSourceX::GlExtensionDestroy()
{
#if 0
  mEglImageExtensions->DestroyImageKHR(mEglImageKHR);

  mEglImageKHR = NULL;
#endif
}

unsigned int NativeImageSourceX::TargetTexture()
{
#if 0
  mEglImageExtensions->TargetTextureKHR(mEglImageKHR);
#endif
  return 0;
}

void NativeImageSourceX::PrepareTexture()
{
}

int NativeImageSourceX::GetPixelDepth(Dali::NativeImageSource::ColorDepth depth) const
{
  switch (depth)
  {
    case Dali::NativeImageSource::COLOR_DEPTH_DEFAULT:
    {
      // Get the default screen depth
      return ecore_x_default_depth_get(ecore_x_display_get(), ecore_x_default_screen_get());
    }
    case Dali::NativeImageSource::COLOR_DEPTH_8:
    {
      return 8;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_16:
    {
      return 16;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_24:
    {
      return 24;
    }
    case Dali::NativeImageSource::COLOR_DEPTH_32:
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

Ecore_X_Pixmap NativeImageSourceX::GetPixmapFromAny(Any pixmap) const
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

void NativeImageSourceX::GetPixmapDetails()
{
  int x, y;

  // get the width, height and depth
  ecore_x_pixmap_geometry_get( mPixmap, &x, &y, reinterpret_cast< int* >( &mWidth ), reinterpret_cast< int* >( &mHeight ) );

  // set whether blending is required according to pixel format based on the depth
  /* default pixel format is RGB888
     If depth = 8, Pixel::A8;
     If depth = 16, Pixel::RGB565;
     If depth = 32, Pixel::RGBA8888 */
  int depth = ecore_x_pixmap_depth_get(mPixmap);
  mBlendingRequired = ( depth == 32 || depth == 8 );
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
