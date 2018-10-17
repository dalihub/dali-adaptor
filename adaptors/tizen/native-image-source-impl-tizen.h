#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_TIZEN_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_TIZEN_H

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
#include <tbm_surface.h>
#include <dali/devel-api/images/native-image-interface-extension.h>
#include <dali/public-api/common/dali-vector.h>

// INTERNAL INCLUDES
#include <native-image-source.h>
#include <native-image-source-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class EglFactory;
class EglImageExtensions;

/**
 * Dali internal NativeImageSource.
 */
class NativeImageSourceTizen: public Internal::Adaptor::NativeImageSource, public NativeImageInterface::Extension
{
public:

  /**
   * Create a new NativeImageSource internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImageSource contains tbm_surface_h or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageSourceTizen* New(unsigned int width,
                          unsigned int height,
                          Dali::NativeImageSource::ColorDepth depth,
                          Any nativeImageSource);

  /**
   * @copydoc Dali::NativeImageSource::GetNativeImageSource()
   */
  Any GetNativeImageSource() const override;

  /**
   * @copydoc Dali::NativeImageSource::GetPixels()
   */
  bool GetPixels(std::vector<unsigned char> &pixbuf, unsigned int &width, unsigned int &height, Pixel::Format& pixelFormat ) const  override;

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
  ~NativeImageSourceTizen() override;

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
  unsigned int TargetTexture() override;

  /**
   * @copydoc Dali::NativeImageSource::PrepareTexture()
   */
  void PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImageSource::GetWidth()
   */
  unsigned int GetWidth() const override
  {
    return mWidth;
  }

  /**
   * @copydoc Dali::NativeImageSource::GetHeight()
   */
  unsigned int GetHeight() const override
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
    return this;
  }

  /**
   * @copydoc Dali::NativeImageInterface::Extension::GetCustomFragmentPreFix()
   */
  const char* GetCustomFragmentPreFix() override;

  /**
   * @copydoc Dali::NativeImageInterface::Extension::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename() override;

  /**
   * @copydoc Dali::NativeImageInterface::Extension::GetEglImageTextureTarget()
   */
  int GetEglImageTextureTarget() override;

  /**
   * @copydoc Dali::NativeImageInterface::Extension::SetDestructorNotification((void *notification)
   */
  void SetDestructorNotification(void* notification) override;

private:

  /**
   * Private constructor; @see NativeImageSource::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImageSource contains either: pixmap of type X11 Pixmap , a Ecore_X_Pixmap or is empty
   */
  NativeImageSourceTizen(unsigned int width,
              unsigned  int height,
              Dali::NativeImageSource::ColorDepth depth,
              Any nativeImageSource);

  void Initialize();

  tbm_surface_h GetSurfaceFromAny( Any source ) const;

  bool CheckBlending( tbm_format format );

private:

  unsigned int mWidth;                        ///< image width
  unsigned int mHeight;                       ///< image height
  bool mOwnTbmSurface;                        ///< Whether we created pixmap or not
  tbm_surface_h mTbmSurface;
  tbm_format mTbmFormat;
  bool mBlendingRequired;                      ///< Whether blending is required
  Dali::NativeImageSource::ColorDepth mColorDepth;  ///< color depth of image
  void* mEglImageKHR;                         ///< From EGL extension
  EglFactory* mEglFactory;                    ///< EGL Factory
  EglImageExtensions* mEglImageExtensions;    ///< The EGL Image Extensions
  bool mSetSource;
  void *mNotification;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_TIZEN_H
