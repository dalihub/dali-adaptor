#ifndef __DALI_RENDER_SURFACE_WL_H__
#define __DALI_RENDER_SURFACE_WL_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-common.h>
#include <wayland-manager.h>
#include <wayland-egl.h>

// INTERNAL INCLUDES
#include <wayland-window.h>
#include <render-surface.h>
#include <egl-interface.h> // for color depth

namespace Dali
{

class TriggerEventInterface;

namespace Internal
{
namespace Adaptor
{
class WaylandManager;
class WindowEventInterface;
}
}

namespace Wayland
{


/**
 * Wayland render surface.
 */
class RenderSurface : public Dali::RenderSurface
{
public:

  /**
   * @brief Constructor.
   *
   * @param [in] positionSize the position and size of the surface
   * @param [in] surface can be a X-window or X-pixmap (type must be unsigned int).
   * @param [in] name optional name of surface passed in
   * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
   */
  RenderSurface(Dali::PositionSize positionSize,
                       Any surface,
                       const std::string& name,
                       bool isTransparent = false);

  /**
   * @brief Destructor
   */
  virtual ~RenderSurface();

protected:


  /**
   * @Create the surface
   */
  void CreateSurface();

public:

  /**
   * @brief Get window handle
   *
   * @return the wayland window pointer
   */
  Window* GetWindow();

  /**
   * Assigns an event interface to the surface for getting
   * input events / window notifications
   */
  void AssignWindowEventInterface( Dali::Internal::Adaptor::WindowEventInterface* eventInterface );

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
  virtual void SetViewMode( ViewMode viewMode );

  /**
   * @copydoc Dali::RenderSurface::StartRender()
   */
  virtual void StartRender();

  /**
   * @copydoc Dali::RenderSurface::PreRender()
   */
  virtual bool PreRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction );

  /**
   * @copydoc Dali::RenderSurface::PostRender()
   */
  virtual void PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, DisplayConnection* displayConnection, bool replacingSurface );

  /**
   * @copydoc Dali::RenderSurface::StopRender();
   */
  virtual void StopRender();

  /**
   * @copydoc Dali::RenderSurface::ReleaseLock()
   */
  virtual void ReleaseLock();

  /**
   * @copydoc Dali::RenderSurface::SetThreadSynchronization()
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization );

protected: // Data

  Window                      mWindow;
  TriggerEventInterface*      mRenderNotification; ///< Render notification trigger
  Dali::ColorDepth            mColorDepth;         ///< Color depth
  Dali::Internal::Adaptor::WaylandManager*   mWaylandManager; ///< wayland manager
  wl_egl_window*              mEglWindow;

};

} // namespace Wayland

} // namespace Dali

#endif // __DALI_RENDER_SURFACE_WL_H__
