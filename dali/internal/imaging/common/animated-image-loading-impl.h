#ifndef DALI_INTERNAL_ANIMATED_IMAGE_LOADING_IMPL_H
#define DALI_INTERNAL_ANIMATED_IMAGE_LOADING_IMPL_H

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
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/devel-api/adaptor-framework/animated-image-loading.h>

namespace Dali
{
class PixelData;
typedef Dali::Uint16Pair ImageDimensions;

namespace Internal
{

namespace Adaptor
{

class AnimatedImageLoading;
typedef IntrusivePtr<AnimatedImageLoading> AnimatedImageLoadingPtr;

/**
 * Class interface for animated image loading.
 * Each loading classes for animated image file format(e.g., gif and webp) needs to inherit this interface
 */
class AnimatedImageLoading : public BaseObject
{
public:

  /**
   * @copydoc Dali::AnimatedImageLoading::New()
   */
  static AnimatedImageLoadingPtr New( const std::string& url, bool isLocalResource );

  AnimatedImageLoading() = default;

  // Moveable but not copyable
  AnimatedImageLoading( const AnimatedImageLoading& );
  AnimatedImageLoading& operator=( const AnimatedImageLoading& );
  AnimatedImageLoading( AnimatedImageLoading&& ) = default;
  AnimatedImageLoading& operator=( AnimatedImageLoading&& ) = default;

  /**
   * @brief Destructor
   */
  virtual ~AnimatedImageLoading() = default;

  /**
   * @copydoc Dali::AnimatedImageLoading::LoadNextNFrames()
   */
  virtual bool LoadNextNFrames( uint32_t frameStartIndex, int count, std::vector<Dali::PixelData>& pixelData ) = 0;

  /**
   * @copydoc Dali::AnimatedImageLoading::LoadFrame()
   */
  virtual Dali::Devel::PixelBuffer LoadFrame( uint32_t frameIndex ) = 0;

  /**
   * @copydoc Dali::AnimatedImageLoading::GetImageSize()
   */
  virtual ImageDimensions GetImageSize() const = 0;

  /**
   * @copydoc Dali::AnimatedImageLoading::GetImageCount()
   */
  virtual uint32_t GetImageCount() const = 0;

  /**
   * @copydoc Dali::AnimatedImageLoading::LoadFrameDelays()
   */
  virtual uint32_t GetFrameInterval( uint32_t frameIndex ) const = 0;

  /**
   * @copydoc Dali::AnimatedImageLoading::GetUrl()
   */
  virtual std::string GetUrl() const = 0;
};

} // namespace Adaptor

} // namespace Internal

// Helpers for api forwarding methods

inline Internal::Adaptor::AnimatedImageLoading& GetImplementation( Dali::AnimatedImageLoading& handle)
{
  DALI_ASSERT_ALWAYS( handle && "AnimatedImageLoading handle is empty" );

  BaseObject& object = handle.GetBaseObject();

  return static_cast< Internal::Adaptor::AnimatedImageLoading& >( object );
}

inline const Internal::Adaptor::AnimatedImageLoading& GetImplementation( const Dali::AnimatedImageLoading& handle )
{
  DALI_ASSERT_ALWAYS( handle && "AnimatedImageLoading handle is empty" );

  const BaseObject& object = handle.GetBaseObject();

  return static_cast< const Internal::Adaptor::AnimatedImageLoading& >( object );
}

} // namespace Dali

#endif // DALI_INTERNAL_ANIMATED_IMAGE_LOADING_IMPL_H
