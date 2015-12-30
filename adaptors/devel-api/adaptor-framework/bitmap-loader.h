#ifndef __DALI_BITMAP_LOADER_H__
#define __DALI_BITMAP_LOADER_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
 */

// EXTERNAL INCLUDES
#include <string>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/images/pixel.h>
#include <dali/public-api/object/base-handle.h>

namespace Dali
{
namespace Internal
{
class BitmapLoader;
}

class DALI_IMPORT_API BitmapLoader : public BaseHandle
{
public:
  /**
   * @brief Create an initialized bitmap loader. This will automatically load the image.
   *
   * @param[in] filename  Filename of the bitmap image to load.
   */
  static BitmapLoader New(const std::string& filename);

  /**
   * @brief Create an empty handle.
   *
   * Use BitmapLoader::New() to create an initialized object.
   */
  BitmapLoader();

  /**
   * Destructor
   */
  ~BitmapLoader();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param [in] handle A reference to the copied handle
   */
  BitmapLoader(const BitmapLoader& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] rhs  A reference to the copied handle
   * @return A reference to this
   */
  BitmapLoader& operator=(const BitmapLoader& rhs);

public:

  /**
   * Get the raw pixel data.
   * @return The pixel data. Use the GetHeight(), GetWidth(), GetStride() and GetPixelFormat() methods
   * to decode the data.
   */
  unsigned char* GetPixelData() const;

  /**
   * Get the buffer height in pixels
   * @return the height of the buffer in pixels
   */
  unsigned int GetImageHeight() const;

  /**
   * Get the buffer width in pixels
   * @return the width of the buffer in pixels
   */
  unsigned int GetImageWidth() const;

  /**
   * Get the number of bytes in each row of pixels
   * @return The buffer stride in bytes.
   */
  unsigned int GetBufferStride() const;

  /**
   * Get the pixel format of the loaded bitmap.
   */
  Pixel::Format GetPixelFormat() const;

public: // Not intended for application developers

  explicit DALI_INTERNAL BitmapLoader(Internal::BitmapLoader*);
};

} // Dali

#endif // __DALI_BITMAP_LOADER_H__
