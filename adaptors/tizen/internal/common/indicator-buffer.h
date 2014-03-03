#ifndef __DALI_INTERNAL_INDICATOR_BUFFER_H__
#define __DALI_INTERNAL_INDICATOR_BUFFER_H__

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/ref-object.h>

// INTERNAL INCLUDES
#include <internal/common/native-bitmap-buffer-impl.h>
#include <internal/common/adaptor-impl.h>

namespace Dali
{

namespace Internal
{
namespace Adaptor
{

class NativeBitmapBuffer;
class IndicatorBuffer;

typedef IntrusivePtr<IndicatorBuffer> IndicatorBufferPtr;

/**
 * The IndicatorBuffer class uses the best available implementation for rendering indicator data.
 * On platforms where EglImage is available it uses either SharedGlBuffer or PixmapImage, on older
 * platforms it falls back to using a bitmap buffer based solution.
 */
class IndicatorBuffer : public RefObject
{
public:

  /**
   * Constructor
   */
  IndicatorBuffer( Adaptor* adaptor, unsigned int width, unsigned int height, Pixel::Format pixelFormat );

  /**
   * Copy bitmap data to pixel buffer.
   * @param src  bitmap data source
   * @param size size of bitmap data
   * @return true if successful, false otherwise
   */
  bool UpdatePixels( const unsigned char *src, size_t size );

  /**
   * Returns the NativeImage used internally
   * @return the NativeImage used internally
   */
  NativeImage& GetNativeImage() const;

  /**
   * Set currently used Adaptor
   * @param adaptor
   */
  void SetAdaptor( Adaptor* adaptor );

private:
  NativeImagePtr mNativeImage; ///< Image buffer created for shared file copy

  NativeBitmapBufferPtr mBitmapBuffer;    ///< Image buffer created for shared file copy if extension not available

  Adaptor*      mAdaptor;

  int           mImageWidth;
  int           mImageHeight;
  Pixel::Format mPixelFormat;

  // Only used with fallback bitmap buffer implementation
  bool  mUpdatingBitmap:1;      ///< Whether BitmapImage is being uploaded to graphics memory
  bool  mUpdateBitmapAgain:1;   ///< Whether to update BitmapImage again after upload complete
};

} // Adaptor
} // Internal
} // Dali

#endif // __DALI_INTERNAL_INDICATOR_H__
