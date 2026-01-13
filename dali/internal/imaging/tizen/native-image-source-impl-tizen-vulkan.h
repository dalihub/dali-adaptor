#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_TIZEN_VULKAN_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_TIZEN_VULKAN_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>
#include <tbm_surface.h>
#include <memory>
#include <mutex>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/imaging/common/native-image-source-impl.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Implementation of NativeImageSource for single TBM surface buffer that is imported into Vulkan.
 */
class NativeImageSourceTizenVulkan : public Internal::Adaptor::NativeImageSource
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
  static NativeImageSourceTizenVulkan* New(uint32_t                            width,
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
  ~NativeImageSourceTizenVulkan() override;

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
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader(std::string& shader, int mask)
   */
  bool ApplyNativeFragmentShader(std::string& shader, int mask) override;

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
   * @copydoc Dali::NativeImageInterface::PostRender()
   */
  void PostRender() override
  {
  }

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
   * @param[in] nativeImageSource contains either tbm_surface_h or is empty
   */
  NativeImageSourceTizenVulkan(uint32_t                            width,
                               unsigned int                        height,
                               Dali::NativeImageSource::ColorDepth depth,
                               Any                                 nativeImageSource);

  /**
   * @brief Initialize TBM surface from parameters or Any object.
   */
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
  uint32_t                            mWidth;          ///< Image width
  uint32_t                            mHeight;         ///< Image height
  Dali::NativeImageSource::ColorDepth mColorDepth;     ///< Color depth of image
  tbm_surface_h                       mTbmSurface;     ///< TBM surface
  tbm_surface_h                       mTbmBackSurface; ///< Back buffer
  tbm_format                          mTbmFormat;      ///< TBM format

  std::vector<int> mPlaneFds; ///< DMA-BUF file descriptors per plane

  Rect<uint32_t>     mUpdatedArea{}; ///< Updated area
  mutable std::mutex mMutex;

  std::unique_ptr<EventThreadCallback> mResourceDestructionCallback; ///< The Resource Destruction Callback
  bool                                 mOwnTbmSurface : 1;           ///< Whether we created pixmap or not
  bool                                 mBlendingRequired : 1;        ///< Whether blending is required
  bool                                 mSetSource : 1;
  bool                                 mResourceCreated : 1;   ///< Resource creation state
  bool                                 mIsBufferAcquired : 1;  ///< Whether AcquireBuffer is called
  bool                                 mBackBufferEnabled : 1; ///< Whether the back buffer is enabled
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_IMPL_TIZEN_VULKAN_H
