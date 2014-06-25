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
#include <Ecore.h>
#include <Ecore_Wayland.h>
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
  mPixelFormat(Pixel::RGB888),
  mColorDepth(depth),
  mAdaptor(Internal::Adaptor::Adaptor::GetImplementation(adaptor)),
  mEglImageKHR(NULL)
{
}

void PixmapImage::Initialize()
{
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
}

Any PixmapImage::GetPixmap(Dali::PixmapImage::PixmapAPI api) const
{
    return NULL;
}

Any PixmapImage::GetDisplay() const
{
    return NULL;
}

bool PixmapImage::GetPixels(std::vector<unsigned char>& pixbuf, unsigned& width, unsigned& height, Pixel::Format& pixelFormat) const
{
    return false;
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
    return false;
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

void PixmapImage::GetPixmapDetails()
{
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
