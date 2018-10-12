#ifndef DALI_PIXEL_BUFFER_H
#define DALI_PIXEL_BUFFER_H

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

// INTERNAL INCLUDES
#include <dali/public-api/images/pixel.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace Internal
{
namespace Adaptor
{
class PixelBuffer;
}
}


// Use namespace to separate from PixelBuffer typedef in buffer-image.h
namespace Devel
{

/**
 * @brief The PixelBuffer object holds a pixel buffer.
 *
 * The PixelBuffer keeps ownership of it's initial buffer however, the
 * user is free to modify the pixel data, either directly, or via
 * image operations.
 *
 * In order to upload the pixel data to texture memory, there are two
 * possibilities - either convert it back to a PixelData object, which
 * releases the PixelBuffer object, leaving the user with an empty handle
 * (ideal for one-time indirect image manipulation), or create a new
 * PixelData object from this object, leaving the buffer intact (ideal
 * for continuous manipulation)
 *
 * @SINCE_1_2.46
 */
class DALI_ADAPTOR_API PixelBuffer : public BaseHandle
{
public:

  /**
   * Create a PixelBuffer with it's own data buffer.
   */
  static PixelBuffer New( unsigned int width,
                          unsigned int height,
                          Dali::Pixel::Format pixelFormat );

  /**
   * @brief Creates an empty handle.
   * Use PixelBuffer::New() to create an initialized object.
   *
   * @SINCE_1_2.46
   */
  PixelBuffer();

  /**
   * @brief Destructor.
   *
   * @SINCE_1_2.46
   */
  ~PixelBuffer();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @SINCE_1_2.46
   * @param[in] handle A reference to the copied handle
   */
  PixelBuffer(const PixelBuffer& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @SINCE_1_2.46
   * @param[in] rhs A reference to the copied handle
   * @return A reference to this object
   */
  PixelBuffer& operator=(const PixelBuffer& rhs);

  /**
   * Convert to a pixel data and release the pixelBuffer's object.
   * This handle is left empty.
   *
   * @warning Any other handles that keep a reference to this object
   * will be left with no buffer, trying to access it will return NULL.
   *
   * @SINCE_1_2.46
   * @param[in,out] pixelBuffer
   * @return a new PixelData which takes ownership of the PixelBuffer's buffer.
   */
  static PixelData Convert( PixelBuffer& pixelBuffer );

  /**
   * Copy the data from this object into a new PixelData object, which could be
   * used for uploading to a texture.
   * @return a new PixelData object containing a copy of this pixel buffer's data.
   */
  Dali::PixelData CreatePixelData() const;

  /**
   * @brief Gets the pixel buffer. This is a pointer to the internal
   * pixel buffer.
   *
   * @warning If there is no pixel buffer (e.g. this object has been
   * converted to a PixelData), this method will return NULL.
   *
   * @SINCE_1_2.46
   * @return The pixel buffer, or NULL.
   */
  unsigned char* GetBuffer();

  /**
   * @brief Gets the width of the buffer in pixels.
   *
   * @SINCE_1_2.46
   * @return The width of the buffer in pixels
   */
  unsigned int GetWidth() const;

  /**
   * @brief Gets the height of the buffer in pixels.
   *
   * @SINCE_1_2.46
   * @return The height of the buffer in pixels
   */
  unsigned int GetHeight() const;

  /**
   * @brief Gets the pixel format.
   *
   * @SINCE_1_2.46
   * @return The pixel format
   */
  Pixel::Format GetPixelFormat() const;

  /**
   * Apply the mask to this pixel data, and return a new pixel data containing
   * the masked image. If this PixelBuffer doesn't have an alpha channel, then
   * the resultant PixelBuffer will be converted to a format that supports at
   * least the width of the color channels and the alpha channel from the mask.
   *
   * If cropToMask is set to true, then the contentScale is applied first to
   * this buffer, and the target buffer is cropped to the size of the mask. If
   * it's set to false, then the mask is scaled to match this buffer's size
   * before the mask is applied.
   *
   * @param[in] mask The mask to apply.
   * @param[in] contentScale The scaling factor to apply to the content
   * @param[in] cropToMask Whether to crop the output to the mask size (true)
   * or scale the mask to the content size (false)
   */
  void ApplyMask( PixelBuffer mask, float contentScale=1.0f, bool cropToMask=false );

  /**
   * Apply a Gaussian blur to this pixel data with the given radius.
   *
   * @note A bigger radius will yield a blurrier image. Only works for pixel data in RGBA format.
   *
   * @param[in] blurRadius The radius for Gaussian blur. A value of 0 or negative value indicates no blur.
   */
  void ApplyGaussianBlur( const float blurRadius );

  /**
   * @brief Crops this buffer to the given crop rectangle.
   *
   * The crop rectangle will be clamped to the edges of the buffer if it is larger.
   * @param[in] x The top left corner's X
   * @param[in] y The top left corner's y
   * @param[in] width The crop width
   * @param[in] height The crop height
   */
  void Crop( uint16_t x, uint16_t y, uint16_t width, uint16_t height );

  /**
   * @brief Resizes the buffer to the given dimensions.
   *
   * Uses either Lanczos4 for downscaling or Mitchell for upscaling
   * @param[in] width The new width
   * @param[in] height The new height
   */
  void Resize( uint16_t width, uint16_t height );

  /**
   * @brief Returns Exif metadata as a property map
   *
   * @param[out] metadata Property map object to write into
   * @return True on success
   */
  bool GetMetadata( Property::Map& metadata ) const;

  /**
   * @brief Multiplies the image's color values by the alpha value. This provides better
   * blending capability.
   */
  void MultiplyColorByAlpha();

  /**
   * @brief Rotates the pixel buffer by the given angle.
   *
   * @note Operation valid for pixel formats: A8, L8, LA88, RGB888, RGB8888, BGR8888, RGBA8888 and BGRA8888. Fails otherwise.
   * @note The operation does nothing for angles equivalent to 0 degrees: -360, 360, 720, etc.
   * @note If the pixel buffer does rotate, all the pointers to the internal pixel buffer retrieved by the method GetPixelBuffer() become invalid.
   *
   * @param[in] angle The angle in degrees.
   *
   * @return @e false if the rotation fails (invalid pixel format or memory issues).
   */
  bool Rotate( Degree angle );

public:

  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   * @SINCE_1_2.46
   * @param[in] pointer A pointer to a newly allocated PixelBuffer
   */
  explicit DALI_INTERNAL PixelBuffer( Internal::Adaptor::PixelBuffer* pointer );
};

} // namespace Devel
} // namespace Dali

#endif // DALI_PIXEL_BUFFER_H
