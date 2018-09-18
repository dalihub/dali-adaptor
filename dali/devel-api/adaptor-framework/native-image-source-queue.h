#ifndef DALI_NATIVE_IMAGE_SOURCE_QUEUE_H
#define DALI_NATIVE_IMAGE_SOURCE_QUEUE_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/images/native-image-interface.h>
#include <dali/public-api/object/any.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class NativeImageSourceQueue;
}
}

class NativeImageSourceQueue;

/**
 * @brief Pointer to Dali::NativeImageSourceQueue.
 */
typedef Dali::IntrusivePtr< Dali::NativeImageSourceQueue > NativeImageSourceQueuePtr;

/**
 * @brief Used for displaying native images.
 *
 * NativeImageSource can be created internally or externally by native image source.
 * NativeImage is a platform specific way of providing pixel data to the GPU for rendering,for example via an EGL image.
 */
class DALI_ADAPTOR_API NativeImageSourceQueue : public NativeImageInterface
{
public:

   /**
    * @brief Enumeration for the instance when creating a native image, the color depth has to be specified.
    */
   enum ColorDepth
   {
     COLOR_DEPTH_DEFAULT,     ///< Uses the current screen default depth (recommended)
     COLOR_DEPTH_24,          ///< 24 bits per pixel
     COLOR_DEPTH_32           ///< 32 bits per pixel
   };

  /**
   * @brief Creates a new NativeImageSourceQueue.
   *        Depending on hardware, the width and height may have to be a power of two.
   * @param[in] width The width of the image
   * @param[in] height The height of the image
   * @param[in] depth color depth of the image
   * @return A smart-pointer to a newly allocated image
   */
  static NativeImageSourceQueuePtr New( unsigned int width, unsigned int height, ColorDepth depth );

  /**
   * @brief Creates a new NativeImageSourceQueue from an existing native image source.
   *
   * @param[in] nativeImageSourceQueue NativeImageSourceQueue must be a any handle with native image source
   * @return A smart-pointer to a newly allocated image
   * @see NativeImageInterface
   */
  static NativeImageSourceQueuePtr New( Any nativeImageSourceQueue );

  /**
   * @brief Retrieves the internal native image.
   *
   * @return Any object containing the internal native image source queue
   */
  Any GetNativeImageSourceQueue();

  /**
   * @brief Sets an existing source.
   *
   * @param[in] source Any handle with the source
   */
  void SetSource( Any source );

private:   // native image

  /**
   * @copydoc Dali::NativeImageInterface::GlExtensionCreate()
   */
  virtual bool GlExtensionCreate();

  /**
   * @copydoc Dali::NativeImageInterface::GlExtensionDestroy()
   */
  virtual void GlExtensionDestroy();

  /**
   * @copydoc Dali::NativeImageInterface::TargetTexture()
   */
  virtual unsigned int TargetTexture();

  /**
   * @copydoc Dali::NativeImageInterface::PrepareTexture()
   */
  virtual void PrepareTexture();

  /**
   * @copydoc Dali::NativeImageInterface::GetWidth()
   */
  virtual unsigned int GetWidth() const;

  /**
   * @copydoc Dali::NativeImageInterface::GetHeight()
   */
  virtual unsigned int GetHeight() const;

  /**
   * @copydoc Dali::NativeImageInterface::RequiresBlending()
   */
  virtual bool RequiresBlending() const;

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetExtension();

private:

  /// @cond internal
  /**
   * @brief Private constructor.
   * @param[in] width The width of the image
   * @param[in] height The height of the image
   * @param[in] depth color depth of the image
   * @param[in] nativeImageSourceQueue contains either: native image source or is empty
   */
  DALI_INTERNAL NativeImageSourceQueue( unsigned int width, unsigned int height, ColorDepth depth, Any nativeImageSourceQueue );

  /**
   * @brief A reference counted object may only be deleted by calling Unreference().
   *
   * The implementation should destroy the NativeImage resources.
   */
  DALI_INTERNAL virtual ~NativeImageSourceQueue();

  /**
   * @brief Undefined copy constructor.
   *
   * This avoids accidental calls to a default copy constructor.
   * @param[in] nativeImageSourceQueue A reference to the object to copy
   */
  DALI_INTERNAL NativeImageSourceQueue( const NativeImageSourceQueue& nativeImageSourceQueue );

  /**
   * @brief Undefined assignment operator.
   *
   * This avoids accidental calls to a default assignment operator.
   * @param[in] rhs A reference to the object to copy
   */
  DALI_INTERNAL NativeImageSourceQueue& operator=(const NativeImageSourceQueue& rhs);
  /// @endcond

private:

  /// @cond internal
  std::unique_ptr< Internal::Adaptor::NativeImageSourceQueue > mImpl; ///< Implementation pointer
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_NATIVE_IMAGE_SOURCE_QUEUE_H
