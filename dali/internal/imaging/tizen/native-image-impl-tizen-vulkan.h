#ifndef DALI_INTERNAL_NATIVE_IMAGE_IMPL_TIZEN_VULKAN_H
#define DALI_INTERNAL_NATIVE_IMAGE_IMPL_TIZEN_VULKAN_H

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
#include <dali/internal/imaging/common/native-image-impl.h>
#include <dali/public-api/adaptor-framework/native-image.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Implementation of NativeImage for single TBM surface buffer that is imported into Vulkan.
 */
class NativeImageTizenVulkan : public Internal::Adaptor::NativeImage
{
public:
  /**
   * Create a new NativeImage internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImage contains tbm_surface_h or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageTizenVulkan* New(uint32_t                      width,
                                     uint32_t                      height,
                                     Dali::NativeImage::ColorDepth depth,
                                     Any                           nativeImage);

  /**
   * @copydoc Dali::NativeImage::GetNativeImage()
   */
  Any GetNativeImage() const override;

  /**
   * @copydoc Dali::NativeImage::GetPixels()
   */
  bool GetPixels(Dali::Vector<uint8_t>& pixbuf, uint32_t& width, uint32_t& height, Pixel::Format& pixelFormat) const override;

  /**
   * @copydoc Dali::NativeImage::SetPixels()
   */
  bool SetPixels(uint8_t* pixbuf, const Pixel::Format& pixelFormat) override;

  /**
   * @copydoc Dali::NativeImage::SetSource( Any source )
   */
  void SetSource(Any source) override;

  /**
   * @copydoc Dali::NativeImage::IsColorDepthSupported( ColorDepth colorDepth )
   */
  bool IsColorDepthSupported(Dali::NativeImage::ColorDepth colorDepth) override;

  /**
   * destructor
   */
  ~NativeImageTizenVulkan() override;

  /**
   * @copydoc Dali::NativeImage::CreateResource()
   */
  bool CreateResource() override;

  /**
   * @copydoc Dali::NativeImage::DestroyResource()
   */
  void DestroyResource() override;

  /**
   * @copydoc Dali::NativeImage::TargetTexture()
   */
  uint32_t TargetTexture() override;

  /**
   * @copydoc Dali::NativeImage::PrepareTexture()
   */
  Dali::NativeImageInterface::PrepareTextureResult PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImage::GetWidth()
   */
  uint32_t GetWidth() const override
  {
    return mWidth;
  }

  /**
   * @copydoc Dali::NativeImage::GetHeight()
   */
  uint32_t GetHeight() const override
  {
    return mHeight;
  }

  /**
   * @copydoc Dali::NativeImage::RequiresBlending()
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
   * @copydoc Dali::Internal::Adaptor::NativeImage::AcquireBuffer()
   */
  uint8_t* AcquireBuffer(uint32_t& width, uint32_t& height, uint32_t& stride) override;

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImage::ReleaseBuffer()
   */
  bool ReleaseBuffer(const Rect<uint32_t>& updatedArea) override;

  /**
   * @copydoc Dali::NativeImage::SetResourceDestructionCallback()
   */
  void SetResourceDestructionCallback(EventThreadCallback* callback, bool ownedCallback) override;

  /**
   * @copydoc Dali::DevelNativeImage::EnableBackBuffer()
   */
  void EnableBackBuffer(bool enable) override;

private:
  /**
   * Private constructor; @see NativeImage::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImage contains either tbm_surface_h or is empty
   */
  NativeImageTizenVulkan(uint32_t                      width,
                         unsigned int                  height,
                         Dali::NativeImage::ColorDepth depth,
                         Any                           nativeImage);

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
  uint32_t                      mWidth;          ///< Image width
  uint32_t                      mHeight;         ///< Image height
  Dali::NativeImage::ColorDepth mColorDepth;     ///< Color depth of image
  tbm_surface_h                 mTbmSurface;     ///< TBM surface
  tbm_surface_h                 mTbmBackSurface; ///< Back buffer
  tbm_format                    mTbmFormat;      ///< TBM format

  std::vector<int> mPlaneFds; ///< DMA-BUF file descriptors per plane

  Rect<uint32_t>     mUpdatedArea{}; ///< Updated area
  mutable std::mutex mMutex;

  EventThreadCallback* mResourceDestructionCallback; ///< The Resource Destruction Callback

  bool mOwnTbmSurface : 1;    ///< Whether we created pixmap or not
  bool mBlendingRequired : 1; ///< Whether blending is required
  bool mSetSource : 1;
  bool mResourceCreated : 1;   ///< Resource creation state
  bool mIsBufferAcquired : 1;  ///< Whether AcquireBuffer is called
  bool mBackBufferEnabled : 1; ///< Whether the back buffer is enabled
  bool mOwnResourceDestructionCallback : 1;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_IMPL_TIZEN_VULKAN_H
