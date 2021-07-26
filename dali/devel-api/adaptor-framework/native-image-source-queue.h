#ifndef DALI_NATIVE_IMAGE_SOURCE_QUEUE_H
#define DALI_NATIVE_IMAGE_SOURCE_QUEUE_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/images/native-image-interface.h>
#include <dali/public-api/object/any.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class NativeImageSourceQueue;
}
} // namespace DALI_INTERNAL

class NativeImageSourceQueue;

/**
 * @brief Pointer to Dali::NativeImageSourceQueue.
 */
typedef Dali::IntrusivePtr<Dali::NativeImageSourceQueue> NativeImageSourceQueuePtr;

/**
 * @brief Used for displaying native images.
 *
 * NativeImage is a platform specific way of providing pixel data to the GPU for rendering,for example via an EGL image.
 * NativeImageSourceQueue can be created internally or externally by native image source.
 * It has a queue which handles some image buffers.
 * Someone should fill the buffers and enqueue them, then DALi will show them.
 */
class DALI_ADAPTOR_API NativeImageSourceQueue : public NativeImageInterface
{
public:
  /**
    * @brief Enumeration for the instance when creating a native image, the color depth has to be specified.
    */
  enum ColorDepth
  {
    COLOR_DEPTH_DEFAULT, ///< Uses the current screen default depth (recommended)
    COLOR_DEPTH_24,      ///< 24 bits per pixel
    COLOR_DEPTH_32       ///< 32 bits per pixel
  };

  /**
   * @brief Creates a new NativeImageSourceQueue.
   *        Depending on hardware, the width and height may have to be a power of two.
   * @param[in] width The width of the image
   * @param[in] height The height of the image
   * @param[in] depth color depth of the image
   * @return A smart-pointer to a newly allocated image
   */
  static NativeImageSourceQueuePtr New(uint32_t width, uint32_t height, ColorDepth depth);

  /**
   * @brief Creates a new NativeImageSourceQueue from an existing native image source.
   *
   * @param[in] nativeImageSourceQueue NativeImageSourceQueue must be a any handle with native image source
   * @return A smart-pointer to a newly allocated image
   * @see NativeImageInterface
   */
  static NativeImageSourceQueuePtr New(Any nativeImageSourceQueue);

  /**
   * @brief Retrieves the internal native image.
   *
   * @return Any object containing the internal native image source queue
   */
  Any GetNativeImageSourceQueue();

  /**
   * @brief Sets the size of the image.
   *
   * @param[in] width The width of the image
   * @param[in] height The height of the image
   */
  void SetSize(uint32_t width, uint32_t height);

  /**
   * @brief Ignores a source image which is inserted to the queue.
   *
   * @note This can be called from worker threads.
   */
  void IgnoreSourceImage();

  /**
   * @brief Checks if the buffer can be got from the queue.
   *
   * Check the available buffer using this API before call DequeueBuffer()
   * @return True if the buffer can be got from the queue.
   */
  bool CanDequeueBuffer();

  /**
   * @brief Dequeue buffer from the queue.
   *
   * Acquire buffer and information of the queue.
   * it returns the information of the buffer.
   * @param[out] width The width of buffer
   * @param[out] height The height of buffer
   * @param[out] stride The stride of buffer
   * @return A pointer of buffer
   */
  uint8_t* DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride);

  /**
   * @brief Enqueue buffer to the queue.
   *
   * Enqueue buffer to the queue
   * this requests the release of the buffer internally.
   * @param[in] buffer A pointer of buffer
   * @return True if success
   */
  bool EnqueueBuffer(uint8_t* buffer);

  /**
   * @copydoc Dali::NativeImageInterface::GetTextureTarget()
   */
  int GetTextureTarget() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomFragmentPrefix()
   */
  const char* GetCustomFragmentPrefix() const override;

  /**
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader()
   */
  bool ApplyNativeFragmentShader(std::string& shader) override;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename() const override;

private: // native image
  /**
   * @copydoc Dali::NativeImageInterface::CreateResource()
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
  uint32_t GetWidth() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetHeight()
   */
  uint32_t GetHeight() const override;

  /**
   * @copydoc Dali::NativeImageInterface::RequiresBlending()
   */
  bool RequiresBlending() const override;

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
  NativeImageInterface::Extension* GetExtension() override;

private:
  /// @cond internal
  /**
   * @brief Private constructor.
   * @param[in] width The width of the image
   * @param[in] height The height of the image
   * @param[in] depth color depth of the image
   * @param[in] nativeImageSourceQueue contains either: native image source or is empty
   */
  DALI_INTERNAL NativeImageSourceQueue(uint32_t width, uint32_t height, ColorDepth depth, Any nativeImageSourceQueue);

  /**
   * @brief A reference counted object may only be deleted by calling Unreference().
   *
   * The implementation should destroy the NativeImage resources.
   */
  DALI_INTERNAL ~NativeImageSourceQueue() override;

  /**
   * @brief Undefined copy constructor.
   *
   * This avoids accidental calls to a default copy constructor.
   * @param[in] nativeImageSourceQueue A reference to the object to copy
   */
  DALI_INTERNAL NativeImageSourceQueue(const NativeImageSourceQueue& nativeImageSourceQueue);

  /**
   * @brief Undefined assignment operator.
   *
   * This avoids accidental calls to a default assignment operator.
   * @param[in] rhs A reference to the object to copy
   */
  DALI_INTERNAL NativeImageSourceQueue& operator=(const NativeImageSourceQueue& rhs);
  /// @endcond

private:
  /// @cond internal
  std::unique_ptr<Internal::Adaptor::NativeImageSourceQueue> mImpl; ///< Implementation pointer
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_NATIVE_IMAGE_SOURCE_QUEUE_H
