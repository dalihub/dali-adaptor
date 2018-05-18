#ifndef DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H
#define DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H

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
#include <tbm_surface.h>
#include <tbm_surface_queue.h>
#include <dali/devel-api/threading/conditional-wait.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/integration-api/native-render-surface.h>

namespace Dali
{

/**
 * Ecore Wayland Native implementation of render surface.
 */
class NativeRenderSurfaceEcoreWl : public Dali::NativeRenderSurface
{
public:

  /**
    * Uses an Wayland surface to render to.
    * @param [in] positionSize the position and size of the surface
    * @param [in] name optional name of surface passed in
    * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
    */
  NativeRenderSurfaceEcoreWl( Dali::PositionSize positionSize,
                             const std::string& name,
                             bool isTransparent = false );

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

public: // from Dali::RenderSurface

  /**
   * @copydoc Dali::RenderSurface::GetPositionSize()
   */
  virtual PositionSize GetPositionSize() const override;

  /**
   * @copydoc Dali::RenderSurface::GetDpi()
   */
  virtual void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) override;

  /**
   * @copydoc Dali::RenderSurface::InitializeEgl()
   */
  virtual void InitializeEgl( EglInterface& egl ) override;

  /**
   * @copydoc Dali::RenderSurface::CreateEglSurface()
   */
  virtual void CreateEglSurface( EglInterface& egl ) override;

  /**
   * @copydoc Dali::RenderSurface::DestroyEglSurface()
   */
  virtual void DestroyEglSurface( EglInterface& egl ) override;

  /**
   * @copydoc Dali::RenderSurface::ReplaceEGLSurface()
   */
  virtual bool ReplaceEGLSurface( EglInterface& egl ) override;

  /**
   * @copydoc Dali::RenderSurface::MoveResize()
   */
  virtual void MoveResize( Dali::PositionSize positionSize) override;

  /**
   * @copydoc Dali::RenderSurface::SetViewMode()
   */
  virtual void SetViewMode( ViewMode viewMode ) override;

  /**
   * @copydoc Dali::RenderSurface::StartRender()
   */
  virtual void StartRender() override;

  /**
   * @copydoc Dali::RenderSurface::PreRender()
   */
  virtual bool PreRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, bool resizingSurface ) override;

  /**
   * @copydoc Dali::RenderSurface::PostRender()
   */
  virtual void PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, DisplayConnection* displayConnection, bool replacingSurface, bool resizingSurface ) override;

  /**
   * @copydoc Dali::RenderSurface::StopRender()
   */
  virtual void StopRender() override;

  /**
   * @copydoc Dali::RenderSurface::SetThreadSynchronization
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )override;

  /**
   * @copydoc Dali::RenderSurface::GetSurfaceType()
   */
  virtual RenderSurface::Type GetSurfaceType() override;

private:

  /**
   * @copydoc Dali::RenderSurface::ReleaseLock()
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

  PositionSize                    mPosition;
  std::string                     mTitle;
  TriggerEventInterface*          mRenderNotification;
  ColorDepth                      mColorDepth;
  tbm_format                      mTbmFormat;
  bool                            mOwnSurface;
  bool                            mDrawableCompleted;

  tbm_surface_queue_h             mTbmQueue;
  tbm_surface_h                   mConsumeSurface;
  ThreadSynchronizationInterface* mThreadSynchronization;     ///< A pointer to the thread-synchronization
  ConditionalWait                 mTbmSurfaceCondition;

};

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_NATIVE_SURFACE_ECORE_WL_H
