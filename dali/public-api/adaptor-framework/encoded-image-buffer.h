#ifndef DALI_ENCODED_IMAGE_BUFFER_H
#define DALI_ENCODED_IMAGE_BUFFER_H
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
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/common/dali-vector.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
class EncodedImageBuffer;
}

/**
 * @brief EncodedImageBuffer contains the large encoded raw buffer informations.
 *
 * We can upload this buffer by generated url.
 * The images are loaded by a worker thread to avoid blocking the main event thread.
 * @note cannot change raw buffer after make handle.
 * @note only regular image upload support now.
 */
class DALI_ADAPTOR_API EncodedImageBuffer : public BaseHandle
{
public:
  using RawBufferType = Dali::Vector<uint8_t>;

public:
  /**
   * @brief Create a new EncodedImageBuffer.
   *
   * @param [in] buffer The encoded raw buffer
   * @return A handle to a new EncodedImageBuffer.
   */
  static EncodedImageBuffer New(const RawBufferType& buffer);

  /**
   * @brief Create an empty handle.
   *
   * Calling member functions of an empty handle is not allowed.
   */
  EncodedImageBuffer();

  /**
   * @brief Destructor.
   */
  ~EncodedImageBuffer();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param [in] handle A reference to the copied handle
   */
  EncodedImageBuffer(const EncodedImageBuffer& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] handle  A reference to the copied handle
   * @return A reference to this
   */
  EncodedImageBuffer& operator=(const EncodedImageBuffer& handle);

  /**
   * @brief Get raw buffer data
   * @note this method return const value. Mean, you cannot change raw buffer
   * @return A RawBufferType this buffer have
   */
  const RawBufferType& GetRawBuffer() const;

public: // Not intended for developer use
  explicit DALI_INTERNAL EncodedImageBuffer(Internal::EncodedImageBuffer* impl);
};

} // namespace Dali

#endif // DALI_ENCODED_IMAGE_BUFFER_H
