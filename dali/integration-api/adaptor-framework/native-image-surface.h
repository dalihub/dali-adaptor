#ifndef DALI_NATIVE_IMAGE_SURFACE_H
#define DALI_NATIVE_IMAGE_SURFACE_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/any.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class NativeImageSurface;
}
} // namespace Internal

class NativeImageSurface;

/**
 * @brief Pointer to Dali::NativeImageSurface.
 */
typedef Dali::IntrusivePtr<Dali::NativeImageSurface> NativeImageSurfacePtr;

/**
 * Native image surface is a surface that GL can render.
 * The surface is not a window, it's like a pixmap.
 */
class DALI_ADAPTOR_API NativeImageSurface : public RefObject
{
public:
  /**
   * Creates a NativeImageSurface
   *
   * @param [in] queue the native image source queue handle
   * @return A smart-pointer to a newly allocated native image surface
   */
  static NativeImageSurfacePtr New(Dali::NativeImageSourceQueuePtr queue);

  /**
   * @brief Gets the native renderable handle
   * @return The native renderable handle
   */
  Any GetNativeRenderable();

  /**
   * @brief Initializes the graphics resources
   */
  void InitializeGraphics();

  /**
   * @brief Terminate the graphics resources
   */
  void TerminateGraphics();

  /**
   * @brief Invoked by render thread before rendering
   */
  void PreRender();

  /**
   * @brief Invoked by render thread after rendering
   */
  void PostRender();

  /**
   * @brief Checks to render
   */
  bool CanRender();

  /**
   * @brief Sets Graphics configuration to the surface
   *
   * @param[in] depth the flag of depth buffer. If true is set, 24bit depth buffer is enabled.
   * @param[in] stencil the flag of stencil. it true is set, 8bit stencil buffer is enabled.
   * @param[in] msaa the bit of msaa.
   * @param[in] version the GLES version.
   * @return True if the config exists, false otherwise.
   */
  bool SetGraphicsConfig(bool depth, bool stencil, int msaa, int version);

private:
  /// @cond internal
  /**
   * @brief Private constructor.
   * @param[in] queue the native image source queue handle
   */
  DALI_INTERNAL NativeImageSurface(Dali::NativeImageSourceQueuePtr queue);

  /**
   * @brief A reference counted object may only be deleted by calling Unreference().
   */
  DALI_INTERNAL ~NativeImageSurface() override;

  /**
   * @brief Undefined copy constructor.
   *
   * This avoids accidental calls to a default copy constructor.
   * @param[in] nativeImageSurface A reference to the object to copy
   */
  DALI_INTERNAL NativeImageSurface(const NativeImageSurface& nativeImageSurface);

  /**
   * @brief Undefined assignment operator.
   *
   * This avoids accidental calls to a default assignment operator.
   * @param[in] rhs A reference to the object to copy
   */
  DALI_INTERNAL NativeImageSurface& operator=(const NativeImageSurface& rhs);
  /// @endcond

private:
  std::unique_ptr<Internal::Adaptor::NativeImageSurface> mImpl; ///< Implementation pointer
};

} // namespace Dali

#endif // DALI_NATIVE_IMAGE_SURFACE_H
