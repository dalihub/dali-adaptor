#ifndef DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H
#define DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H

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
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/threading/conditional-wait.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/egl-interface.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
class DisplayConnection;
class EglInterface;

/**
 * Ecore Wayland Native implementation of render surface.
 */
class NativeRenderSurfaceEcoreWl : public Dali::NativeRenderSurface
{
public:
  /**
   * Uses an Wayland surface to render to.
   * @param [in] surfaceSize the size of the surface
   * @param [in] surface the native surface handle
   * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
   */
  NativeRenderSurfaceEcoreWl(SurfaceSize surfaceSize, Any surface, bool isTransparent = false);

  /**
   * @brief Destructor
   */
  virtual ~NativeRenderSurfaceEcoreWl();

public:
  /**
   * @brief Triggers the FrameRenderedCallback
   */
  void TriggerFrameRenderedCallback();

public: // from NativeRenderSurface
  /**
   * @copydoc Dali::NativeRenderSurface::SetRenderNotification()
   */
  void SetRenderNotification(TriggerEventInterface* renderNotification) override;

  /**
   * @copydoc Dali::NativeRenderSurface::GetNativeRenderable()
   */
  virtual Any GetNativeRenderable() override;

  /**
   * @copydoc Dali::NativeRenderSurface::SetFrameRenderedCallback()
   */
  void SetFrameRenderedCallback(CallbackBase* callback) override;

public: // from Dali::Integration::RenderSurfaceInterface
  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetPositionSize()
   */
  PositionSize GetPositionSize() const override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetDpi()
   */
  void GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetSurfaceOrientation()
   */
  int GetSurfaceOrientation() const override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetScreenOrientation()
   */
  int GetScreenOrientation() const override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::InitializeGraphics()
   */
  void InitializeGraphics() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::CreateSurface()
   */
  void CreateSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::DestroySurface()
   */
  void DestroySurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::ReplaceGraphicsSurface()
   */
  bool ReplaceGraphicsSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::MoveResize()
   */
  void MoveResize(Dali::PositionSize positionSize) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::Resize()
   */
  void Resize(Uint16Pair size) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::StartRender()
   */
  void StartRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::PreRender()
   */
  bool PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::PostRender()
   */
  void PostRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::StopRender()
   */
  void StopRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::SetThreadSynchronization
   */
  void SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetSurfaceType()
   */
  Dali::Integration::RenderSurfaceInterface::Type GetSurfaceType() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::MakeContextCurrent()
   */
  void MakeContextCurrent() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetDepthBufferRequired()
   */
  Integration::DepthBufferAvailable GetDepthBufferRequired() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetStencilBufferRequired()
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired() override;

private:
  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::ReleaseLock()
   */
  void ReleaseLock() override;

  /**
   * @copydoc Dali::NativeRenderSurface::CreateNativeRenderable()
   */
  void CreateNativeRenderable() override;

  /**
   * Creates the context
   */
  void CreateContext();

  /**
   * Destroys the context
   */
  void DestroyContext();

private: // Data
  SurfaceSize                  mSurfaceSize;
  TriggerEventInterface*       mRenderNotification;
  Graphics::GraphicsInterface* mGraphics; ///< The graphics interface
  EglInterface*                mEGL;
  EGLSurface                   mEGLSurface;
  EGLContext                   mEGLContext;
  ColorDepth                   mColorDepth;
  tbm_format                   mTbmFormat;
  bool                         mOwnSurface;

  tbm_surface_queue_h                  mTbmQueue;
  ThreadSynchronizationInterface*      mThreadSynchronization; ///< A pointer to the thread-synchronization
  std::unique_ptr<EventThreadCallback> mFrameRenderedCallback; ///< The FrameRendredCallback called from graphics driver
};

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H
