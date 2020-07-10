#ifndef DALI_ANIMATED_IMAGE_LOADING_H
#define DALI_ANIMATED_IMAGE_LOADING_H

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
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

namespace Dali
{

class PixelData;
typedef Dali::Uint16Pair ImageDimensions;

/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class AnimatedImageLoading;
}
}

/**
 * Class to manage loading frames of an animated image in small chunks. Lazy initializes only when
 * data is actually needed.
 * Note, once the Animated Image has loaded, the undecoded data will reside in memory until this object
 * is released. (This is to speed up frame loads, which would otherwise have to re-acquire the
 * data from disk)
 */
class DALI_ADAPTOR_API AnimatedImageLoading : public BaseHandle
{
public:

  /**
   * Create a GifLoading with the given url and resourceType.
   * @param[in] url The url of the animated image to load
   * @param[in] isLocalResource The true or false whether this is a local resource.
   * @return A newly created GifLoading.
   */
  static AnimatedImageLoading New( const std::string& url, bool isLocalResource );

  /**
   * @brief Constructor
   */
  AnimatedImageLoading();

  /**
   * @brief Downcast an Object handle to Capture handle.
   *
   * If handle points to a Capture object the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle to An object.
   * @return handle to a Capture object or an uninitialized handle.
   */
  static AnimatedImageLoading DownCast( BaseHandle handle );


  /**
   * @brief Copy constructor.
   *
   * @param[in] copy The AnimatedImageLoading to copy
   */
  AnimatedImageLoading(const AnimatedImageLoading& copy) = default;

  /**
   * @brief Assignment operator
   *
   * @param[in] rhs The AnimatedImageLoading to copy
   * @return A reference to this
   */
  AnimatedImageLoading& operator=(const AnimatedImageLoading& rhs) = default;

  /**
   * @brief Destructor
   */
  ~AnimatedImageLoading();

  /**
   * @brief Load the next N Frames of the animated image.
   *
   * @note This function will load the entire animated image into memory if not already loaded.
   * @param[in] frameStartIndex The frame counter to start from. Will usually be the next frame
   * after the previous invocation of this method, or 0 to start.
   * @param[in] count The number of frames to load
   * @param[out] pixelData The vector in which to return the frame data
   * @return True if the frame data was successfully loaded
   */
  bool LoadNextNFrames( uint32_t frameStartIndex, int count, std::vector<Dali::PixelData>& pixelData );

   /**
   * @brief Load a frame of the animated image.
   *
   * @note This function will load the entire animated image into memory if not already loaded.
   * @param[in] frameIndex The frame index to load.
   * @return Dali::Devel::PixelBuffer The loaded PixelBuffer. If loading is fail, return empty handle.
   */

  Dali::Devel::PixelBuffer LoadFrame( uint32_t frameIndex );

  /**
   * @brief Get the size of a animated image.
   *
   * @return The width and height in pixels of the animated image.
   */
  ImageDimensions GetImageSize() const;

  /**
   * @brief Get the number of frames in this animated image.
   */
  uint32_t GetImageCount() const;

  /**
   * @brief Get the frame interval of the frame index
   *
   * @note The frame is needed to be loaded before this function is called.
   *
   * @return The time interval of the frame(microsecond).
   */
  uint32_t GetFrameInterval( uint32_t frameIndex ) const;

  /**
   * @brief Get the animated image file URL
   *
   * @return The URL string of the animated image file
   */
  std::string GetUrl() const;

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief This constructor is used by New() methods.
   *
   * @param[in] internal A pointer to a newly allocated Dali resource.
   */
  explicit DALI_INTERNAL AnimatedImageLoading( Internal::Adaptor::AnimatedImageLoading* internal );
  /// @endcond
};

} // namespace Dali

#endif // DALI_ANIMATED_IMAGE_LOADING_H
