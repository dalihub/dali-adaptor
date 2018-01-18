#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_H

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
  static NativeImageSource* New(unsigned int width,
                                unsigned int height,
                                Dali::NativeImageSource::ColorDepth depth,
                                Any nativeImageSource);
  /**
   * @copydoc Dali::NativeImageSource::GetNativeImageSource()
   */
  virtual Any GetNativeImageSource() const;

  /**
   * @copydoc Dali::NativeImageSource::GetPixels()
   */
  virtual bool GetPixels(std::vector<unsigned char> &pixbuf, unsigned int &width, unsigned int &height, Pixel::Format& pixelFormat ) const;

  /**
   * @copydoc Dali::NativeImageSource::EncodeToFile(const std::string& )
   */
  virtual bool EncodeToFile(const std::string& filename) const;

  /**
   * @copydoc Dali::NativeImageSource::SetSource( Any source )
   */
  virtual void SetSource( Any source );

  /**
   * @copydoc Dali::NativeImageSource::IsColorDepthSupported( ColorDepth colorDepth )
   */
  virtual bool IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth );

  /**
   * destructor
   */
  virtual ~NativeImageSource();

  /**
   * @copydoc Dali::NativeImageSource::GlExtensionCreate()
   */
  virtual bool GlExtensionCreate();

  /**
   * @copydoc Dali::NativeImageSource::GlExtensionDestroy()
   */
  virtual void GlExtensionDestroy();

  /**
   * @copydoc Dali::NativeImageSource::TargetTexture()
   */
  virtual unsigned int TargetTexture();

  /**
   * @copydoc Dali::NativeImageSource::PrepareTexture()
   */
  virtual void PrepareTexture();

  /**
   * @copydoc Dali::NativeImageSource::GetWidth()
   */
  virtual unsigned int GetWidth() const;

  /**
   * @copydoc Dali::NativeImageSource::GetHeight()
   */
  virtual unsigned int GetHeight() const;

  /**
   * @copydoc Dali::NativeImageSource::RequiresBlending()
   */
  virtual bool RequiresBlending() const;

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  virtual NativeImageInterface::Extension* GetNativeImageInterfaceExtension();
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_H
