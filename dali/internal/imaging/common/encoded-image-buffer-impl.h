#ifndef DALI_ENCODED_IMAGE_BUFFER_IMPL_H
#define DALI_ENCODED_IMAGE_BUFFER_IMPL_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
 */

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/encoded-image-buffer.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
class EncodedImageBuffer : public BaseObject
{
public:
  using RawBufferType = Dali::EncodedImageBuffer::RawBufferType;
  using ImageType     = Dali::EncodedImageBuffer::ImageType;

  /**
   * @copydoc Dali::EncodedImageBuffer::New
   */
  static IntrusivePtr<EncodedImageBuffer> New(const RawBufferType& buffer, ImageType type);

  /**
   * @copydoc Dali::EncodedImageBuffer::New
   */
  static IntrusivePtr<EncodedImageBuffer> New(RawBufferType&& buffer, ImageType type);

  /**
   * @copydoc Dali::EncodedImageBuffer::GetRawBuffer
   */
  const RawBufferType& GetRawBuffer() const;

  /**
   * @copydoc Dali::EncodedImageBuffer::GetHash
   */
  std::size_t GetHash() const;

  /**
   * @copydoc Dali::EncodedImageBuffer::SetImageType
   */
  void SetImageType(ImageType type);

  /**
   * @copydoc Dali::EncodedImageBuffer::GetImageType
   */
  ImageType GetImageType() const;

protected:
  /**
   * Constructor
   * @param[in] buffer The raw buffer of image.
   * @param[in] type The type of image.
   */
  EncodedImageBuffer(const RawBufferType& buffer, ImageType type);

  /**
   * Constructor as moved buffer
   * @param[in] buffer The raw buffer of image as moved.
   * @param[in] type The type of image.
   */
  EncodedImageBuffer(RawBufferType&& buffer, ImageType type);

  /**
   * Destructor
   */
  ~EncodedImageBuffer();

private:
  // Undefined
  EncodedImageBuffer(const EncodedImageBuffer& imageBuffer);

  // Undefined
  EncodedImageBuffer& operator=(const EncodedImageBuffer& imageBuffer);

private:
  Dali::Vector<uint8_t> mBuffer;
  std::size_t           mBufferHash;
  ImageType             mType;
};

} // namespace Internal

inline const Internal::EncodedImageBuffer& GetImplementation(const Dali::EncodedImageBuffer& encodedImageBuffer)
{
  DALI_ASSERT_ALWAYS(encodedImageBuffer && "EncodedImageBuffer handle is empty");

  const BaseObject& handle = encodedImageBuffer.GetBaseObject();

  return static_cast<const Internal::EncodedImageBuffer&>(handle);
}

inline Internal::EncodedImageBuffer& GetImplementation(Dali::EncodedImageBuffer& encodedImageBuffer)
{
  DALI_ASSERT_ALWAYS(encodedImageBuffer && "EncodedImageBuffer handle is empty");

  BaseObject& handle = encodedImageBuffer.GetBaseObject();

  return static_cast<Internal::EncodedImageBuffer&>(handle);
}

} // namespace Dali

#endif // DALI_ENCODED_IMAGE_BUFFER_IMPL_H
