#ifndef __DALI_INTERNAL_ECORE_WL_WINDOW_RENDER_SURFACE_H__
#define __DALI_INTERNAL_ECORE_WL_WINDOW_RENDER_SURFACE_H__

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
#include <Ecore_Wayland.h>
#include <wayland-egl.h>
#include <dali/public-api/common/dali-common.h>

// INTERNAL INCLUDES
#include <dali/integration-api/egl-interface.h>
#include <dali/integration-api/thread-synchronization-interface.h>
#include <dali/internal/window-system/common/window-render-surface.h>

namespace Dali
{

namespace Internal
{
namespace Adaptor
{

/**
 * Ecore Wayland Window implementation of render surface.
 */
class WindowRenderSurfaceEcoreWl : public WindowRenderSurface
{
public:

  /**
    * Uses an Wayland surface to render to.
    * @param [in] positionSize the position and size of the surface
    * @param [in] surface can be a Wayland-window or Wayland-pixmap (type must be unsigned int).
    * @param [in] name optional name of surface passed in
    * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
    */
  WindowRenderSurfaceEcoreWl( Dali::PositionSize positionSize,
                       Any surface,
                       const std::string& name,
                       bool isTransparent = false );

  /**
   * @brief Destructor
   */
  virtual ~WindowRenderSurfaceEcoreWl();

public: // API

  /**
   * @brief Get window handle
   * @return the Ecore Waylnad window handle
   */
  Ecore_Wl_Window* GetWlWindow();

  /**
   * Notify output is transformed.
   */
  void OutputTransformed();

public: // from WindowRenderSurface

  /**
   * @copydoc Dali::Internal::Adaptor::WindowRenderSurface::GetWindow()
   */
  virtual Any GetWindow() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowRenderSurface::Map()
   */
  virtual void Map() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowRenderSurface::SetRenderNotification()
   */
  virtual void SetRenderNotification( TriggerEventInterface* renderNotification ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowRenderSurface::SetTransparency()
   */
  virtual void SetTransparency( bool transparent ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowRenderSurface::RequestRotation()
   */
  virtual void RequestRotation( int angle, int width, int height ) override;

public: // from Dali::RenderSurface

  /**
   * @copydoc Dali::RenderSurface::GetPositionSize()
   */
  virtual PositionSize GetPositionSize() const;

  /**
   * @copydoc Dali::RenderSurface::InitializeEgl()
   */
  virtual void InitializeEgl( EglInterface& egl );

  /**
   * @copydoc Dali::RenderSurface::CreateEglSurface()
   */
  virtual void CreateEglSurface( EglInterface& egl );

  /**
   * @copydoc Dali::RenderSurface::DestroyEglSurface()
   */
  virtual void DestroyEglSurface( EglInterface& egl );

  /**
   * @copydoc Dali::RenderSurface::ReplaceEGLSurface()
   */
  virtual bool ReplaceEGLSurface( EglInterface& egl );

  /**
   * @copydoc Dali::RenderSurface::MoveResize()
   */
  virtual void MoveResize( Dali::PositionSize positionSize);

  /**
   * @copydoc Dali::RenderSurface::SetViewMode()
   */
  void SetViewMode( ViewMode viewMode );

  /**
   * @copydoc Dali::RenderSurface::StartRender()
   */
  virtual void StartRender();

  /**
   * @copydoc Dali::RenderSurface::PreRender()
   */
  virtual bool PreRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, bool resizingSurface );

  /**
   * @copydoc Dali::RenderSurface::PostRender()
   */
  virtual void PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, DisplayConnection* displayConnection, bool replacingSurface, bool resizingSurface );

  /**
   * @copydoc Dali::RenderSurface::StopRender()
   */
  virtual void StopRender();

  /**
   * @copydoc Dali::RenderSurface::SetThreadSynchronization
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization );

  /**
   * @copydoc Dali::RenderSurface::ReleaseLock()
   */
  virtual void ReleaseLock();

  /**
   * @copydoc Dali::RenderSurface::GetSurfaceType()
   */
  virtual RenderSurface::Type GetSurfaceType();

private: // from WindowRenderSurface

  /**
   * @copydoc Dali::Internal::Adaptor::WindowRenderSurface::Initialize()
   */
  void Initialize( Any surface ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowRenderSurface::CreateRenderable()
   */
  void CreateRenderable() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowRenderSurface::UseExistingRenderable()
   */
  void UseExistingRenderable( unsigned int surfaceId ) override;

private:

  /**
   * Get the surface id if the surface parameter is not empty
   * @param surface Any containing a surface id, or can be empty
   * @return surface id, or zero if surface is empty
   */
  unsigned int GetSurfaceId( Any surface ) const;

  /**
   * Used as the callback for the rotation-trigger.
   */
  void ProcessRotationRequest();

private: // Data

  std::string                     mTitle;              ///< Title of window which shows from "xinfo -topvwins" command
  PositionSize                    mPositionSize;       ///< Position
  Ecore_Wl_Window*                mWlWindow;           ///< Wayland-Window
  wl_surface*                     mWlSurface;
  wl_egl_window*                  mEglWindow;
  ThreadSynchronizationInterface* mThreadSynchronization;
  TriggerEventInterface*          mRenderNotification; ///< Render notification trigger
  TriggerEventInterface*          mRotationTrigger;
  ColorDepth                      mColorDepth;         ///< Color depth of surface (32 bit or 24 bit)
  int                             mRotationAngle;
  int                             mScreenRotationAngle;
  bool                            mOwnSurface;         ///< Whether we own the surface (responsible for deleting it)
  bool                            mRotationSupported;
  bool                            mRotationFinished;
  bool                            mScreenRotationFinished;
  bool                            mResizeFinished;

}; // class WindowRenderSurfaceEcoreWl

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // __DALI_INTERNAL_ECORE_WL_WINDOW_RENDER_SURFACE_H__
