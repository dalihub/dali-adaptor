#ifndef __DALI_NATIVE_BITMAP_BUFFER_H__
#define __DALI_NATIVE_BITMAP_BUFFER_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// EXTERNAL HEADERS
#include <dali/public-api/images/native-image.h>
#include <dali/public-api/images/pixel.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/lockless-buffer.h>
#include <dali/public-api/common/dali-vector.h>

// INTERNAL HEADERS
#include <internal/common/adaptor-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class NativeBitmapBuffer;
typedef IntrusivePtr<NativeBitmapBuffer> NativeBitmapBufferPtr;

/**
 * A Bitmap-based implementation of the NativeImage interface.
 */
class NativeBitmapBuffer : public NativeImage
{

public:
  /**
   * Constructor.
   * @param adaptor Adaptor used
   * @param width width of image
   * @param height height of image
   * @param pixelFormat pixel format for image
   */
  NativeBitmapBuffer( Adaptor* adaptor, unsigned int width, unsigned int height, Pixel::Format pixelFormat );

  /**
   * virtual destructor
   */
  virtual ~NativeBitmapBuffer();

  /**
   * Write to buffer. Does not block.
   * @param[in] src  data source
   * @param[in] size size of data in bytes
   * @return true if successful, false if currently reading from buffer in render thread
   */
  void Write( const unsigned char* src, size_t size );

public:
  /**
   * @copydoc Dali::NativeImage::GlExtensionCreate()
   */
  virtual bool GlExtensionCreate();

  /**
   * @copydoc Dali::NativeImage::GlExtensionDestroy()
   */
  virtual void GlExtensionDestroy();

  /**
   * @copydoc Dali::NativeImage::TargetTexture()
   */
  virtual unsigned int TargetTexture();

  /**
   * @copydoc Dali::NativeImage::PrepareTexture()
   */
  virtual void PrepareTexture();

  /**
   * @copydoc Dali::NativeImage::GetWidth()
   */
  virtual unsigned int GetWidth() const;

  /**
   * @copydoc Dali::NativeImage::GetHeight()
   */
  virtual unsigned int GetHeight() const;

  /**
   * @copydoc Dali::NativeImage::GetPixelFormat()
   */
  virtual Pixel::Format GetPixelFormat() const;

private:
  NativeBitmapBuffer( const NativeBitmapBuffer& );             ///< not defined
  NativeBitmapBuffer& operator =( const NativeBitmapBuffer& ); ///< not defined
  NativeBitmapBuffer(); ///< not defined

private:
  Integration::GlAbstraction*  mGlAbstraction; ///< GlAbstraction used

  Integration::LocklessBuffer* mBuffer;        ///< bitmap data double buffered
  unsigned int                 mWidth;         ///< Image width
  unsigned int                 mHeight;        ///< Image height
  Pixel::Format                mPixelFormat;   ///< Image pixelformat
  const unsigned char*         mLastReadBuffer; ///< last buffer that was read
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_NATIVE_BITMAP_BUFFER_H__
