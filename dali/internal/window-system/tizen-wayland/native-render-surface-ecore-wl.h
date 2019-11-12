#ifndef DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H
#define DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
    * @param [in] positionSize the position and size of the surface
    * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
    */
  NativeRenderSurfaceEcoreWl( Dali::PositionSize positionSize, bool isTransparent = false );

  /**
   * @brief Destructor
   */
  virtual ~NativeRenderSurfaceEcoreWl();

public: // from WindowRenderSurface

  /**
   * @copydoc Dali::NativeRenderSurface::GetSurface()
   */
  virtual Any GetDrawable() override;

  /**
   * @copydoc Dali::NativeRenderSurface::SetRenderNotification()
   */
  virtual void SetRenderNotification( TriggerEventInterface* renderNotification ) override;

  /**
   * @copydoc Dali::NativeRenderSurface::WaitUntilSurfaceReplaced()
   */
  virtual void WaitUntilSurfaceReplaced() override;

public: // from Dali::Integration::RenderSurface

  /**
   * @copydoc Dali::Integration::RenderSurface::GetPositionSize()
   */
  virtual PositionSize GetPositionSize() const override;

  /**
   * @copydoc Dali::Integration::RenderSurface::GetDpi()
   */
  virtual void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::InitializeGraphics()
   */
  virtual void InitializeGraphics() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::CreateSurface()
   */
  virtual void CreateSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::DestroySurface()
   */
  virtual void DestroySurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::ReplaceGraphicsSurface()
   */
  virtual bool ReplaceGraphicsSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::MoveResize()
   */
  virtual void MoveResize( Dali::PositionSize positionSize) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::StartRender()
   */
  virtual void StartRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::PreRender()
   */
  virtual bool PreRender( bool resizingSurface ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::PostRender()
   */
  virtual void PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface );

  /**
   * @copydoc Dali::Integration::RenderSurface::StopRender()
   */
  virtual void StopRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::SetThreadSynchronization
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )override;

  /**
   * @copydoc Dali::Integration::RenderSurface::GetSurfaceType()
   */
  virtual Integration::RenderSurface::Type GetSurfaceType() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::MakeContextCurrent()
   */
  virtual void MakeContextCurrent() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::GetDepthBufferRequired()
   */
  virtual Integration::DepthBufferAvailable GetDepthBufferRequired() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::GetStencilBufferRequired()
   */
  virtual Integration::StencilBufferAvailable GetStencilBufferRequired() override;

private:

  /**
   * @copydoc Dali::Integration::RenderSurface::ReleaseLock()
   */
  virtual void ReleaseLock() override;

  /**
   * @copydoc Dali::NativeRenderSurface::CreateNativeRenderable()
   */
  virtual void CreateNativeRenderable() override;

  /**
   * @copydoc Dali::NativeRenderSurface::ReleaseDrawable()
   */
  virtual void ReleaseDrawable() override;

private: // Data

  PositionSize                           mPosition;
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
