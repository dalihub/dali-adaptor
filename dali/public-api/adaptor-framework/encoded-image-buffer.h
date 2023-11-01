#ifndef DALI_ENCODED_IMAGE_BUFFER_H
#define DALI_ENCODED_IMAGE_BUFFER_H
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
 *
 */

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/object/base-handle.h>

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
 * @SINCE_2_0.34
 * @note Object will copy raw buffer data.
 */
class DALI_ADAPTOR_API EncodedImageBuffer : public BaseHandle
{
public:
  using RawBufferType = Dali::Vector<uint8_t>;

  /**
   * @brief The list of type of encoded image buffer.
   *
   * @SINCE_2_2.51
   */
  enum class ImageType
  {
    REGULAR_IMAGE,
    VECTOR_IMAGE, ///< svg format.
    ANIMATED_VECTOR_IMAGE, ///< lottie format.

    DEFAULT = REGULAR_IMAGE,
  };

public:
  /**
   * @brief Create a new EncodedImageBuffer. ImageType will be setted as DEFAULT.
   *
   * @SINCE_2_0.34
   * @param [in] buffer The encoded raw buffer
   * @return A handle to a new EncodedImageBuffer.
   */
  static EncodedImageBuffer New(const RawBufferType& buffer);

  /**
   * @brief Create a new EncodedImageBuffer with ImageType.
   *
   * @SINCE_2_2.51
   * @param [in] buffer The encoded raw buffer
   * @param [in] type The type hint of encoded raw buffer
   * @return A handle to a new EncodedImageBuffer.
   */
  static EncodedImageBuffer New(const RawBufferType& buffer, ImageType type);

  /**
   * @brief Create an empty handle.
   *
   * @SINCE_2_0.34
   * Calling member functions of an empty handle is not allowed.
   */
  EncodedImageBuffer();

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @SINCE_2_0.34
   */
  ~EncodedImageBuffer();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @SINCE_2_0.34
   * @param [in] handle A reference to the copied handle
   */
  EncodedImageBuffer(const EncodedImageBuffer& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @SINCE_2_0.34
   * @param [in] handle  A reference to the copied handle
   * @return A reference to this
   */
  EncodedImageBuffer& operator=(const EncodedImageBuffer& handle);

  /**
   * @brief This move constructor is required for (smart) pointer semantics.
   *
   * @SINCE_2_0.34
   * @param [in] handle A reference to the moved handle
   */
  EncodedImageBuffer(EncodedImageBuffer&& handle) noexcept;

  /**
   * @brief This move assignment operator is required for (smart) pointer semantics.
   *
   * @SINCE_2_0.34
   * @param [in] handle  A reference to the moved handle
   * @return A reference to this
   */
  EncodedImageBuffer& operator=(EncodedImageBuffer&& handle) noexcept;

  /**
   * @brief Get raw buffer data
   *
   * @SINCE_2_0.34
   * @note this method return const value. Mean, you cannot change raw buffer
   * @return A RawBufferType this buffer have
   */
  const RawBufferType& GetRawBuffer() const;

  /**
   * @brief Get the hash value of raw buffer
   *
   * @SINCE_2_1.20
   * @return A hash value of raw buffer.
   */
  std::size_t GetHash() const;

  /**
   * @brief Set type of raw buffer.
   *
   * @SINCE_2_2.51
   * @param[in] type A ImageType for this buffer
   */
  void SetImageType(ImageType type);

  /**
   * @brief Get type of raw buffer.
   *
   * @SINCE_2_2.51
   * @return A ImageType this buffer have
   */
  ImageType GetImageType() const;

public: // Not intended for developer use
  explicit DALI_INTERNAL EncodedImageBuffer(Internal::EncodedImageBuffer* impl);
};

} // namespace Dali

#endif // DALI_ENCODED_IMAGE_BUFFER_H
