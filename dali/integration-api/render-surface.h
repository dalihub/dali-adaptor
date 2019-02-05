#ifndef DALI_RENDER_SURFACE_H
#define DALI_RENDER_SURFACE_H

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
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/common/view-mode.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/graphics/graphics-interface.h>

namespace Dali
{


class ThreadSynchronizationInterface;

/**
 * @brief The position and size of the render surface.
 */
typedef Dali::Rect<int> PositionSize;

/**
 * @brief Interface for a render surface onto which Dali draws.
 *
 * Dali::Adaptor requires a render surface to draw on to. This is
 * usually a window in the native windowing system, or some other
 * mapped pixel buffer.
 *
 * Dali::Application will automatically create a render surface using a window.
 *
 * The implementation of the factory method below should choose an appropriate
 * implementation of RenderSurface for the given platform
 */

class RenderSurface
{
public:

  enum Type
  {
    WINDOW_RENDER_SURFACE,
    PIXMAP_RENDER_SURFACE,
    NATIVE_RENDER_SURFACE
  };

  /**
   * @brief Constructor
   * Inlined as this is a pure abstract interface
   */
  RenderSurface() {}

  /**
   * @brief Virtual Destructor.
   * Inlined as this is a pure abstract interface
   */
  virtual ~RenderSurface() {}

  /**
   * @brief Return the size and position of the surface.
   * @return The position and size
   */
  virtual PositionSize GetPositionSize() const = 0;

  /**
   * @brief Get DPI
   * @param[out] dpiHorizontal set to the horizontal dpi
   * @param[out] dpiVertical set to the vertical dpi
   */
  virtual void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) = 0;

  /**
   * Initialize Graphics.
   * @param graphics The graphics implementation to use for the creation
   */
  virtual void InitializeGraphics( Graphics::GraphicsInterface& graphics ) = 0;

  /**
   * @brief Creates the Surface
   */
  virtual void CreateSurface() = 0;

  /**
   * @brief Destroys the Surface
   */
  virtual void DestroySurface() = 0;

  /**
   * @brief Replace the Surface
   * @return true if context was lost
   */
  virtual bool ReplaceGraphicsSurface() = 0;

  /**
   * @brief Resizes the underlying surface.
   * @param[in] The dimensions of the new position
   */
  virtual void MoveResize( Dali::PositionSize positionSize ) = 0;

  /**
   * @brief Called when Render thread has started
   */
  virtual void StartRender() = 0;

  /**
   * @brief Invoked by render thread before Core::Render
   * If the operation fails, then Core::Render should not be called until there is
   * a surface to render onto.
   * @param[in] resizingSurface True if the surface is being resized
   * @return True if the operation is successful, False if the operation failed
   */
  virtual bool PreRender( bool resizingSurface ) = 0;

  /**
   * @brief Invoked by render thread after Core::Render
   * @param[in] renderToFbo True if render to FBO.
   * @param[in] replacingSurface True if the surface is being replaced.
   * @param[in] resizingSurface True if the surface is being resized.
   */
  virtual void PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface ) = 0;
  /**
   * @brief Invoked by render thread when the thread should be stop
   */
  virtual void StopRender() = 0;

  /**
   * @brief Invoked by Event Thread when the compositor lock should be released and rendering should resume.
   */
  virtual void ReleaseLock() = 0;

  /**
   * @brief Sets the ThreadSynchronizationInterface
   *
   * @param threadSynchronization The thread-synchronization implementation.
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization ) = 0;

  /**
   * @brief Gets the surface type
   */
  virtual RenderSurface::Type GetSurfaceType() = 0;

private:

  /**
   * @brief Undefined copy constructor. RenderSurface cannot be copied
   */
  RenderSurface( const RenderSurface& rhs );

  /**
   * @brief Undefined assignment operator. RenderSurface cannot be copied
   */
  RenderSurface& operator=( const RenderSurface& rhs );
};

} // namespace Dali

#endif // DALI_RENDER_SURFACE_H
