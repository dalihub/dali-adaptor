#ifndef DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_IMAGE_SURFACE_IMPL_ECORE_WL_H
#define DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_IMAGE_SURFACE_IMPL_ECORE_WL_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/integration-api/adaptor-framework/egl-interface.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/window-system/common/native-image-surface-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class NativeImageSurfaceEcoreWl : public Dali::Internal::Adaptor::NativeImageSurface
{
public:
  /**
    * @param [in] queue the NativeImageSourceQueue pointer
    */
  NativeImageSurfaceEcoreWl(Dali::NativeImageSourceQueuePtr queue);

  /**
   * @brief Destructor
   */
  ~NativeImageSurfaceEcoreWl() = default;

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
   * @copydoc Dali::NativeImageSurface::TerminateGraphics()
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

private:
  void MakeContextCurrent();

private:                                                           // Data
  std::unique_ptr<Dali::DisplayConnection>     mDisplayConnection; ///< The native display connection
  std::unique_ptr<Graphics::GraphicsInterface> mGraphics;          ///< Graphics interface
#if(!VULKAN_ENABLED)
  EglInterface* mEGL;
  EGLSurface    mEGLSurface;
  EGLContext    mEGLContext;
#endif
  ColorDepth          mColorDepth;
  tbm_format          mTbmFormat;
  tbm_surface_queue_h mTbmQueue;

  bool mDepth : 1;
  bool mStencil : 1;
  int  mGLESVersion;
  int  mMSAA;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_IMAGE_SURFACE_IMPL_ECORE_WL_H
