#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_H

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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/native-image-source.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Dali internal NativeImageSource.
 */
class NativeImageSource
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
  static NativeImageSource* New(uint32_t width,
                                uint32_t height,
                                Dali::NativeImageSource::ColorDepth depth,
                                Any nativeImageSource);
  /**
   * @copydoc Dali::NativeImageSource::GetNativeImageSource()
   */
  virtual Any GetNativeImageSource() const = 0;

  /**
   * @copydoc Dali::NativeImageSource::GetPixels()
   */
  virtual bool GetPixels(std::vector<unsigned char> &pixbuf, uint32_t &width, uint32_t &height, Pixel::Format& pixelFormat ) const = 0;

  /**
   * @copydoc Dali::NativeImageSource::EncodeToFile(const std::string& )
   */
  virtual bool EncodeToFile(const std::string& filename) const = 0;

  /**
   * @copydoc Dali::NativeImageSource::SetSource( Any source )
   */
  virtual void SetSource( Any source ) = 0;

  /**
   * @copydoc Dali::NativeImageSource::IsColorDepthSupported( ColorDepth colorDepth )
   */
  virtual bool IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth ) = 0;

  /**
   * destructor
   */
  virtual ~NativeImageSource() = default;

  /**
   * @copydoc Dali::NativeImageSource::GlExtensionCreate()
   */
  virtual bool GlExtensionCreate() = 0;

  /**
   * @copydoc Dali::NativeImageSource::GlExtensionDestroy()
   */
  virtual void GlExtensionDestroy() = 0;

  /**
   * @copydoc Dali::NativeImageSource::TargetTexture()
   */
  virtual uint32_t TargetTexture() = 0;

  /**
   * @copydoc Dali::NativeImageSource::PrepareTexture()
   */
  virtual void PrepareTexture() = 0;

  /**
   * @copydoc Dali::NativeImageSource::GetWidth()
   */
  virtual uint32_t GetWidth() const = 0;

  /**
   * @copydoc Dali::NativeImageSource::GetHeight()
   */
  virtual uint32_t GetHeight() const = 0;

  /**
   * @copydoc Dali::NativeImageSource::RequiresBlending()
   */
  virtual bool RequiresBlending() const = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  virtual NativeImageInterface::Extension* GetNativeImageInterfaceExtension() = 0;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_H
