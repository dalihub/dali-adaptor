#ifndef __DALI_INTERNAL_ECORE_X_RENDER_SURFACE_H__
#define __DALI_INTERNAL_ECORE_X_RENDER_SURFACE_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// EXTERNAL INCLUDES
#include <string>

#include <Ecore_X.h>
#include <dali/public-api/common/dali-common.h>

// INTERNAL INCLUDES
#include <internal/common/render-surface-impl.h>
#include <internal/common/ecore-x/ecore-x-types.h>
#include <internal/common/gl/egl-implementation.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
class TriggerEvent;

namespace ECoreX
{

/**
 * Ecore X11 implementation of render surface.
 * @todo change namespace to ECore_X11 as the class
 * is no longer pure X11.
 */
class DALI_IMPORT_API RenderSurface : public Internal::Adaptor::RenderSurface
{
public:
  /**
    * Uses an X11 surface to render to.
    * @param [in] type the type of surface passed in
    * @param [in] positionSize the position and size of the surface
    * @param [in] surface can be a X-window or X-pixmap (type must be unsigned int).
    * @param [in] display connection to X-server if the surface is a X window or pixmap (type must be void * to X display struct)
    * @param [in] name optional name of surface passed in
    * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
    */
  RenderSurface( SurfaceType type,
                 Dali::PositionSize positionSize,
                 boost::any surface,
                 boost::any display,
                 const std::string& name,
                 bool isTransparent = false);

  /**
   * Destructor.
   * Will delete the display, if it has ownership.
   * Will delete the window/pixmap if it has owner ship
   */
  virtual ~RenderSurface();

protected:
  /**
   * Second stage construction
   * Creates the surface (window, pixmap or native buffer)
   */
  void Init( boost::any surface );

public: // API

  /**
   * @return the Ecore X window handle
   */
  Ecore_X_Window GetXWindow();

  /**
   * @return the Main X display
   */
  XDisplay* GetMainDisplay();

  /**
   * Sets the render notification trigger to call when render thread is completed a frame
   * @param renderNotification to use
   */
  void SetRenderNotification( TriggerEvent* renderNotification );

  /**
   * Get the surface as an Ecore_X_drawable
   */
  virtual Ecore_X_Drawable GetDrawable();

public: // from Dali::RenderSurface

  /**
   * @copydoc Dali::RenderSurface::GetType()
   */
  virtual Dali::RenderSurface::SurfaceType GetType() = 0;

  /**
   * @copydoc Dali::RenderSurface::GetSurface()
   */
  virtual boost::any GetSurface() = 0;

  /**
   * @copydoc Dali::RenderSurface::GetDisplay()
   */
  virtual boost::any GetDisplay();

  /**
   * @copydoc Dali::RenderSurface::GetPositionSize()
   */
  virtual PositionSize GetPositionSize() const;

  /**
   * @copydoc Dali::RenderSurface::SetRenderMode()
   */
  virtual void SetRenderMode(RenderMode mode);

  /**
   * @copydoc Dali::RenderSurface::GetRenderMode()
   */
  virtual RenderMode GetRenderMode() const;

public:  // from Internal::Adaptor::RenderSurface

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::CreateEglSurface()
   */
  virtual void CreateEglSurface( EglInterface& egl ) = 0;

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::DestroyEglSurface()
   */
  virtual void DestroyEglSurface( EglInterface& egl ) = 0;

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::ReplaceEGLSurface()
   */
  virtual bool ReplaceEGLSurface( EglInterface& egl ) = 0;

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::MoveResize()
   */
  virtual void MoveResize( Dali::PositionSize positionSize);

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::GetDpi()
   */
  virtual void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) const;

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::Map()
   */
  virtual void Map();

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::TransferDisplayOwner()
   */
  virtual void TransferDisplayOwner( Internal::Adaptor::RenderSurface& newSurface );

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::PreRender()
   */
  virtual bool PreRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction ) = 0;

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::PostRender()
   */
  virtual bool PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, unsigned int timeDelta ) = 0;

  /**
   * @copydoc Dali::Internal::Adaptor::RenderSurface::StopRender()
   */
  virtual void StopRender() { mIsStopped = true;};

private:

  /**
   * Sets the display, if display parameter is empty it opens a new display
   * @param display
   */
  void SetDisplay( boost::any display );

  /**
   * Get the surface id if the surface parameter is not empty
   * @param surface boost::any containing a surface id, or can be empty
   * @return surface id, or zero if surface is empty
   */
  unsigned int GetSurfaceId( boost::any surface ) const;

protected:

  /**
   * Create XRenderable
   */
  virtual void CreateXRenderable() = 0;

  /**
   * Use an existing render surface
   * @param surfaceId the id of the surface
   */
  virtual void UseExistingRenderable( unsigned int surfaceId ) = 0;

  /**
   * Perform render sync
   * @param[in] currentTime Current time in microseconds
   * @return true if the calling thread should wait for a RenderSync from Adaptor
   */
  bool RenderSync( unsigned int timeDelta );

protected: // Data

  XDisplay*                   mMainDisplay;        ///< X-connection for rendering
  Ecore_X_Window              mRootWindow;         ///< X-Root window
  SurfaceType                 mType;               ///< type of renderable
  PositionSize                mPosition;           ///< Position
  bool                        mOwnSurface;         ///< Whether we own the surface (responsible for deleting it)
  bool                        mOwnDisplay;         ///< Whether we own the display (responsible for deleting it)
  std::string                 mTitle;              ///< Title of window which shows from "xinfo -topvwins" command
  ColorDepth                  mColorDepth;         ///< Color depth of surface (32 bit or 24 bit)
  RenderMode                  mRenderMode;         ///< Render mode for pixmap surface type
  TriggerEvent*               mRenderNotification; ///< Render notificatin trigger
  bool                        mIsStopped;          ///< Render should be stopped

};

} // namespace ECoreX

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // __DALI_INTERNAL_ECORE_X_RENDER_SURFACE_H__
