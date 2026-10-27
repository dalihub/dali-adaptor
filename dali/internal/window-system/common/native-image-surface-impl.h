#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_NATIVE_IMAGE_SURFACE_IMPL_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_NATIVE_IMAGE_SURFACE_IMPL_H

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
#include <memory> ///< for std::shared_ptr

#include <dali/public-api/object/any.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-queue.h>

namespace DALI_NAMESPACE
{
class NativeImageSurface;

namespace Internal
{
namespace Adaptor
{
class NativeImageSurface
{
public:
  /**
   * @brief Destructor
   */
  virtual ~NativeImageSurface() = default;

  /**
   * @copydoc Dali::NativeImageSurface::GetNativeRenderable()
   */
  virtual Any GetNativeRenderable() = 0;

  /**
   * @copydoc Dali::NativeImageSurface::InitializeGraphics()
   */
  virtual void InitializeGraphics() = 0;

  /**
   * @copydoc Dali::NativeImageSurface::TerminateGraphics()
   */
  virtual void TerminateGraphics() = 0;

  /**
   * @copydoc Dali::NativeImageSurface::PreRender()
   */
  virtual void PreRender() = 0;

  /**
   * @copydoc Dali::NativeImageSurface::PostRender()
   */
  virtual void PostRender() = 0;

  /**
   * @copydoc Dali::NativeImageSurface::CanRender()
   */
  virtual bool CanRender() = 0;

  /**
   * @copydoc Dali::NativeImageSurface::SetGraphicsConfig()
   */
  virtual bool SetGraphicsConfig(bool depth, bool stencil, int msaa, int version) = 0;

public: // graphics library lifetime
  /**
   * @brief Sets the graphics backend module that keeps the library loaded while this object is alive.
   * @note Only the public wrapper may call this: implementation code must not fetch the module
   *       itself, as the backend library binds to its own copy of the accessor.
   * @param[in] graphicsLibrary The graphics backend module
   */
  void SetGraphicsLibrary(std::shared_ptr<void> graphicsLibrary)
  {
    mGraphicsLibrary = std::move(graphicsLibrary);
  }

  /**
   * @brief Gets the graphics backend module.
   * @note Deleting code must copy this first, so the last reference is released only after this
   *       object's destructor, whose code lives in the backend library, has returned.
   * @return The graphics backend module
   */
  std::shared_ptr<void> GetGraphicsLibrary() const
  {
    return mGraphicsLibrary;
  }

private:
  std::shared_ptr<void> mGraphicsLibrary; ///< Keeps the graphics backend library loaded while this object is alive
};

} // namespace Adaptor
} // namespace Internal
} //namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_NATIVE_IMAGE_SURFACE_IMPL_H
