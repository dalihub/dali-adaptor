#ifndef DALI_INTERNAL_NATIVE_IMAGE_QUEUE_IMPL_H
#define DALI_INTERNAL_NATIVE_IMAGE_QUEUE_IMPL_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-queue.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal NativeImageQueue.
 */
class NativeImageQueue
{
public:
  /**
   * @copydoc Dali::NativeImageQueue::New()
   */
  static NativeImageQueue* New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue);

  /**
   * @copydoc Dali::NativeImageQueue::GetNativeImageQueue()
   */
  virtual Any GetNativeImageQueue() const = 0;

  /**
   * @copydoc Dali::NativeImageQueue::GetQueueCount
   */
  virtual uint32_t GetQueueCount() const = 0;

  /**
   * @copydoc Dali::NativeImageQueue::SetSize
   */
  virtual void SetSize(uint32_t width, uint32_t height) = 0;

  /**
   * @copydoc Dali::NativeImageQueue::IgnoreSourceImage
   */
  virtual void IgnoreSourceImage() = 0;

  /**
   * @copydoc Dali::NativeImageQueue::CanDequeueBuffer
   */
  virtual bool CanDequeueBuffer() = 0;

  /**
   * @copydoc Dali::NativeImageQueue::DequeueBuffer
   */
  virtual uint8_t* DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride, Dali::NativeImageQueue::BufferAccessType type) = 0;

  /**
   * @copydoc Dali::NativeImageQueue::EnqueueBuffer
   */
  virtual bool EnqueueBuffer(uint8_t* buffer) = 0;

  /**
   * @copydoc Dali::NativeImageQueue::CancelDequeuedBuffer
   */
  virtual void CancelDequeuedBuffer(uint8_t* buffer) = 0;

  /**
   * @copydoc Dali::NativeImageQueue::FreeReleasedBuffers
   */
  virtual void FreeReleasedBuffers() = 0;

  /**
   * @copydoc Dali::NativeImageQueue::SetQueueUsageHint
   */
  virtual void SetQueueUsageHint(Dali::NativeImageQueue::QueueUsageType type) = 0;

  /**
   * destructor
   */
  virtual ~NativeImageQueue() = default;

  /**
   * @copydoc Dali::NativeImageInterface::CreateResource()
   */
  virtual bool CreateResource() = 0;

  /**
   * @copydoc Dali::NativeImageInterface::DestroyResource()
   */
  virtual void DestroyResource() = 0;

  /**
   * @copydoc Dali::NativeImageInterface::TargetTexture()
   */
  virtual uint32_t TargetTexture() = 0;

  /**
   * @copydoc Dali::NativeImageInterface::PrepareTexture()
   */
  virtual Dali::NativeImageInterface::PrepareTextureResult PrepareTexture() = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetWidth()
   */
  virtual uint32_t GetWidth() const = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetHeight()
   */
  virtual uint32_t GetHeight() const = 0;

  /**
   * @copydoc Dali::NativeImageInterface::RequiresBlending()
   */
  virtual bool RequiresBlending() const = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetTextureTarget()
   */
  virtual int GetTextureTarget() const = 0;

  /**
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader()
   */
  virtual bool ApplyNativeFragmentShader(std::string& shader, int mask) = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomSamplerTypename()
   */
  virtual const char* GetCustomSamplerTypename() const = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetNativeImageHandle()
   */
  virtual Any GetNativeImageHandle() const = 0;

  /**
   * @copydoc Dali::NativeImageInterface::SourceChanged()
   */
  virtual bool SourceChanged() const = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetUpdatedArea()
   */
  virtual Rect<uint32_t> GetUpdatedArea() = 0;

  /**
   * @copydoc Dali::NativeImageInterface::PostRender()
   */
  virtual void PostRender() = 0;

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  virtual NativeImageInterface::Extension* GetNativeImageInterfaceExtension() = 0;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_QUEUE_IMPL_H
