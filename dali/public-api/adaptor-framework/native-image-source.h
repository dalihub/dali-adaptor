#ifndef __DALI_NATIVE_IMAGE_SOURCE_H__
#define __DALI_NATIVE_IMAGE_SOURCE_H__

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

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
 * @SINCE_1_0.0
 */
typedef Dali::IntrusivePtr<Dali::NativeImageSource> NativeImageSourcePtr;

/**
 * @brief Used for displaying native images.
 *
 * NativeImageSource can be created internally or
 * externally by native image source.
 * NativeImage is a platform specific way of providing pixel data to the GPU for rendering,
 * for example via an EGL image.
 *
 * @SINCE_1_1.4
 * @see NativeImage
 */
class DALI_ADAPTOR_API NativeImageSource : public NativeImageInterface
{
public:

   /**
    * @brief Enumeration for the instance when creating a native image, the color depth has to be specified.
    * @SINCE_1_0.0
    */
   enum ColorDepth
   {
     COLOR_DEPTH_DEFAULT,     ///< Uses the current screen default depth (recommended) @SINCE_1_0.0
     COLOR_DEPTH_8,           ///< 8 bits per pixel @SINCE_1_0.0
     COLOR_DEPTH_16,          ///< 16 bits per pixel @SINCE_1_0.0
     COLOR_DEPTH_24,          ///< 24 bits per pixel @SINCE_1_0.0
     COLOR_DEPTH_32           ///< 32 bits per pixel @SINCE_1_0.0
   };

  /**
   * @brief Creates a new NativeImageSource.
   *
   * Depending on hardware, the width and height may have to be a power of two.
   * @SINCE_1_0.0
   * @param[in] width The width of the image
   * @param[in] height The height of the image
   * @param[in] depth color depth of the image
   * @return A smart-pointer to a newly allocated image
   */
  static NativeImageSourcePtr New( unsigned int width, unsigned int height, ColorDepth depth );

  /**
   * @brief Creates a new NativeImageSource from an existing native image source.
   *
   * @SINCE_1_0.0
   * @param[in] nativeImageSource NativeImageSource must be a any handle with native image source
   * @return A smart-pointer to a newly allocated image
   * @see NativeImageInterface
   */
  static NativeImageSourcePtr New( Any nativeImageSource );

  /**
   * @brief Retrieves the internal native image.
   *
   * @SINCE_1_0.0
   * @return Any object containing the internal native image source
   */
  Any GetNativeImageSource();

  /**
   * @brief Gets a copy of the pixels used by NativeImageSource.
   *
   * This is only supported for 24 bit RGB and 32 bit RGBA internal formats
   * (COLOR_DEPTH_24 and COLOR_DEPTH_32).
   * @SINCE_1_0.0
   * @param[out] pixbuf A vector to store the pixels in
   * @param[out] width The width of image
   * @param[out] height The height of image
   * @param[out] pixelFormat pixel format used by image
   * @return     @c true if the pixels were gotten, and @c false otherwise
   */
  bool GetPixels( std::vector<unsigned char>& pixbuf, unsigned int& width, unsigned int& height, Pixel::Format& pixelFormat ) const;

  /**
   * @brief Converts the current pixel contents to either a JPEG or PNG format
   * and write that to the filesystem.
   *
   * @SINCE_1_0.0
   * @param[in] filename Identify the filesystem location at which to write the encoded image.
   *                     The extension determines the encoding used.
   *                     The two valid encoding are (".jpeg"|".jpg") and ".png".
   * @return    @c true if the pixels were written, and @c false otherwise
   */
  bool EncodeToFile(const std::string& filename) const;

  /**
   * @brief Sets an existing source.
   *
   * @SINCE_1_1.19
   * @param[in] source Any handle with the source
   */
  void SetSource( Any source );

  /**
   * @brief Checks if the specified color depth is supported.
   *
   * @SINCE_1_1.34
   * @param[in] colorDepth The color depth to check
   * @return @c true if colorDepth is supported, @c false otherwise
   */
  bool IsColorDepthSupported( ColorDepth colorDepth );

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

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetExtension();

private:

  /// @cond internal
  /**
   * @brief Private constructor.
   * @SINCE_1_0.0
   * @param[in] width The width of the image
   * @param[in] height The height of the image
   * @param[in] depth color depth of the image
   * @param[in] nativeImageSource contains either: native image source or is empty
   */
  DALI_INTERNAL NativeImageSource( unsigned int width, unsigned int height, ColorDepth depth, Any nativeImageSource );

  /**
   * @brief A reference counted object may only be deleted by calling Unreference().
   *
   * The implementation should destroy the NativeImage resources.
   * @SINCE_1_0.0
   */
  DALI_INTERNAL virtual ~NativeImageSource();

  /**
   * @brief Undefined copy constructor.
   *
   * This avoids accidental calls to a default copy constructor.
   * @SINCE_1_0.0
   * @param[in] nativeImageSource A reference to the object to copy
   */
  DALI_INTERNAL NativeImageSource( const NativeImageSource& nativeImageSource );

  /**
   * @brief Undefined assignment operator.
   *
   * This avoids accidental calls to a default assignment operator.
   * @SINCE_1_0.0
   * @param[in] rhs A reference to the object to copy
   */
  DALI_INTERNAL NativeImageSource& operator=(const NativeImageSource& rhs);
  /// @endcond

private:

  /// @cond internal
  Internal::Adaptor::NativeImageSource* mImpl; ///< Implementation pointer
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_NATIVE_IMAGE_SOURCE_H__
