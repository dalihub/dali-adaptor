#ifndef __DALI_TIZEN_BUFFER_RENDER_SURFACE_H__
#define __DALI_TIZEN_BUFFER_RENDER_SURFACE_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-common.h>

// INTERNAL INCLUDES
#ifdef DALI_ADAPTOR_COMPILATION
#include <render-surface.h>
#include <egl-interface.h>
#else
#include <dali/devel-api/adaptor-framework/render-surface.h>
#include <dali/integration-api/adaptors/egl-interface.h>
#endif

namespace Dali
{

class TriggerEventInterface;

/**
 * Ecore X11 implementation of render surface.
 */
class DALI_IMPORT_API NativeSourceRenderSurface : public Dali::RenderSurface
{
public:

  /**
    * Uses an Wayland surface to render to.
    * @param [in] positionSize the position and size of the surface
    * @param [in] name optional name of surface passed in
    * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
    */
  NativeSourceRenderSurface( Dali::PositionSize positionSize,
                             const std::string& name,
                             bool isTransparent = false );

  /**
   * @copydoc Dali::RenderSurface::~RenderSurface
   */
  virtual ~NativeSourceRenderSurface();

public: // API

  /**
   * @brief Sets the render notification trigger to call when render thread is completed a frame
   *
   * @param renderNotification to use
   */
  void SetRenderNotification( TriggerEventInterface* renderNotification );

  /**
   */
  virtual tbm_surface_h GetDrawable();

  /**
   * @brief GetSurface
   *
   * @return pixmap
   */
  virtual Any GetSurface();

  virtual void ReleaseNativeSource();

public: // from Dali::RenderSurface

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
   * @copydoc Dali::RenderSurface::StopRender()
   */
  virtual void StopRender();

  /**
   * @copydoc Dali::RenderSurface::SetThreadSynchronization
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization );

  virtual RenderSurface::Type GetSurfaceType();

  virtual PositionSize GetPositionSize() const;

  virtual void MoveResize( Dali::PositionSize positionSize );

  virtual void SetViewMode( ViewMode viewMode );

private:

  /**
   * Release any locks.
   */
  void ReleaseLock();

  /**
   * Create XPixmap
   */
  virtual void CreateWlRenderable();

private: // Data

  struct Impl;

  Impl* mImpl;

};

} // namespace Dali

#endif // __DALI_TIZEN_BUFFER_RENDER_SURFACE_H__
