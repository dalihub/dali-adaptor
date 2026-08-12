#ifndef DALI_PIXEL_BUFFER_H
#define DALI_PIXEL_BUFFER_H

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
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/images/pixel.h>
#include <dali/public-api/object/base-handle.h>

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
class PixelBuffer;
}
} // namespace Internal DALI_INTERNAL

/**
 * @brief The PixelBuffer object holds a pixel buffer that the application can read and write.
 *
 * Unlike PixelData, which keeps its pixels opaque, PixelBuffer hands out a pointer to
 * its buffer through GetBuffer(), so the application is free to fill or edit the pixels
 * directly. The buffer is owned by the PixelBuffer object and released with it.
 *
 * A PixelBuffer is either created empty with New(), or obtained already filled from an
 * image loading API such as LoadImageFromFile().
 *
 * When the pixels are ready to be used as a texture, Convert() turns this object into a
 * PixelData without copying the buffer. That leaves this handle empty, so convert only
 * once the editing is done.
 *
 * @SINCE_2_5.35
 */
class DALI_ADAPTOR_API PixelBuffer : public BaseHandle
{
public:
  /**
   * @brief Creates a PixelBuffer with its own buffer.
   *
   * @note The contents of the buffer are undefined until the application writes into it.
   *
   * @SINCE_2_5.35
   * @param[in] width The buffer width in pixels
   * @param[in] height The buffer height in pixels
   * @param[in] pixelFormat The pixel format
   * @return A handle to a newly allocated PixelBuffer
   */
  static PixelBuffer New(uint32_t            width,
                         uint32_t            height,
                         Dali::Pixel::Format pixelFormat);

  /**
   * @brief Creates an empty handle.
   *
   * Use PixelBuffer::New() to create an initialized object.
   *
   * @SINCE_2_5.35
   */
  PixelBuffer();

  /**
   * @brief Destructor.
   *
   * @SINCE_2_5.35
   */
  ~PixelBuffer();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @SINCE_2_5.35
   * @param[in] handle A reference to the copied handle
   */
  PixelBuffer(const PixelBuffer& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @SINCE_2_5.35
   * @param[in] rhs A reference to the copied handle
   * @return A reference to this object
   */
  PixelBuffer& operator=(const PixelBuffer& rhs);

  /**
   * @brief This move constructor is required for (smart) pointer semantics.
   *
   * @SINCE_2_5.35
   * @param[in] handle A reference to the moved handle
   */
  PixelBuffer(PixelBuffer&& handle) noexcept;

  /**
   * @brief This move assignment operator is required for (smart) pointer semantics.
   *
   * @SINCE_2_5.35
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this object
   */
  PixelBuffer& operator=(PixelBuffer&& rhs) noexcept;

  /**
   * @brief Converts a PixelBuffer into a PixelData, which can be uploaded to a texture.
   *
   * The buffer is handed over to the new PixelData without being copied, and the given
   * handle is reset to an empty handle.
   *
   * @warning Any other handle that keeps a reference to the same object is left with no
   * buffer. GetBuffer() returns NULL for such a handle, and the sizes it reports are 0.
   *
   * @SINCE_2_5.35
   * @param[in,out] pixelBuffer The pixel buffer to convert. Reset to an empty handle on return
   * @return A new PixelData which owns the pixel buffer's buffer
   */
  static PixelData Convert(PixelBuffer& pixelBuffer);

  /**
   * @brief Gets the buffer. This is a pointer to the internal pixel buffer.
   *
   * The pixels are laid out row by row, each row being GetStrideBytes() bytes long, so
   * use GetStrideBytes() rather than the width to step from one row to the next.
   *
   * @warning The returned pointer becomes invalid when the buffer is replaced or released,
   * that is when Convert() or Resize() is called on this object. There is no pixel buffer
   * left after Convert(), and this method returns NULL in that case.
   *
   * @SINCE_2_5.35
   * @return The buffer, or NULL if there is no pixel buffer
   */
  uint8_t* GetBuffer();

  /**
   * @brief Gets the buffer. This is a pointer to the internal pixel buffer.
   *
   * @warning The returned pointer becomes invalid when the buffer is replaced or released,
   * that is when Convert() or Resize() is called on this object. There is no pixel buffer
   * left after Convert(), and this method returns NULL in that case.
   *
   * @SINCE_2_5.35
   * @return The buffer, or NULL if there is no pixel buffer
   */
  const uint8_t* GetBuffer() const;

  /**
   * @brief Gets the size of the buffer in bytes.
   *
   * This is the size of the whole allocation that GetBuffer() points at, so writing
   * within this many bytes is safe.
   *
   * @SINCE_2_5.35
   * @return The size of the buffer in bytes, or 0 if there is no pixel buffer
   */
  uint32_t GetBufferSize() const;

  /**
   * @brief Gets the width of the buffer in pixels.
   *
   * @SINCE_2_5.35
   * @return The width of the buffer in pixels
   */
  uint32_t GetWidth() const;

  /**
   * @brief Gets the height of the buffer in pixels.
   *
   * @SINCE_2_5.35
   * @return The height of the buffer in pixels
   */
  uint32_t GetHeight() const;

  /**
   * @brief Gets the stride of the buffer in bytes.
   *
   * The stride is the number of bytes from the start of one row of pixels to the start of
   * the next, which can be larger than the width in bytes. A tightly packed buffer reports
   * the width in bytes rather than 0.
   *
   * @SINCE_2_5.35
   * @return The stride of the buffer in bytes, or 0 if there is no pixel buffer
   */
  uint32_t GetStrideBytes() const;

  /**
   * @brief Gets the pixel format.
   *
   * @SINCE_2_5.35
   * @return The pixel format
   */
  Pixel::Format GetPixelFormat() const;

  /**
   * @brief Applies a Gaussian blur to this pixel buffer with the given radius.
   *
   * The blur is applied in place, so the pointer returned by GetBuffer() and the
   * dimensions of this object stay valid.
   *
   * @note A bigger radius will yield a blurrier image.
   * @note A radius of 0 does nothing and succeeds. A negative radius does nothing
   * and fails.
   * @note Valid for the pixel formats A8, L8, LA88, RGB888, RGB8888, BGR8888,
   * RGBA8888, BGRA8888, CHROMINANCE_U and CHROMINANCE_V. Fails otherwise.
   *
   * @SINCE_2_5.35
   * @param[in] blurRadius The radius for the Gaussian blur
   * @return @c false if the Gaussian blur fails (invalid radius, invalid pixel format,
   * empty buffer or memory issues)
   */
  bool ApplyGaussianBlur(float blurRadius);

  /**
   * @brief Resizes the buffer to the given dimensions.
   *
   * Uses either Lanczos4 for downscaling or Mitchell for upscaling.
   *
   * @warning The buffer is replaced, so the pointer returned by a previous GetBuffer()
   * call becomes invalid. Call GetBuffer() again after resizing.
   *
   * @note Resizing to the current dimensions does nothing and succeeds.
   * @note Valid for the pixel formats A8, L8, LA88, RGB888, RGB8888, BGR8888,
   * RGBA8888, BGRA8888, CHROMINANCE_U and CHROMINANCE_V. Fails otherwise.
   *
   * @SINCE_2_5.35
   * @param[in] width The new width in pixels
   * @param[in] height The new height in pixels
   * @return @c false if the resize fails (invalid dimensions, invalid pixel format,
   * empty buffer or memory issues)
   */
  bool Resize(uint16_t width, uint16_t height);

public:
  /// @cond internal
  /**
   * @brief The constructor.
   *
   * @note  Not intended for application developers.
   * @SINCE_2_5.35
   * @param[in] pointer A pointer to a newly allocated PixelBuffer
   */
  explicit DALI_INTERNAL PixelBuffer(Internal::Adaptor::PixelBuffer* pointer);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_PIXEL_BUFFER_H
