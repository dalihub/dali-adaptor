#ifndef DALI_NATIVE_IMAGE_SOURCE_QUEUE_H
#define DALI_NATIVE_IMAGE_SOURCE_QUEUE_H

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
#include <dali/public-api/images/native-image-interface.h>
#include <dali/public-api/object/any.h>

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
class NativeImageQueue;
}
} // namespace Internal DALI_INTERNAL

class NativeImageSourceQueue;

/**
 * @brief Pointer to Dali::NativeImageSourceQueue.
 */
typedef Dali::IntrusivePtr<Dali::NativeImageSourceQueue> NativeImageSourceQueuePtr;

/**
 * @brief Used for displaying native images.
 *
 * @deprecated Use NativeImageQueue instead. This is kept for legacy apps.
 *
 * NativeImage is a platform specific way of providing pixel data to the GPU for rendering,for example via an EGL image.
 * NativeImageSourceQueue can be created internally or externally by native image.
 * It has a queue which handles some image buffers.
 * Someone should fill the buffers and enqueue them, then DALi will show them.
 */
class DALI_ADAPTOR_API NativeImageSourceQueue : public NativeImageInterface
{
public:
  /**
   * @brief Enumeration for the instance when creating a native image, the color depth has to be specified.
   * @note This ColorFormat follows pixel byte order.
   */
  enum class ColorFormat
  {
    BGR888   = 0, /// 8 blue bits, 8 green bits, 8 red bits
    BGRA8888 = 1, /// 8 blue bits, 8 green bits, 8 red bits, alpha 8 bits
    BGRX8888 = 2, /// 8 blue bits, 8 green bits, 8 red bits, and 8 ignored bits
    RGB888   = 3, /// 8 red bits, 8 green bits, 8 blue bits
    RGBA8888 = 4, /// 8 red bits, 8 green bits, 8 blue bits, alpha 8 bits
    RGBX8888 = 5, /// 8 red bits, 8 green bits, 8 blue bits, and 8 ignored bits
  };

  /**
   * @brief Creates a new NativeImageSourceQueue.
   *        Depending on hardware, the width and height may have to be a power of two.
   *        It will use 3, or defined by DALI_TBM_SURFACE_QUEUE_SIZE as default.
   * @param[in] width The width of the image
   * @param[in] height The height of the image
   * @param[in] colorFormat The color format of the image
   * @return A smart-pointer to a newly allocated image
   */
  static NativeImageSourceQueuePtr New(uint32_t width, uint32_t height, ColorFormat colorFormat);

  /**
   * @brief Retrieves the internal native image.
   *
   * @return Any object containing the internal native image queue
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
   * @copydoc Dali::NativeImageInterface::GetTextureTarget()
   */
  int GetTextureTarget() const override;

  /**
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader(Dali::String& shader)
   */
  bool ApplyNativeFragmentShader(Dali::String& shader) override;

  /**
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader(Dali::String& shader, int mask);
   */
  bool ApplyNativeFragmentShader(Dali::String& shader, int mask) override;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename() const override;

  /**
   * @brief Get the number of queue count for this image.
   *
   * @return The number of queue count.
   */
  uint32_t GetQueueCount() const;

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
  Dali::NativeImageInterface::PrepareTextureResult PrepareTexture() override;

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
   * @copydoc Dali::NativeImageInterface::GetUpdatedArea()
   */
  Rect<uint32_t> GetUpdatedArea() override;

  /**
   * @copydoc Dali::NativeImageInterface::PostRender()
   */
  void PostRender() override;

private:
  /// @cond internal
  /**
   * @brief Private constructor.
   * @param[in] queueCount The number of queue of the image. If it is 0, will use default.
   * @param[in] width The width of the image
   * @param[in] height The height of the image
   * @param[in] colorFormat The color format of the image
   * @param[in] nativeImageSourceQueue contains either: native image or is empty
   */
  DALI_INTERNAL NativeImageSourceQueue(uint32_t queueCount, uint32_t width, uint32_t height, ColorFormat colorFormat, Any nativeImageSourceQueue);

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
   * @param[in] nativeImageQueue A reference to the object to copy
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
  Internal::Adaptor::NativeImageQueue* mImpl; ///< Implementation pointer
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_NATIVE_IMAGE_SOURCE_QUEUE_H
