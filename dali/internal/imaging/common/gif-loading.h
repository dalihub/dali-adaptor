#ifndef DALI_INTERNAL_GIF_LOADING_H
#define DALI_INTERNAL_GIF_LOADING_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <cstdint>
#include <memory>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/math/uint-16-pair.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/internal/imaging/common/animated-image-loading-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Class to manage loading frames of an animated gif in small chunks. Lazy initializes only when
 * data is actually needed.
 * Note, once the GIF has loaded, the undecoded data will reside in memory until this object
 * is released. (This is to speed up frame loads, which would otherwise have to re-acquire the
 * data from disk)
 */
class GifLoading: public Internal::Adaptor::AnimatedImageLoading
{
public:

  /**
   * Create a GifLoading with the given url and resourceType.
   * @param[in] url The url of the gif image to load
   * @param[in] isLocalResource The true or false whether this is a local resource.
   * @return A newly created GifLoading.
   */
  static AnimatedImageLoadingPtr New( const std::string& url, bool isLocalResource );

  /**
   * @brief Constructor
   *
   * Construct a Loader with the given URL
   * @param[in] url The url of the gif image to load
   * @param[in] isLocalResource The true or false whether this is a local resource.
   */
  GifLoading( const std::string& url, bool isLocalResource );

  /**
   * @brief Destructor
   */
  ~GifLoading() override;

  /**
   * @brief Load the next N Frames of the gif.
   *
   * @note This function will load the entire gif into memory if not already loaded.
   * @param[in] frameStartIndex The frame counter to start from. Will usually be the next frame
   * after the previous invocation of this method, or 0 to start.
   * @param[in] count The number of frames to load
   * @param[out] pixelData The vector in which to return the frame data
   * @return True if the frame data was successfully loaded
   */
  bool LoadNextNFrames( uint32_t frameStartIndex, int count, std::vector<Dali::PixelData>& pixelData ) override;

   /**
   * @brief Load the next Frame of the animated image.
   *
   * @note This function will load the entire animated image into memory if not already loaded.
   * @param[in] frameIndex The frame counter to load. Will usually be the next frame.
   * @return Dali::Devel::PixelBuffer The loaded PixelBuffer. If loading is fail, return empty handle.
   */

  Dali::Devel::PixelBuffer LoadFrame( uint32_t frameIndex ) override;

  /**
   * @brief Get the size of a gif image.
   *
   * @return The width and height in pixels of the gif image.
   */
  ImageDimensions GetImageSize() const override;

  /**
   * @brief Get the number of frames in this gif.
   */
  uint32_t GetImageCount() const override;

  /**
   * @brief Get the frame interval of the frame index
   *
   * @note The frame is needed to be loaded before this function is called.
   *
   * @return The time interval of the frame(microsecond).
   */
  uint32_t GetFrameInterval( uint32_t frameIndex ) const override;

  /**
   * @brief Get the animated image file URL
   *
   * @return The URL string of the animated image file
   */
  std::string GetUrl() const override;

private:
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GIF_LOADING_H
