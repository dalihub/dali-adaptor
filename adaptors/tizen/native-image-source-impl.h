#ifndef __DALI_INTERNAL_NATIVE_IMAGE_SOURCE_H__
#define __DALI_INTERNAL_NATIVE_IMAGE_SOURCE_H__

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

// EXTERNAL INCLUDES
#include <tbm_surface.h>
#include <dali/devel-api/images/native-image-interface-extension.h>
#include <dali/public-api/common/dali-vector.h>

// INTERNAL INCLUDES
#include <native-image-source.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
class EglImageExtensions;

namespace
{
  typedef Dali::Vector< void* > EglImageContainer;
};

/**
 * Dali internal NativeImageSource.
 */
class NativeImageSource: public NativeImageInterface::Extension
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
  Any GetNativeImageSource() const;

  /**
   * @copydoc Dali::NativeImageSource::GetPixels()
   */
  bool GetPixels(std::vector<unsigned char> &pixbuf, unsigned int &width, unsigned int &height, Pixel::Format& pixelFormat ) const;

  /**
   * @copydoc Dali::NativeImageSource::EncodeToFile(const std::string& )
   */
  bool EncodeToFile(const std::string& filename) const;

  /**
   * @copydoc Dali::NativeImageSource::SetSource( Any source )
   */
  void SetSource( Any source );

  /**
   * @copydoc Dali::NativeImageSource::IsColorDepthSupported( ColorDepth colorDepth )
   */
  bool IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth );

  /**
   * destructor
   */
  ~NativeImageSource();

  /**
   * @copydoc Dali::NativeImageSource::GlExtensionCreate()
   */
  bool GlExtensionCreate();

  /**
   * @copydoc Dali::NativeImageSource::GlExtensionDestroy()
   */
  void GlExtensionDestroy();

  /**
   * @copydoc Dali::NativeImageSource::TargetTexture()
   */
  unsigned int TargetTexture();

  /**
   * @copydoc Dali::NativeImageSource::PrepareTexture()
   */
  void PrepareTexture();

  /**
   * @copydoc Dali::NativeImageSource::GetWidth()
   */
  unsigned int GetWidth() const
  {
    return mWidth;
  }

  /**
   * @copydoc Dali::NativeImageSource::GetHeight()
   */
  unsigned int GetHeight() const
  {
    return mHeight;
  }

  /**
   * @copydoc Dali::NativeImageSource::RequiresBlending()
   */
  bool RequiresBlending() const
  {
    return mBlendingRequired;
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetNativeImageInterfaceExtension()
  {
    return this;
  }

  /**
   * @copydoc Dali::NativeImageInterface::Extension::GetCustomFragmentPreFix()
   */
  const char* GetCustomFragmentPreFix();

  /**
   * @copydoc Dali::NativeImageInterface::Extension::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename();

  /**
   * @copydoc Dali::NativeImageInterface::Extension::GetEglImageTextureTarget()
   */
  int GetEglImageTextureTarget();

private:

  /**
   * Private constructor; @see NativeImageSource::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImageSource contains either: pixmap of type X11 Pixmap , a Ecore_X_Pixmap or is empty
   */
  NativeImageSource(unsigned int width,
              unsigned  int height,
              Dali::NativeImageSource::ColorDepth depth,
              Any nativeImageSource);

  void Initialize();

  int GetPixelDepth(Dali::NativeImageSource::ColorDepth depth) const;

  tbm_surface_h GetSurfaceFromAny( Any source ) const;

private:

  unsigned int mWidth;                        ///< image width
  unsigned int mHeight;                       ///< image heights
  bool mOwnTbmsurface;                            ///< Whether we created pixmap or not
  tbm_surface_h mTbmsurface;
  bool mBlendingRequired;                      ///< Whether blending is required
  Dali::NativeImageSource::ColorDepth mColorDepth;  ///< color depth of image
  void* mEglImageKHR;                         ///< From EGL extension
  EglImageExtensions* mEglImageExtensions;    ///< The EGL Image Extensions
  bool mSetSource;
  EglImageContainer mEglImageKHRContainer;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_NATIVE_IMAGE_SOURCE_H__
