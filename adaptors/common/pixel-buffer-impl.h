#ifndef DALI_INTERNAL_ADAPTOR_PIXEL_BUFFER_H
#define DALI_INTERNAL_ADAPTOR_PIXEL_BUFFER_H

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

// INTERNAL INCLUDES
#include <pixel-buffer.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/object/base-object.h>

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
  static PixelBufferPtr New( unsigned int width,
                             unsigned int height,
                             Pixel::Format pixelFormat );

  /**
   * @brief Create a PixelBuffer object. For internal use only.
   *
   * @param [in] buffer           The raw pixel data.
   * @param [in] bufferSize       The size of the buffer in bytes
   * @param [in] width            Buffer width in pixels
   * @param [in] height           Buffer height in pixels
   * @param [in] pixelFormat      The pixel format
   * @param [in] releaseFunction  The function used to release the memory.
   */
  static PixelBufferPtr New( unsigned char* buffer,
                             unsigned int bufferSize,
                             unsigned int width,
                             unsigned int height,
                             Pixel::Format pixelFormat );

  /**
   * Convert a pixelBuffer object into a PixelData object.
   * The new object takes ownership of the buffer data, and the
   * mBuffer pointer is reset to NULL.
   * @param[in] pixelBuffer The buffer to convert
   * @return the pixelData
   */
  static Dali::PixelData Convert( PixelBuffer& pixelBuffer );

  /**
   * @brief Constructor.
   *
   * @param [in] buffer           The raw pixel data.
   * @param [in] bufferSize       The size of the buffer in bytes
   * @param [in] width            Buffer width in pixels
   * @param [in] height           Buffer height in pixels
   * @param [in] pixelFormat      The pixel format
   */
  PixelBuffer( unsigned char* buffer,
               unsigned int bufferSize,
               unsigned int width,
               unsigned int height,
               Pixel::Format pixelFormat );

protected:

  /**
   * @brief Destructor.
   *
   * Release the pixel buffer if exists.
   */
  ~PixelBuffer();

public:

  /**
   * Get the width of the buffer in pixels.
   * @return The width of the buffer in pixels
   */
  unsigned int GetWidth() const;

  /**
   * Get the height of the buffer in pixels
   * @return The height of the buffer in pixels
   */
  unsigned int GetHeight() const;

  /**
   * Get the pixel format
   * @return The pixel format
   */
  Pixel::Format GetPixelFormat() const;

  /**
   * Get the pixel buffer if it's present.
   * @return The buffer if exists, or NULL if there is no pixel buffer.
   */
  unsigned char* GetBuffer() const;

  /**
   * Get the size of the buffer in bytes
   * @return The size of the buffer
   */
  unsigned int GetBufferSize() const;

  /**
   * Copy the buffer into a new PixelData
   */
  Dali::PixelData CreatePixelData() const;

  /**
   * Apply the mask to the current buffer. This method may update the
   * internal object - e.g. the new buffer may have a different pixel
   * format - as an alpha channel may be added.
   * @param[in] mask The mask to apply to this pixel buffer
   */
  void ApplyMask( const PixelBuffer& mask );

private:
  /*
   * Undefined copy constructor.
   */
  PixelBuffer(const PixelBuffer& other);

  /*
   * Undefined assignment operator.
   */
  PixelBuffer& operator= (const PixelBuffer& other);

  /**
   * Release the buffer
   */
  void ReleaseBuffer();

private:

  unsigned char* mBuffer;           ///< The raw pixel data
  unsigned int   mBufferSize;       ///< Buffer sized in bytes
  unsigned int   mWidth;            ///< Buffer width in pixels
  unsigned int   mHeight;           ///< Buffer height in pixels
  Pixel::Format  mPixelFormat;      ///< Pixel format
};

} // namespace Adaptor

} // namespace Internal

/**
 * Helper methods for public API
 */
inline Internal::Adaptor::PixelBuffer& GetImplementation( Devel::PixelBuffer& handle )
{
  DALI_ASSERT_ALWAYS( handle && "handle is empty" );

  BaseObject& object = handle.GetBaseObject();

  return static_cast<Internal::Adaptor::PixelBuffer&>( object );
}

inline const Internal::Adaptor::PixelBuffer& GetImplementation( const Devel::PixelBuffer& handle )
{
  DALI_ASSERT_ALWAYS( handle && "handle is empty" );

  const BaseObject& object = handle.GetBaseObject();

  return static_cast<const Internal::Adaptor::PixelBuffer&>( object );
}

} // namespace Dali

#endif // __DALI_INTERNAL_ADAPTOR_PIXEL_BUFFER_H__
