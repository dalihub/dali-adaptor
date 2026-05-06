/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/ubuntu-x11/native-image-impl-x.h>

// EXTERNAL INCLUDES
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
// Pieces needed to save compressed images (temporary location while plumbing):
namespace
{
/**
 * Free an allocated XImage on destruction.
 */
struct XImageJanitor
{
  XImageJanitor(XImage* const pXImage)
  : mXImage(pXImage)
  {
    DALI_ASSERT_DEBUG(pXImage != 0 && "Null pointer to XImage.");
  }

  ~XImageJanitor()
  {
    if(mXImage)
    {
      if(!XDestroyImage(mXImage))
      {
        DALI_LOG_ERROR("XImage deallocation failure");
      }
    }
  }
  XImage* const mXImage;

private:
  XImageJanitor(const XImageJanitor& rhs);
  XImageJanitor& operator=(const XImageJanitor& rhs);
};
} // namespace

NativeImageX* NativeImageX::New(uint32_t width, uint32_t height, Dali::NativeImage::ColorDepth depth, Any nativeImage)
{
  NativeImageX* image = new NativeImageX(width, height, depth, nativeImage);
  DALI_ASSERT_DEBUG(image && "NativeImage allocation failed.");

  // 2nd phase construction
  if(image) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

NativeImageX::NativeImageX(uint32_t width, uint32_t height, Dali::NativeImage::ColorDepth depth, Any nativeImage)
: mWidth(width),
  mHeight(height),
  mOwnPixmap(true),
  mPixmap(0),
  mBlendingRequired(false),
  mColorDepth(depth),
  mEglImageChanged(false),
  mEglImageKHR(NULL),
  mEglGraphics(NULL),
  mEglImageExtensions(NULL),
  mResourceDestructionCallback(nullptr),
  mOwnResourceDestructionCallback(false)
{
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  auto graphics = &(Adaptor::GetImplementation(Adaptor::Get()).GetGraphicsInterface());
  mEglGraphics  = static_cast<EglGraphics*>(graphics);

  // assign the pixmap
  mPixmap = GetPixmapFromAny(nativeImage);
}

void NativeImageX::Initialize()
{
  // if pixmap has been created outside of X11 Image we can return
  if(mPixmap)
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
  mBlendingRequired = (depth == 32 || depth == 8);

  mPixmap = ecore_x_pixmap_new(0, mWidth, mHeight, depth);
  ecore_x_sync();
}

NativeImageX::~NativeImageX()
{
  if(mOwnResourceDestructionCallback)
  {
    delete mResourceDestructionCallback;
  }
  if(mOwnPixmap && mPixmap)
  {
    // Temporarily disable this as this causes a crash with EFL Version 1.24.0
    // ecore_x_pixmap_free(mPixmap);
  }
}

Any NativeImageX::GetNativeImage() const
{
  // return ecore x11 type
  return Any(mPixmap);
}

bool NativeImageX::GetPixels(Dali::Vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const
{
  DALI_ASSERT_DEBUG(sizeof(uint32_t) == 4);
  bool success = false;
  width        = mWidth;
  height       = mHeight;

  // Open a display connection
  Display* displayConnection = XOpenDisplay(0);

  XImageJanitor xImageJanitor(XGetImage(displayConnection,
                                        mPixmap,
                                        0,
                                        0, // x,y of subregion to extract.
                                        width,
                                        height, // of subregion to extract.
                                        0xFFFFFFFF,
                                        ZPixmap));
  XImage* const pXImage = xImageJanitor.mXImage;
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
        pixbuf.ResizeUninitialized(width * height * 3);
        uint8_t* bufPtr = &pixbuf[0];

        for(uint32_t y = 0; y < height; ++y)
        {
          for(uint32_t x = 0; x < width; ++x, bufPtr += 3)
          {
            const uint32_t pixel = XGetPixel(pXImage, x, y);

            // store as RGB
            const uint32_t blue  = pixel & 0xFFU;
            const uint32_t green = (pixel >> 8) & 0xFFU;
            const uint32_t red   = (pixel >> 16) & 0xFFU;

            *bufPtr       = red;
            *(bufPtr + 1) = green;
            *(bufPtr + 2) = blue;
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
          pixbuf.ResizeUninitialized(width * height * 4);
          uint32_t*      bufPtr        = reinterpret_cast<uint32_t*>(&pixbuf[0]);
          const uint32_t xDataLineSkip = pXImage->bytes_per_line;
          const size_t   copy_count    = static_cast<size_t>(width) * 4;
          pixelFormat                  = Pixel::BGRA8888;

          for(uint32_t y = 0; y < height; ++y, bufPtr += width)
          {
            const char* const in = pXImage->data + xDataLineSkip * y;

            // Copy a whole scanline at a time:
            DALI_ASSERT_DEBUG(size_t(bufPtr) >= size_t(&pixbuf[0]));
            DALI_ASSERT_DEBUG(reinterpret_cast<size_t>(bufPtr) + copy_count <= reinterpret_cast<size_t>(pixbuf.End()));
            DALI_ASSERT_DEBUG(in >= pXImage->data);
            DALI_ASSERT_DEBUG(in + copy_count <= pXImage->data + xDataLineSkip * height);
            __builtin_memcpy(bufPtr, in, copy_count);
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
        DALI_LOG_ERROR("Pixmap has unsupported bit-depth for getting pixels: %u\n", pXImage->depth);
      }
    }
  }
  if(!success)
  {
    DALI_LOG_ERROR("Failed to get pixels from NativeImage.\n");
    pixbuf.ResizeUninitialized(0);
    width  = 0;
    height = 0;
  }

  // Close the display connection
  XCloseDisplay(displayConnection);

  return success;
}

bool NativeImageX::SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat)
{
  return false;
}

void NativeImageX::SetSource(Any source)
{
  mPixmap = GetPixmapFromAny(source);

  if(mPixmap)
  {
    // we don't own the pixmap
    mOwnPixmap = false;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
  }
}

bool NativeImageX::IsColorDepthSupported(Dali::NativeImage::ColorDepth colorDepth)
{
  return true;
}

bool NativeImageX::CreateResource()
{
  mEglImageExtensions = mEglGraphics->GetImageExtensions();
  DALI_ASSERT_DEBUG(mEglImageExtensions);

  // if the image existed previously delete it.
  if(mEglImageKHR != NULL)
  {
    DestroyResource();
  }

  // casting from an unsigned int to a void *, which should then be cast back
  // to an unsigned int in the driver.
  EGLClientBuffer eglBuffer = reinterpret_cast<EGLClientBuffer>(mPixmap);

  mEglImageKHR     = mEglImageExtensions->CreateImageKHR(eglBuffer);
  mEglImageChanged = true;

  return mEglImageKHR != NULL;
}

void NativeImageX::DestroyResource()
{
  mEglImageExtensions->DestroyImageKHR(mEglImageKHR);

  mEglImageKHR     = NULL;
  mEglImageChanged = true;

  if(mResourceDestructionCallback)
  {
    mResourceDestructionCallback->Trigger();
  }
}

uint32_t NativeImageX::TargetTexture()
{
  mEglImageExtensions->TargetTextureKHR(mEglImageKHR);

  return 0;
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageX::PrepareTexture()
{
  Dali::NativeImageInterface::PrepareTextureResult result = Dali::NativeImageInterface::PrepareTextureResult::UNKNOWN_ERROR;
  if(DALI_LIKELY(mEglImageKHR))
  {
    result           = mEglImageChanged ? Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED : Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR;
    mEglImageChanged = false;
  }
  else
  {
    result = mEglImageExtensions ? Dali::NativeImageInterface::PrepareTextureResult::NOT_INITIALIZED_GRAPHICS : Dali::NativeImageInterface::PrepareTextureResult::NOT_INITIALIZED_IMAGE;
  }

  return result;
}

int NativeImageX::GetPixelDepth(Dali::NativeImage::ColorDepth depth) const
{
  switch(depth)
  {
    case Dali::NativeImage::COLOR_DEPTH_DEFAULT:
    {
      // Get the default screen depth
      return ecore_x_default_depth_get(ecore_x_display_get(), ecore_x_default_screen_get());
    }
    case Dali::NativeImage::COLOR_DEPTH_8:
    {
      return 8;
    }
    case Dali::NativeImage::COLOR_DEPTH_16:
    {
      return 16;
    }
    case Dali::NativeImage::COLOR_DEPTH_24:
    {
      return 24;
    }
    case Dali::NativeImage::COLOR_DEPTH_32:
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

int NativeImageX::GetTextureTarget() const
{
  return GL_TEXTURE_2D;
}

bool NativeImageX::ApplyNativeFragmentShader(std::string& shader, int mask)
{
  return false;
}

const char* NativeImageX::GetCustomSamplerTypename() const
{
  return nullptr;
}

Any NativeImageX::GetNativeImageHandle() const
{
  return Any(mPixmap);
}

bool NativeImageX::SourceChanged() const
{
  return true;
}

Ecore_X_Pixmap NativeImageX::GetPixmapFromAny(Any pixmap) const
{
  if(pixmap.Empty())
  {
    return 0;
  }

  // see if it is of type x11 pixmap
  if(pixmap.IsType<Pixmap>())
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

void NativeImageX::GetPixmapDetails()
{
  int x, y;

  // get the width, height and depth
  ecore_x_pixmap_geometry_get(mPixmap, &x, &y, reinterpret_cast<int*>(&mWidth), reinterpret_cast<int*>(&mHeight));

  // set whether blending is required according to pixel format based on the depth
  /* default pixel format is RGB888
     If depth = 8, Pixel::A8;
     If depth = 16, Pixel::RGB565;
     If depth = 32, Pixel::RGBA8888 */
  int depth         = ecore_x_pixmap_depth_get(mPixmap);
  mBlendingRequired = (depth == 32 || depth == 8);
}

uint8_t* NativeImageX::AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return NULL;
}

bool NativeImageX::ReleaseBuffer(const Rect<uint32_t>& updatedArea)
{
  return false;
}

void NativeImageX::SetResourceDestructionCallback(EventThreadCallback* callback, bool ownedCallback)
{
  if(mOwnResourceDestructionCallback)
  {
    delete mResourceDestructionCallback;
  }
  mResourceDestructionCallback    = callback;
  mOwnResourceDestructionCallback = ownedCallback;
}

void NativeImageX::EnableBackBuffer(bool enable)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
