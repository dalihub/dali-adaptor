#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_TIZEN_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_TIZEN_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-vector.h>
#include <tbm_surface.h>
#include <memory>
#include <mutex>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-source-impl.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class EglGraphics;
class EglImageExtensions;

/**
 * Dali internal NativeImageSource.
 */
class NativeImageSourceTizen : public Internal::Adaptor::NativeImageSource
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
  static NativeImageSourceTizen* New(uint32_t                            width,
                                     uint32_t                            height,
                                     Dali::NativeImageSource::ColorDepth depth,
                                     Any                                 nativeImageSource);

  /**
   * @copydoc Dali::NativeImageSource::GetNativeImageSource()
   */
  Any GetNativeImageSource() const override;

  /**
   * @copydoc Dali::NativeImageSource::GetPixels()
   */
  bool GetPixels(std::vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const override;

  /**
   * @copydoc Dali::NativeImageSource::SetPixels()
   */
  bool SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat) override;

  /**
   * @copydoc Dali::NativeImageSource::SetSource( Any source )
   */
  void SetSource(Any source) override;

  /**
   * @copydoc Dali::NativeImageSource::IsColorDepthSupported( ColorDepth colorDepth )
   */
  bool IsColorDepthSupported(Dali::NativeImageSource::ColorDepth colorDepth) override;

  /**
   * destructor
   */
  ~NativeImageSourceTizen() override;

  /**
   * @copydoc Dali::NativeImageSource::CreateResource()
   */
  bool CreateResource() override;

  /**
   * @copydoc Dali::NativeImageSource::DestroyResource()
   */
  void DestroyResource() override;

  /**
   * @copydoc Dali::NativeImageSource::TargetTexture()
   */
  uint32_t TargetTexture() override;

  /**
   * @copydoc Dali::NativeImageSource::PrepareTexture()
   */
  Dali::NativeImageInterface::PrepareTextureResult PrepareTexture() override;

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
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader(std::string& shader)
   */
  bool ApplyNativeFragmentShader(std::string& shader) override;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetTextureTarget()
   */
  int GetTextureTarget() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetNativeImageHandle()
   */
  Any GetNativeImageHandle() const override;

  /**
   * @copydoc Dali::NativeImageInterface::SourceChanged()
   */
  bool SourceChanged() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetUpdatedArea()
   */
  Rect<uint32_t> GetUpdatedArea() override;

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetNativeImageInterfaceExtension() override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageSource::AcquireBuffer()
   */
  uint8_t* AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride) override;

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageSource::ReleaseBuffer()
   */
  bool ReleaseBuffer(const Rect<uint32_t>& updatedArea) override;

  /**
   * @copydoc Dali::NativeImageSource::SetResourceDestructionCallback()
   */
  void SetResourceDestructionCallback(EventThreadCallback* callback) override;

  /**
   * @copydoc Dali::DevelNativeImageSource::EnableBackBuffer()
   */
  void EnableBackBuffer(bool enable) override;

private:
  /**
   * Private constructor; @see NativeImageSource::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImageSource contains either: pixmap of type X11 Pixmap , a Ecore_X_Pixmap or is empty
   */
  NativeImageSourceTizen(uint32_t                            width,
                         unsigned int                        height,
                         Dali::NativeImageSource::ColorDepth depth,
                         Any                                 nativeImageSource);

  void Initialize();

  tbm_surface_h GetSurfaceFromAny(Any source) const;

  bool CheckBlending(tbm_format format);

  void DestroySurface();

  /**
   * @brief Create back buffer
   */
  void CreateBackBuffer();

  /**
   * @brief Destroy back buffer
   */
  void DestroyBackBuffer();

private:
  uint32_t                             mWidth;          ///< image width
  uint32_t                             mHeight;         ///< image height
  tbm_surface_h                        mTbmSurface;     ///< Tbm surface
  tbm_surface_h                        mTbmBackSurface; ///< Back buffer
  tbm_format                           mTbmFormat;
  Dali::NativeImageSource::ColorDepth  mColorDepth;    ///< color depth of image
  Rect<uint32_t>                       mUpdatedArea{}; ///< Updated area
  mutable std::mutex                   mMutex;
  void*                                mEglImageKHR;                 ///< From EGL extension
  EglGraphics*                         mEglGraphics;                 ///< EGL Graphics
  EglImageExtensions*                  mEglImageExtensions;          ///< The EGL Image Extensions
  std::unique_ptr<EventThreadCallback> mResourceDestructionCallback; ///< The Resource Destruction Callback

  bool mOwnTbmSurface : 1;    ///< Whether we created pixmap or not
  bool mBlendingRequired : 1; ///< Whether blending is required
  bool mEglImageChanged : 1;  ///< Whether EGLImage changed or not. Reset flag as false at PrepareTexture().
  bool mSetSource : 1;
  bool mIsBufferAcquired : 1;  ///< Whether AcquireBuffer is called
  bool mBackBufferEnabled : 1; ///< Whether the back buffer is enabled
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_TIZEN_H
