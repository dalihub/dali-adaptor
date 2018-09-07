#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_H

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
#include <dali/devel-api/images/native-image-interface-extension.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

// INTERNAL INCLUDES
#include <native-image-source-queue-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class EglImageExtensions;

/**
 * Dali internal NativeImageSource.
 */
class NativeImageSourceQueueTizen: public Internal::Adaptor::NativeImageSourceQueue, public NativeImageInterface::Extension
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
  static NativeImageSourceQueueTizen* New(unsigned int width, unsigned int height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue );

  /**
   * @copydoc Dali::NativeImageSourceQueue::GetNativeImageSourceQueue()
   */
  Any GetNativeImageSourceQueue() const override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::SetSource( Any source )
   */
  void SetSource( Any source ) override;

  /**
   * destructor
   */
  ~NativeImageSourceQueueTizen() override;

  /**
   * @copydoc Dali::NativeImageInterface::GlExtensionCreate()
   */
  bool GlExtensionCreate() override;

  /**
   * @copydoc Dali::NativeImageInterface::GlExtensionDestroy()
   */
  void GlExtensionDestroy() override;

  /**
   * @copydoc Dali::NativeImageInterface::TargetTexture()
   */
  unsigned int TargetTexture() override;

  /**
   * @copydoc Dali::NativeImageInterface::PrepareTexture()
   */
  void PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImageInterface::GetWidth()
   */
  unsigned int GetWidth() const override
  {
    return mWidth;
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetHeight()
   */
  unsigned int GetHeight() const override
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
   * Private constructor; @see NativeImageSourceQueue::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImageSourceQueue contains tbm_surface_queue_h or is empty
   */
  NativeImageSourceQueueTizen( unsigned int width, unsigned int height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue );

  void Initialize( Dali::NativeImageSourceQueue::ColorDepth depth );

  void DestroyQueue();

  tbm_surface_queue_h GetSurfaceFromAny( Any source ) const;

  bool CheckBlending( int format );

private:

  typedef std::pair< tbm_surface_h, void* > EglImagePair;

  unsigned int                     mWidth;                ///< image width
  unsigned int                     mHeight;               ///< image height
  tbm_surface_queue_h              mTbmQueue;             ///< Tbm surface queue handle
  tbm_surface_h                    mConsumeSurface;       ///< The current tbm surface
  std::vector< EglImagePair >      mEglImages;            ///< EGL Image vector
  EglImageExtensions*              mEglImageExtensions;   ///< The EGL Image Extensions
  bool                             mOwnTbmQueue;          ///< Whether we created tbm queue
  bool                             mBlendingRequired;     ///< Whether blending is required
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_H
