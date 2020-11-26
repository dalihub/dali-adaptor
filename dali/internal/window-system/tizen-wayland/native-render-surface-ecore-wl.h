#ifndef DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H
#define DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H

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
#include <tbm_surface.h>
#include <tbm_surface_queue.h>
#include <dali/devel-api/threading/conditional-wait.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/integration-api/adaptor-framework/egl-interface.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>
#include <dali/internal/graphics/common/graphics-interface.h>

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
  NativeRenderSurfaceEcoreWl( SurfaceSize surfaceSize, Any surface, bool isTransparent = false );

  /**
   * @brief Destructor
   */
  virtual ~NativeRenderSurfaceEcoreWl();

public: // from WindowRenderSurface

  /**
   * @copydoc Dali::NativeRenderSurface::GetSurface()
   */
  Any GetDrawable() override;

  /**
   * @copydoc Dali::NativeRenderSurface::SetRenderNotification()
   */
  void SetRenderNotification( TriggerEventInterface* renderNotification ) override;

  /**
   * @copydoc Dali::NativeRenderSurface::WaitUntilSurfaceReplaced()
   */
  void WaitUntilSurfaceReplaced() override;

  /**
   * @copydoc Dali::NativeRenderSurface::GetNativeRenderable()
   */
  virtual Any GetNativeRenderable() override;

public: // from Dali::RenderSurfaceInterface

  /**
   * @copydoc Dali::RenderSurfaceInterface::GetPositionSize()
   */
  PositionSize GetPositionSize() const override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::GetDpi()
   */
  void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::InitializeGraphics()
   */
  void InitializeGraphics() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::CreateSurface()
   */
  void CreateSurface() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::DestroySurface()
   */
  void DestroySurface() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::ReplaceGraphicsSurface()
   */
  bool ReplaceGraphicsSurface() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::MoveResize()
   */
  void MoveResize( Dali::PositionSize positionSize) override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::StartRender()
   */
  void StartRender() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::PreRender()
   */
  bool PreRender( bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect ) override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::PostRender()
   */
  void PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface, const std::vector<Rect<int>>& damagedRects ) override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::StopRender()
   */
  void StopRender() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::SetThreadSynchronization
   */
  void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::GetSurfaceType()
   */
  Dali::RenderSurfaceInterface::Type GetSurfaceType() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::MakeContextCurrent()
   */
  void MakeContextCurrent() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::GetDepthBufferRequired()
   */
  Integration::DepthBufferAvailable GetDepthBufferRequired() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::GetStencilBufferRequired()
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired() override;

private:

  /**
   * @copydoc Dali::RenderSurfaceInterface::ReleaseLock()
   */
  void ReleaseLock() override;

  /**
   * @copydoc Dali::NativeRenderSurface::CreateNativeRenderable()
   */
  void CreateNativeRenderable() override;

  /**
   * @copydoc Dali::NativeRenderSurface::ReleaseDrawable()
   */
  void ReleaseDrawable() override;

private: // Data

  SurfaceSize                            mSurfaceSize;
  TriggerEventInterface*                 mRenderNotification;
  Internal::Adaptor::GraphicsInterface*  mGraphics;                  ///< The graphics interface
  EglInterface*                          mEGL;
  EGLSurface                             mEGLSurface;
  EGLContext                             mEGLContext;
  ColorDepth                             mColorDepth;
  tbm_format                             mTbmFormat;
  bool                                   mOwnSurface;
  bool                                   mDrawableCompleted;

  tbm_surface_queue_h                    mTbmQueue;
  tbm_surface_h                          mConsumeSurface;
  ThreadSynchronizationInterface*        mThreadSynchronization;     ///< A pointer to the thread-synchronization
  ConditionalWait                        mTbmSurfaceCondition;

};

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H
