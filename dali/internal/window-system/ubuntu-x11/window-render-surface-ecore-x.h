#ifndef __DALI_INTERNAL_ECORE_X_WINDOW_RENDER_SURFACE_H__
#define __DALI_INTERNAL_ECORE_X_WINDOW_RENDER_SURFACE_H__

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

// INTERNAL INCLUDES
#include <dali/integration-api/egl-interface.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/ubuntu-x11/ecore-x-types.h>

// EXTERNAL INCLUDES
#include <Ecore_X.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * Ecore X11 Window implementation of render surface.
 */
class WindowRenderSurfaceEcoreX : public WindowRenderSurface
{
public:

  /**
    * Uses an X11 surface to render to.
    * @param [in] positionSize the position and size of the surface
    * @param [in] surface can be a X-window or X-pixmap (type must be unsigned int).
    * @param [in] name optional name of surface passed in
    * @param [in] className optional class name of the surface passed in
    * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
    */
  WindowRenderSurfaceEcoreX( Dali::PositionSize positionSize,
                       Any surface,
                       const std::string& name,
                       const std::string& className,
                       bool isTransparent = false );

  /**
   * @brief Destructor
   */
  virtual ~WindowRenderSurfaceEcoreX();

public: // API

  /**
   * @brief Get window handle
   * @return the Ecore X window handle
   */
  Ecore_X_Window GetXWindow();

  /**
   * Request to approve deiconify operation
   * If it is requested, it will send ECORE_X_ATOM_E_DEICONIFY_APPROVE event to window manager after rendering
   */
  void RequestToApproveDeiconify();

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
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization ) override;

  /**
   * @copydoc Dali::RenderSurface::ReleaseLock()
   */
  virtual void ReleaseLock() override;

  /**
   * @copydoc Dali::RenderSurface::GetSurfaceType()
   */
  virtual RenderSurface::Type GetSurfaceType() override;

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

private: // Data

  std::string      mTitle;                  ///< Title of window which shows from "xinfo -topvwins" command
  std::string      mClassName;              ///< The class name of the window
  PositionSize     mPosition;               ///< Position
  ColorDepth       mColorDepth;             ///< Color depth of surface (32 bit or 24 bit)
  Ecore_X_Window   mX11Window;              ///< X-Window
  bool             mOwnSurface;             ///< Whether we own the surface (responsible for deleting it)
  bool             mNeedToApproveDeiconify; ///< Whether need to send ECORE_X_ATOM_E_DEICONIFY_APPROVE event

}; // class WindowRenderSurfaceEcoreX

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // __DALI_INTERNAL_ECORE_X_WINDOW_RENDER_SURFACE_H__
