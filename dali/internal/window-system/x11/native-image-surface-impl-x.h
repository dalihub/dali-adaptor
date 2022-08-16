#ifndef DALI_INTERNAL_WINDOW_SYSTEM_X11_NATIVE_IMAGE_SURFACE_IMPL_X_H
#define DALI_INTERNAL_WINDOW_SYSTEM_X11_NATIVE_IMAGE_SURFACE_IMPL_X_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/internal/window-system/common/native-image-surface-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class NativeImageSurfaceX : public Dali::Internal::Adaptor::NativeImageSurface
{
public:
  /**
    * @param [in] queue the NativeImageSourceQueue pointer
    */
  NativeImageSurfaceX(Dali::NativeImageSourceQueuePtr queue);

  /**
   * @brief Destructor
   */
  ~NativeImageSurfaceX();

public:
  /**
   * @copydoc Dali::NativeImageSurface::GetNativeRenderable()
   */
  Any GetNativeRenderable() override;

  /**
   * @copydoc Dali::NativeImageSurface::InitializeGraphics()
   */
  void InitializeGraphics() override;

  /**
   * @copydoc Dali::NativeImageSurface::InitializeGraphics()
   */
  void TerminateGraphics() override;

  /**
   * @copydoc Dali::NativeImageSurface::PreRender()
   */
  void PreRender() override;

  /**
   * @copydoc Dali::NativeImageSurface::PostRender()
   */
  void PostRender() override;

  /**
   * @copydoc Dali::NativeImageSurface::CanRender()
   */
  bool CanRender() override;

  /**
   * @copydoc Dali::NativeImageSurface::SetGraphicsConfig()
   */
  bool SetGraphicsConfig(bool depth, bool stencil, int msaa, int version) override;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOW_SYSTEM_X11_NATIVE_IMAGE_SURFACE_IMPL_X_H
