#ifndef __DALI_ECORE_X_PIXMAP_RENDER_SURFACE_H__
#define __DALI_ECORE_X_PIXMAP_RENDER_SURFACE_H__

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
#include <dali/internal/graphics/gles20/egl-implementation.h>
#include <dali/internal/window-system/common/pixmap-render-surface.h>
#include <dali/internal/window-system/ubuntu-x11/ecore-x-types.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>
#include <dali/devel-api/threading/conditional-wait.h>
#include <Ecore_X.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * Ecore X11 Pixmap implementation of render surface.
 */
class PixmapRenderSurfaceEcoreX : public PixmapRenderSurface
{
public:

  /**
    * Uses an X11 surface to render to.
    * @param [in] positionSize the position and size of the surface
    * @param [in] surface can be a X-window or X-pixmap (type must be unsigned int).
    * @param [in] name optional name of surface passed in
    * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
    */
  PixmapRenderSurfaceEcoreX( Dali::PositionSize positionSize,
                       Any surface,
                       const std::string& name,
                       bool isTransparent = false);

  /**
   * @brief Destructor
   */
  virtual ~PixmapRenderSurfaceEcoreX();

public: // from WindowRenderSurface

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::GetSurface()
   */
  virtual Any GetSurface() override;

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::SetRenderNotification()
   */
  virtual void SetRenderNotification( TriggerEventInterface* renderNotification ) override;

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
  virtual void MoveResize( Dali::PositionSize positionSize) {}

  /**
   * @copydoc Dali::RenderSurface::SetViewMode()
   */
  void SetViewMode( ViewMode viewMode ) {}

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
   * @copydoc Dali::RenderSurface::GetSurfaceType()
   */
  virtual RenderSurface::Type GetSurfaceType();

private: // from PixmapRenderSurface

  /**
   * @copydoc Dali::RenderSurface::ReleaseLock()
   */
  virtual void ReleaseLock();

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::Initialize()
   */
  virtual void Initialize( Any surface ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::Initialize()
   */
  virtual void CreateRenderable() override;

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::Initialize()
   */
  virtual void UseExistingRenderable( unsigned int surfaceId ) override;

private:

  /**
   * Get the surface id if the surface parameter is not empty
   * @param surface Any containing a surface id, or can be empty
   * @return surface id, or zero if surface is empty
   */
  unsigned int GetSurfaceId( Any surface ) const;

private: // Data

  static const int BUFFER_COUNT = 2;

  PositionSize                    mPosition;               ///< Position
  std::string                     mTitle;                  ///< Title of window which shows from "xinfo -topvwins" command
  TriggerEventInterface*          mRenderNotification;     ///< Render notification trigger
  ColorDepth                      mColorDepth;             ///< Color depth of surface (32 bit or 24 bit)
  bool                            mOwnSurface;             ///< Whether we own the surface (responsible for deleting it)

  int                             mProduceBufferIndex;
  int                             mConsumeBufferIndex;
  XPixmap                         mX11Pixmaps[BUFFER_COUNT];  ///< X-Pixmap
  EGLSurface                      mEglSurfaces[BUFFER_COUNT];
  ThreadSynchronizationInterface* mThreadSynchronization;     ///< A pointer to the thread-synchronization
  ConditionalWait                 mPixmapCondition;           ///< condition to share pixmap
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // __DALI_ECORE_X_PIXMAP_RENDER_SURFACE_H__
