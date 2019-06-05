#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_H

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

// EXTERNAL INCLUDES
#include <hardware_buffer.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-interface.h>

#include <dali/public-api/adaptor-framework/native-image-source.h>

#include <dali/internal/imaging/common/native-image-source-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
class EglImageExtensions;

/**
 * Dali internal NativeImageSource.
 */
class NativeImageSourceAndroid : public Internal::Adaptor::NativeImageSource
{
public:

  /**
   * Create a new NativeImageSource internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImageSource contains either: pixmap of type X11 Pixmap , a Ecore_X_Pixmap or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageSourceAndroid* New( uint32_t width,
                          uint32_t height,
                          Dali::NativeImageSource::ColorDepth depth,
                          Any nativeImageSource);
  /**
   * @copydoc Dali::NativeImageSource::GetNativeImageSource()
   */
  Any GetNativeImageSource() const override;

  /**
   * @copydoc Dali::NativeImageSource::GetPixels()
   */
  bool GetPixels(std::vector<unsigned char> &pixbuf, uint32_t &width, uint32_t &height, Pixel::Format& pixelFormat ) const override;

  /**
   * @copydoc Dali::NativeImageSource::EncodeToFile(const std::string& )
   */
  bool EncodeToFile(const std::string& filename) const override;

  /**
   * @copydoc Dali::NativeImageSource::SetSource( Any source )
   */
  void SetSource( Any source ) override;

  /**
   * @copydoc Dali::NativeImageSource::IsColorDepthSupported( ColorDepth colorDepth )
   */
  bool IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth ) override;

  /**
   * destructor
   */
  ~NativeImageSourceAndroid() override;

  /**
   * @copydoc Dali::NativeImageSource::GlExtensionCreate()
   */
  bool GlExtensionCreate() override;

  /**
   * @copydoc Dali::NativeImageSource::GlExtensionDestroy()
   */
  void GlExtensionDestroy() override;

  /**
   * @copydoc Dali::NativeImageSource::TargetTexture()
   */
  uint32_t TargetTexture() override;

  /**
   * @copydoc Dali::NativeImageSource::PrepareTexture()
   */
  void PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImageSource::GetWidth()
   */
  uint32_t GetWidth() const override
  {
    return mWidth;
  }

  /**
   * @copydoc Dali::NativeImageSource::GetHeight()
   */
  uint32_t GetHeight() const override
  {
    return mHeight;
  }

  /**
   * @copydoc Dali::NativeImageSource::RequiresBlending()
   */
  bool RequiresBlending() const override
  {
    return mBlendingRequired;
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetNativeImageInterfaceExtension() override
  {
    return nullptr;
  }

private:

  /**
   * Private constructor; @see NativeImageSource::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImageSource contains either: pixmap of type X11 Pixmap , a Ecore_X_Pixmap or is empty
   */
  NativeImageSourceAndroid( uint32_t width,
              uint32_t height,
              Dali::NativeImageSource::ColorDepth depth,
              Any nativeImageSource);

  /**
   * 2nd phase construction.
   */
  void Initialize();

  /**
   * Uses X11 to get the default depth.
   * @param depth the PixelImage depth enum
   * @return default x11 pixel depth
   */
  int GetPixelDepth(Dali::NativeImageSource::ColorDepth depth) const;

  /**
   * Gets the pixmap from the Any parameter
   * @param pixmap contains either: pixmap of type X11 Pixmap , a Ecore_X_Pixmap or is empty
   * @return pixmap pixmap
   */
  void* GetPixmapFromAny(Any pixmap) const;

  /**
   * Given an existing pixmap, the function uses X to find out
   * the width, heigth and depth of that pixmap.
   */
  void GetPixmapDetails();

private:

  uint32_t mWidth;                            ///< image width
  uint32_t mHeight;                           ///< image heights
  bool mOwnPixmap;                            ///< Whether we created pixmap or not
  AHardwareBuffer* mPixmap;                   ///<
  bool mBlendingRequired;                     ///< Whether blending is required
  Dali::NativeImageSource::ColorDepth mColorDepth;  ///< color depth of image
  void* mEglImageKHR;                         ///< From EGL extension
  EglImageExtensions* mEglImageExtensions;    ///< The EGL Image Extensions
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_H
