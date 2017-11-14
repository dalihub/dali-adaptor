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
#include <dali/public-api/images/image-operations.h> // For ImageDimensions
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
   * @param[in] contentScale The scaling factor to apply to the content
   * @param[in] cropToMask Whether to crop the output to the mask size (true) or scale the
   * mask to the content size (false)
   */
  void ApplyMask( const PixelBuffer& mask, float contentScale, bool cropToMask );

  /**
   * Apply a Gaussian blur to the current buffer with the given radius.
   *
   * @param[in] blurRadius The radius for Gaussian blur
   */
  void ApplyGaussianBlur( const float blurRadius );

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
   * Internal method to apply the mask to this buffer. Expects that they are the same size.
   */
  void ApplyMaskInternal( const PixelBuffer& mask );

  /**
   * Takes ownership of the other object's pixel buffer.
   */
  void TakeOwnershipOfBuffer( PixelBuffer& pixelBuffer );

  /**
   * Release the buffer
   */
  void ReleaseBuffer();

  /**
   * Scales this buffer buffer by the given factor, and crops at the center to the
   * given dimensions.
   */
  void ScaleAndCrop( float scaleFactor, ImageDimensions cropDimensions );

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
  static PixelBufferPtr NewCrop( const PixelBuffer& inBuffer, uint16_t x, uint16_t y, ImageDimensions cropDimensions );

  /**
   * Creates a new buffer which is a resized version of the passed in buffer.
   * Uses either Lanczos4 for downscaling, or Mitchell for upscaling.
   * @param[in] inBuffer The source buffer
   * @param[in] outDimensions The new dimensions
   * @return a new buffer of the given size.
   */
  static PixelBufferPtr NewResize( const PixelBuffer& inBuffer, ImageDimensions outDimensions );

  /**
   * Crops this buffer to the given crop rectangle. Assumes the crop rectangle
   * is within the bounds of this size.
   * @param[in] x The top left corner's X
   * @param[in] y The top left corner's y
   * @param[in] cropDimensions The dimensions of the crop
   */
  void Crop( uint16_t x, uint16_t y, ImageDimensions cropDimensions );

  /**
   * Resizes the buffer to the given dimensions. Uses either Lanczos4 for downscaling
   * or Mitchell for upscaling
   * @param[in] outDimensions The new dimensions
   */
  void Resize( ImageDimensions outDimensions );

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
