#ifndef __DALI_NATIVE_IMAGE_SOURCE_H__
#define __DALI_NATIVE_IMAGE_SOURCE_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <string>

#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/images/native-image-interface.h>
#include <dali/public-api/images/pixel.h>
#include <dali/public-api/object/any.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class NativeImageSource;
}
}

class NativeImageSource;
/**
 * @brief Pointer to Dali::NativeImageSource.
 */
typedef IntrusivePtr<NativeImageSource> NativeImageSourcePtr;

/**
 * @brief Used for displaying native images.
 *
 * The native image source can be created internally or
 * externally by X11 or ECORE-X11.
 *
 * @since DALi 1.1.4
 */
class DALI_IMPORT_API NativeImageSource : public NativeImageInterface
{
public:

   /**
    * @brief When creating a native image the color depth has to be specified.
    */
   enum ColorDepth
   {
     COLOR_DEPTH_DEFAULT,     ///< Uses the current X screen default depth (recommended)
     COLOR_DEPTH_8,           ///< 8 bits per pixel
     COLOR_DEPTH_16,          ///< 16 bits per pixel
     COLOR_DEPTH_24,          ///< 24 bits per pixel
     COLOR_DEPTH_32           ///< 32 bits per pixel
   };

  /**
   * @brief Create a new NativeImageSource.
   *
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageSourcePtr New( unsigned int width, unsigned int height, ColorDepth depth );

  /**
   * @brief Create a new NativeImageSource from an existing native image.
   *
   * @param[in] nativeImageSource must be a X11 pixmap or a Ecore_X_Pixmap
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageSourcePtr New( Any nativeImageSource );

  /**
   * @brief Retrieve the internal native image.
   *
   * @return Any object containing the internal native image.
   */
  Any GetNativeImageSource();

  /**
   * @brief Get a copy of the pixels used by NativeImageSource.
   *
   * This is only supported for 24 bit RGB and 32 bit RGBA internal formats
   * (COLOR_DEPTH_24 and COLOR_DEPTH_32).
   * @param[out] pixbuf a vector to store the pixels in
   * @param[out] width  width of image
   * @param[out] height height of image
   * @param[out] pixelFormat pixel format used by image
   * @return     True if the pixels were gotten, and false otherwise.
   */
  bool GetPixels( std::vector<unsigned char>& pixbuf, unsigned int& width, unsigned int& height, Pixel::Format& pixelFormat ) const;

  /**
   * @brief Convert the current pixel contents to either a JPEG or PNG format
   * and write that to the filesytem.
   *
   * @param[in] filename Identify the filesytem location at which to write the
   *                     encoded image. The extension determines the encoding used.
   *                     The two valid encoding are (".jpeg"|".jpg") and ".png".
   * @return    True if the pixels were written, and false otherwise.
   */
  bool EncodeToFile(const std::string& filename) const;

private:   // native image

  /**
   * @copydoc Dali::NativeImageInterface::GlExtensionCreate()
   */
  virtual bool GlExtensionCreate();

  /**
   * @copydoc Dali::NativeImageInterface::GlExtensionDestroy()
   */
  virtual void GlExtensionDestroy();

  /**
   * @copydoc Dali::NativeImageInterface::TargetTexture()
   */
  virtual unsigned int TargetTexture();

  /**
   * @copydoc Dali::NativeImageInterface::PrepareTexture()
   */
  virtual void PrepareTexture();

  /**
   * @copydoc Dali::NativeImageInterface::GetWidth()
   */
  virtual unsigned int GetWidth() const;

  /**
   * @copydoc Dali::NativeImageInterface::GetHeight()
   */
  virtual unsigned int GetHeight() const;

  /**
   * @copydoc Dali::NativeImageInterface::RequiresBlending()
   */
  virtual bool RequiresBlending() const;

private:

  /**
   * @brief Private constructor
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImageSource contains either: pixmap of type X11 Pixmap , a Ecore_X_Pixmap or is empty
   */
  DALI_INTERNAL NativeImageSource( unsigned int width, unsigned int height, ColorDepth depth, Any nativeImageSource );

  /**
   * @brief A reference counted object may only be deleted by calling Unreference().
   *
   * The implementation should destroy the NativeImage resources.
   */
  DALI_INTERNAL virtual ~NativeImageSource();

  /**
   * @brief Undefined copy constructor
   *
   * This avoids accidental calls to a default copy constructor.
   * @param[in] nativeImageSource A reference to the object to copy.
   */
  DALI_INTERNAL NativeImageSource( const NativeImageSource& nativeImageSource );

  /**
   * @brief Undefined assignment operator.
   *
   * This avoids accidental calls to a default assignment operator.
   * @param[in] rhs A reference to the object to copy.
   */
  DALI_INTERNAL NativeImageSource& operator=(const NativeImageSource& rhs);

private:

  Internal::Adaptor::NativeImageSource* mImpl; ///< Implementation pointer
};

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_NATIVE_IMAGE_SOURCE_H__
