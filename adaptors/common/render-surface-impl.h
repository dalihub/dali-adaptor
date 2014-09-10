#ifndef __DALI_INTERNAL_RENDER_SURFACE_H__
#define __DALI_INTERNAL_RENDER_SURFACE_H__

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
#include <render-surface.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/common/view-mode.h>
#include <base/interfaces/egl-interface.h>

namespace Dali
{

namespace Integration
{

class GlAbstraction;

} // namespace Integration

namespace Internal
{

namespace Adaptor
{

class EglInterface;

/**
 * This is the internal RenderSurface API
 */
class DALI_IMPORT_API RenderSurface : public Dali::RenderSurface
{
public:

  /**
   * Constructor
   */
  RenderSurface();

  /**
   * Destructor
   */
  virtual ~RenderSurface();

public: // API

  /**
   * Initialize EGL, RenderSurface should create egl display and initialize
   * @param egl implementation to use for the creation
   */
  virtual void InitializeEgl( EglInterface& egl ) = 0;

  /**
   * Creates EGL Surface
   * @param egl implementation to use for the creation
   */
  virtual void CreateEglSurface( EglInterface& egl ) = 0;

  /**
   * Destroys EGL Surface
   * @param egl implementation to use for the destruction
   */
  virtual void DestroyEglSurface( EglInterface& egl ) = 0;

  /**
   * Replace the EGL Surface
   * @param egl implementation to use for the creation
   * @return true if context was lost
   */
  virtual bool ReplaceEGLSurface( EglInterface& egl ) = 0;

  /**
   * Resizes the underlying surface.
   * Only available for x window
   */
  virtual void MoveResize( Dali::PositionSize positionSize ) = 0;

  /**
   * Get DPI
   * @param dpiHorizontal set to the horizontal dpi
   * @param dpiVertical set to the vertical dpi
   */
  virtual void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) const = 0;

  /**
   * Call to map the surface (only works if surface is a window)
   */
  virtual void Map() = 0;

  /**
   * Transfers the ownership of a display
   * @param newSurface to transfer
   */
  virtual void TransferDisplayOwner( Internal::Adaptor::RenderSurface& newSurface ) = 0;

  /**
   *  Consumes any possible events on the queue so that there is no leaking between frames
   */
  virtual void ConsumeEvents() = 0;

  /**
   * Set the stereoscopic 3D view mode
   * @param[in] viewMode The new view mode
   */
  virtual void SetViewMode( ViewMode viewMode ) = 0;

  /**
   * Called when Render thread has started
   */
  virtual void StartRender() = 0;

  /**
   * Invoked by render thread before Core::Render
   * @param[in] egl The Egl interface
   * @param[in] glAbstraction OpenGLES abstraction interface
   * @return True if the operation is successful
   */
  virtual bool PreRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction ) = 0;

  /**
   * Invoked by render thread after Core::Render
   * @param[in] egl The Egl interface
   * @param[in] glAbstraction OpenGLES abstraction interface
   * @param[in] deltaTime Time (in microseconds) since PostRender was last called.
   * @param[in] replacingSurface True if the surface is being replaced.
   */
  virtual void PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, unsigned int deltaTime, bool replacingSurface ) = 0;

  /**
   * Invoked by render thread when the thread should be stop
   */
  virtual void StopRender() = 0;

  /**
   * Invoked by Event Thread when the compositor lock should be released and rendering should resume.
   */
  virtual void ReleaseLock() = 0;
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // __DALI_INTERNAL_RENDER_SURFACE_H__
