#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_ANDROID_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_ANDROID_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
 * Dali internal NativeImageSourceQueue.
 */
class NativeImageSourceQueueAndroid: public Internal::Adaptor::NativeImageSourceQueue, public NativeImageInterface::Extension
{
public:

  /**
   * Create a new NativeImageSourceQueueAndroid internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] depth color depth of the image.
   * @param[in] nativeImageSourceQueue contains tbm_surface_queue_h or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageSourceQueueAndroid* New( uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue );

  /**
   * @copydoc Dali::NativeImageSourceQueue::GetNativeImageSourceQueue()
   */
  Any GetNativeImageSourceQueue() const override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::SetSize
   */
  void SetSize( uint32_t width, uint32_t height ) override;

  /**
   * destructor
   */
  ~NativeImageSourceQueueAndroid() override;

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
    return true;
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

private:

  /**
   * Private constructor; @see NativeImageSourceQueue::New()
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colour depth of the image.
   * @param[in] nativeImageSourceQueue contains tbm_surface_queue_h or is empty
   */
  NativeImageSourceQueueAndroid( uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue );

private:

  uint32_t    mWidth;                ///< image width
  uint32_t    mHeight;               ///< image height

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_ANDROID_H
