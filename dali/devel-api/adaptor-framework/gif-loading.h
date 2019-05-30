#ifndef DALI_INTERNAL_GIF_LOADING_H
#define DALI_INTERNAL_GIF_LOADING_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

namespace Dali
{
class PixelData;
typedef Dali::Uint16Pair ImageDimensions;

/**
 * Class to manage loading frames of an animated gif in small chunks. Lazy initializes only when
 * data is actually needed.
 * Note, once the GIF has loaded, the undecoded data will reside in memory until this object
 * is released. (This is to speed up frame loads, which would otherwise have to re-acquire the
 * data from disk)
 */
class DALI_ADAPTOR_API GifLoading
{
public:

  /**
   * Create a GifLoading with the given url and resourceType.
   * @param[in] url The url of the gif image to load
   * @param[in] isLocalResource The true or false whether this is a local resource.
   * @return A newly created GifLoading.
   */
  static std::unique_ptr<GifLoading> New( const std::string& url, bool isLocalResource );

  /**
   * @brief Constructor
   *
   * Construct a Loader with the given URL
   * @param[in] url The url of the gif image to load
   * @param[in] isLocalResource The true or false whether this is a local resource.
   */
  GifLoading( const std::string& url, bool isLocalResource );

  // Moveable but not copyable

  GifLoading( const GifLoading& ) = delete;
  GifLoading& operator=( const GifLoading& ) = delete;
  GifLoading( GifLoading&& ) = default;
  GifLoading& operator=( GifLoading&& ) = default;

  /**
   * @brief Destructor
   */
  ~GifLoading();

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
  bool LoadNextNFrames( int frameStartIndex, int count, std::vector<Dali::PixelData>& pixelData );

  /**
   * @brief Load all frames of an animated gif file.
   *
   * @note This function will load the entire gif into memory if not already loaded.
   *
   * @param[out] pixelData The loaded pixel data for each frame.
   * @param[out] frameDelays The loaded delay time for each frame.
   *
   * @return True if the loading succeeded, false otherwise.
   */
  bool LoadAllFrames( std::vector<Dali::PixelData>& pixelData, Dali::Vector<uint32_t>& frameDelays );

  /**
   * @brief Get the size of a gif image.
   *
   * @note This function will load the entire gif into memory if not already loaded.
   *
   * @return The width and height in pixels of the gif image.
   */
  ImageDimensions GetImageSize();

  /**
   * @brief Get the number of frames in this gif.
   *
   * @note This function will load the entire gif into memory if not already loaded.
   */
  int GetImageCount();

  /**
   * @brief Load the frame delay counts into the provided array.
   *
   * @note This function will load the entire gif into memory if not already loaded.
   * @param[in] frameDelays a vector to write the frame delays into
   * @return true if the frame delays were successfully loaded
   */
  bool LoadFrameDelays( Dali::Vector<uint32_t>& frameDelays );

private:
  struct Impl;
  Impl* mImpl;
};

} // namespace Dali

#endif // DALI_INTERNAL_GIF_LOADING_H
