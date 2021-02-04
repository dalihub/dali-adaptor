#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-source-queue-impl.h>

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
class NativeImageSourceQueueTizen : public Internal::Adaptor::NativeImageSourceQueue
{
public:
  /**
   * Create a new NativeImageSourceQueueTizen internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImageSourceQueue contains tbm_surface_queue_h or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageSourceQueueTizen* New(uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue);

  /**
   * @copydoc Dali::NativeImageSourceQueue::GetNativeImageSourceQueue()
   */
  Any GetNativeImageSourceQueue() const override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::SetSize
   */
  void SetSize(uint32_t width, uint32_t height) override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::IgnoreSourceImage
   */
  void IgnoreSourceImage() override;

  /**
   * destructor
   */
  ~NativeImageSourceQueueTizen() override;

  /**
   * @copydoc Dali::NativeImageInterface::CreateResource
   */
  bool CreateResource() override;

  /**
   * @copydoc Dali::NativeImageInterface::DestroyResource()
   */
  void DestroyResource() override;

  /**
   * @copydoc Dali::NativeImageInterface::TargetTexture()
   */
  uint32_t TargetTexture() override;

  /**
   * @copydoc Dali::NativeImageInterface::PrepareTexture()
   */
  void PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImageInterface::GetWidth()
   */
  uint32_t GetWidth() const override
  {
    return mWidth;
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetHeight()
   */
  uint32_t GetHeight() const override
  {
    return mHeight;
  }

  /**
   * @copydoc Dali::NativeImageInterface::RequiresBlending()
   */
  bool RequiresBlending() const override
  {
    return mBlendingRequired;
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomFragmentPrefix()
   */
  const char* GetCustomFragmentPrefix() const override;

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
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetNativeImageInterfaceExtension() override
  {
    return nullptr;
  }

private:
  /**
   * Private constructor; @see NativeImageSourceQueue::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImageSourceQueue contains tbm_surface_queue_h or is empty
   */
  NativeImageSourceQueueTizen(uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue);

  void Initialize(Dali::NativeImageSourceQueue::ColorDepth depth);

  void ResetEglImageList();

  tbm_surface_queue_h GetSurfaceFromAny(Any source) const;

  bool CheckBlending(int format);

private:
  typedef std::pair<tbm_surface_h, void*> EglImagePair;

  Dali::Mutex               mMutex;              ///< Mutex
  uint32_t                  mWidth;              ///< image width
  uint32_t                  mHeight;             ///< image height
  tbm_surface_queue_h       mTbmQueue;           ///< Tbm surface queue handle
  tbm_surface_h             mConsumeSurface;     ///< The current tbm surface
  std::vector<EglImagePair> mEglImages;          ///< EGL Image vector
  EglGraphics*              mEglGraphics;        ///< EGL Graphics
  EglImageExtensions*       mEglImageExtensions; ///< The EGL Image Extensions
  bool                      mOwnTbmQueue;        ///< Whether we created tbm queue
  bool                      mBlendingRequired;   ///< Whether blending is required
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_H
