#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_NATIVE_IMAGE_SURFACE_IMPL_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_NATIVE_IMAGE_SURFACE_IMPL_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>

namespace Dali
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
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_NATIVE_IMAGE_SURFACE_IMPL_H
