#ifndef DALI_PIXEL_BUFFER_H
#define DALI_PIXEL_BUFFER_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/public-api/images/pixel.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/object/base-handle.h>

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
class DALI_IMPORT_API PixelBuffer : public BaseHandle
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
   * Apply the mask to this pixel data, and return a new pixel data
   * containing the masked image. If this PixelBuffer doesn't have an alpha channel,
   * then the resultant PixelBuffer will be converted to a format that
   * supports at least the width of the color channels and the alpha channel
   * from the mask.
   * @param[in] mask The mask to apply.
   */
  void ApplyMask( PixelBuffer mask );

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
