#ifndef __DALI_INTERNAL_RENDER_HELPER_H__
#define __DALI_INTERNAL_RENDER_HELPER_H__

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

// INTERNAL INCLUDES
#include <egl-interface.h>
#include <render-surface.h> // needed for Dali::RenderSurface

namespace Dali
{

class RenderSurface;
class DisplayConnection;

namespace Integration
{
class GlAbstraction;
}

namespace Internal
{
namespace Adaptor
{

class AdaptorInternalServices;
class EglFactoryInterface;

/**
 * Helper class for EGL, surface, pre & post rendering
 */
class RenderHelper
{
public:

  /**
   * Create a RenderHelper.
   * @param[in] adaptorInterfaces base adaptor interface
   */
  RenderHelper( AdaptorInternalServices& adaptorInterfaces );

  /**
   * Non-virtual Destructor
   */
  ~RenderHelper();

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Called on the Event Thread
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Sets up all render related objects to start rendering.
   */
  void Start();

  /**
   * Sets up all render related objects to stop rendering.
   */
  void Stop();

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Called on the Rendering Thread
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Consumes any pending events to avoid memory leaks
   *
   * @note Called from rendering thread
   */
  void ConsumeEvents();

  /**
   * Initializes EGL.
   *
   * @note Called from rendering thread
   */
  void InitializeEgl();

  /**
   * Replaces the rendering surface
   *
   * Used for replacing pixmaps due to resizing
   * @param newSurface to use
   *
   * @note Called from render thread
   */
  void ReplaceSurface( RenderSurface* newSurface );

  /**
   * Shuts down EGL.
   *
   * @note Called from render thread
   */
  void ShutdownEgl();

  /**
   * Called before core renders the scene
   *
   * @return true if successful and Core::Render should be called.
   *
   * @note Called from render thread
   */
  bool PreRender();

  /**
   * Called after core has rendered the scene
   *
   * @note Called from render thread
   */
  void PostRender();

private:

  // Undefined
  RenderHelper( const RenderHelper& RenderHelper );

  // Undefined
  RenderHelper& operator=( const RenderHelper& RenderHelper );

private: // Data

  Integration::GlAbstraction&   mGLES;                   ///< GL abstraction reference
  EglFactoryInterface*          mEglFactory;             ///< Factory class to create EGL implementation
  EglInterface*                 mEGL;                    ///< Interface to EGL implementation
  RenderSurface*                mSurface;                ///< Current surface
  Dali::DisplayConnection*      mDisplayConnection;      ///< Display connection
  bool                          mSurfaceReplaced;        ///< True when new surface has been initialized.
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_RENDER_HELPER_H__
