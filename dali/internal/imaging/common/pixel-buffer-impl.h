#ifndef DALI_INTERNAL_ADAPTOR_PIXEL_BUFFER_H
#define DALI_INTERNAL_ADAPTOR_PIXEL_BUFFER_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/image-operations.h> // For ImageDimensions
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/property-map.h>

// EXTERNAL INCLUDES
#include <memory>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class PixelBuffer;
typedef IntrusivePtr<PixelBuffer> PixelBufferPtr;

class PixelBuffer : public BaseObject
{
public:
  /**
   * @brief Create a PixelBuffer object with a pre-allocated buffer.
   * The PixelBuffer object owns this buffer, which may be retrieved
   * and modified using GetBuffer().
   *
   * @param [in] width            Buffer width in pixels
   * @param [in] height           Buffer height in pixels
   * @param [in] pixelFormat      The pixel format
   */
  static PixelBufferPtr New(uint32_t      width,
                            uint32_t      height,
                            Pixel::Format pixelFormat);

  /**
   * @brief Create a PixelBuffer object. For internal use only.
   *
   * @param [in] buffer           The raw pixel data.
   * @param [in] bufferSize       The size of the buffer in bytes
   * @param [in] width            Buffer width in pixels
   * @param [in] height           Buffer height in pixels
   * @param [in] stride           Buffer stride in pixels, 0 means the buffer is tightly packed
   * @param [in] pixelFormat      The pixel format
   */
  static PixelBufferPtr New(uint8_t*      buffer,
                            uint32_t      bufferSize,
                            uint32_t      width,
                            uint32_t      height,
                            uint32_t      stride,
                            Pixel::Format pixelFormat);

  /**
   * Convert a pixelBuffer object into a PixelData object.
   * The new object takes ownership of the buffer data, and the
   * mBuffer pointer is reset to NULL.
   * @param[in] pixelBuffer The buffer to convert
   * @param[in] releaseAfterUpload Whether converted PixelData released after upload or not.
   * @return the pixelData
   */
  static Dali::PixelData Convert(PixelBuffer& pixelBuffer, bool releaseAfterUpload);

  /**
   * @brief Constructor.
   *
   * @param [in] buffer           The raw pixel data.
   * @param [in] bufferSize       The size of the buffer in bytes
   * @param [in] width            Buffer width in pixels
   * @param [in] height           Buffer height in pixels
   * @param [in] stride           Buffer stride in pixels, 0 means the buffer is tightly packed
   * @param [in] pixelFormat      The pixel format
   */
  PixelBuffer(uint8_t*      buffer,
              uint32_t      bufferSize,
              uint32_t      width,
              uint32_t      height,
              uint32_t      stride,
              Pixel::Format pixelFormat);

protected:
  /**
   * @brief Destructor.
   *
   * Release the pixel buffer if exists.
   */
  ~PixelBuffer() override;

public:
  /**
   * Get the total allocated size of current pixel buffers
   */
  static uint32_t GetTotalAllocatedSize()
  {
#if defined(DEBUG_ENABLED)
    return gPixelBufferAllocationTotal;
#else
    return 0;
#endif
  }

  /**
   * Get the width of the buffer in pixels.
   * @return The width of the buffer in pixels
   */
  uint32_t GetWidth() const;

  /**
   * Get the height of the buffer in pixels
   * @return The height of the buffer in pixels
   */
  uint32_t GetHeight() const;

  /**
   * @brief Gets the stride of the buffer in pixels.
   * @return The stride of the buffer in pixels. 0 means the buffer is tightly packed.
   */
  uint32_t GetStride() const;

  /**
   * @brief Gets the stride of the buffer in bytes.
   * @return The stride of the buffer in bytes. 0 means the buffer is tightly packed.
   */
  uint32_t GetStrideBytes() const;

  /**
   * Get the pixel format
   * @return The pixel format
   */
  Pixel::Format GetPixelFormat() const;

  /**
   * Get the pixel buffer if it's present.
   * @return The buffer if exists, or NULL if there is no pixel buffer.
   */
  uint8_t* GetBuffer() const;

  /**
   * @copydoc Devel::PixelBuffer::GetBuffer()
   */
  const uint8_t* GetConstBuffer() const;

  /**
   * Get the size of the buffer in bytes
   * @return The size of the buffer
   */
  uint32_t GetBufferSize() const;

  /**
   * Copy the buffer into a new PixelData
   */
  Dali::PixelData CreatePixelData() const;

  /**
   * @brief Apply the mask to the current buffer.
   *
   * This method may update the internal object - e.g. the new buffer
   * may have a different pixel format - as an alpha channel may be
   * added.
   * @param[in] mask The mask to apply to this pixel buffer
   * @param[in] contentScale The scaling factor to apply to the content
   * @param[in] cropToMask Whether to crop the output to the mask size (true) or scale the
   * mask to the content size (false)
   */
  void ApplyMask(const PixelBuffer& mask, float contentScale, bool cropToMask);

  /**
   * @brief Apply a Gaussian blur to the current buffer with the given radius.
   *
   * @param[in] blurRadius The radius for Gaussian blur
   */
  void ApplyGaussianBlur(const float blurRadius);

  /**
   * Crops this buffer to the given crop rectangle. Assumes the crop rectangle
   * is within the bounds of this size.
   * @param[in] x The top left corner's X
   * @param[in] y The top left corner's y
   * @param[in] cropDimensions The dimensions of the crop
   */
  void Crop(uint16_t x, uint16_t y, ImageDimensions cropDimensions);

  /**
   * Resizes the buffer to the given dimensions. Uses either Lanczos4 for downscaling
   * or Mitchell for upscaling
   * @param[in] outDimensions The new dimensions
   */
  void Resize(ImageDimensions outDimensions);

  /**
   * @copydoc Devel::PixelBuffer::MultiplyColorByAlpha()
   */
  void MultiplyColorByAlpha();

  /**
   * @brief Sets image metadata
   *
   * @param map Property map containing Exif fields
   */
  void SetMetadata(const Property::Map& map);

  /**
   * @brief Returns image metadata as a property map
   * @param[out] outMetadata Property map to copy the data into
   * @return True on success
   */
  bool GetMetadata(Property::Map& outMetadata) const;

  /**
   * @brief Sets metadata property map for the pixel buffer
   * @note The function takes over the ownership of the property map
   * @param[in] metadata Property map to copy the data into
   */
  void SetMetadata(std::unique_ptr<Property::Map> metadata);

  /**
   * Allocates fixed amount of memory for the pixel data. Used by compressed formats.
   * @param[in] size Size of memory to be allocated
   */
  void AllocateFixedSize(uint32_t size);

  /**
   * @copydoc Devel::PixelBuffer::Rotate()
   */
  bool Rotate(Degree angle);

  /**
   * @copydoc Devel::PixelBuffer::IsAlphaPreMultiplied()
   */
  bool IsAlphaPreMultiplied() const;

  /**
   * @copydoc Devel::PixelBuffer::GetBrightness()
   */
  uint32_t GetBrightness() const;

private:
  /*
   * Undefined copy constructor.
   */
  PixelBuffer(const PixelBuffer& other);

  /*
   * Undefined assignment operator.
   */
  PixelBuffer& operator=(const PixelBuffer& other);

  /**
   * Internal method to apply the mask to this buffer. Expects that they are the same size.
   */
  void ApplyMaskInternal(const PixelBuffer& mask);

  /**
   * Takes ownership of the other object's pixel buffer.
   */
  void TakeOwnershipOfBuffer(PixelBuffer& pixelBuffer);

  /**
   * Release the buffer
   */
  void ReleaseBuffer();

  /**
   * Scales this buffer buffer by the given factor, and crops at the center to the
   * given dimensions.
   */
  void ScaleAndCrop(float scaleFactor, ImageDimensions cropDimensions);

  /**
   * Creates a new buffer which is a crop of the passed in buffer,
   * using the given crop rectangle. Assumes the crop rectangle is
   * within the bounds of this size.
   * @param[in] inBuffer The source buffer
   * @param[in] x The top left corner's X
   * @param[in] y The top left corner's y
   * @param[in] cropDimensions The dimensions of the crop
   * @return the new pixel buffer
   */
  static PixelBufferPtr NewCrop(const PixelBuffer& inBuffer, uint16_t x, uint16_t y, ImageDimensions cropDimensions);

  /**
   * Creates a new buffer which is a resized version of the passed in buffer.
   * Uses either Lanczos4 for downscaling, or Mitchell for upscaling.
   * @param[in] inBuffer The source buffer
   * @param[in] outDimensions The new dimensions
   * @return a new buffer of the given size.
   */
  static PixelBufferPtr NewResize(const PixelBuffer& inBuffer, ImageDimensions outDimensions);

private:
  std::unique_ptr<Property::Map> mMetadata; ///< Metadata fields

  uint8_t*      mBuffer;      ///< The raw pixel data
  uint32_t      mBufferSize;  ///< Buffer sized in bytes
  uint32_t      mWidth;       ///< Buffer width in pixels
  uint32_t      mHeight;      ///< Buffer height in pixels
  uint32_t      mStrideBytes; ///< Buffer stride in bytes, 0 means the buffer is tightly packed
  Pixel::Format mPixelFormat; ///< Pixel format

  bool mPreMultiplied : 1; ///< PreMultiplied

#if defined(DEBUG_ENABLED)
  static uint32_t gPixelBufferAllocationTotal;
#endif
};

} // namespace Adaptor

} // namespace Internal

/**
 * Helper methods for public API
 */
inline Internal::Adaptor::PixelBuffer& GetImplementation(Devel::PixelBuffer& handle)
{
  DALI_ASSERT_ALWAYS(handle && "handle is empty");

  BaseObject& object = handle.GetBaseObject();

  return static_cast<Internal::Adaptor::PixelBuffer&>(object);
}

inline const Internal::Adaptor::PixelBuffer& GetImplementation(const Devel::PixelBuffer& handle)
{
  DALI_ASSERT_ALWAYS(handle && "handle is empty");

  const BaseObject& object = handle.GetBaseObject();

  return static_cast<const Internal::Adaptor::PixelBuffer&>(object);
}

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_PIXEL_BUFFER_H
