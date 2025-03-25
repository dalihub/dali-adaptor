#ifndef DALI_INTEGRATION_RENDER_SURFACE_INTERFACE_H
#define DALI_INTEGRATION_RENDER_SURFACE_INTERFACE_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/dali-adaptor-common.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/core-enumerations.h>
#include <dali/integration-api/scene.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/public-api/math/vector4.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/weak-handle.h>

namespace Dali
{
class DisplayConnection;
class ThreadSynchronizationInterface;

namespace Graphics
{
class GraphicsInterface;
}

namespace Internal::Adaptor
{
class AdaptorInternalServices;
} // namespace Internal::Adaptor

/**
 * @brief The position and size of the render surface.
 */
using PositionSize = Dali::Rect<int>;
using SurfaceSize  = Uint16Pair;

namespace Integration
{
struct DALI_ADAPTOR_API RenderSurface
{
};

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
class RenderSurfaceInterface : public Integration::RenderSurface
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
  RenderSurfaceInterface()
  : mAdaptor(nullptr),
    mGraphics(nullptr),
    mDisplayConnection(nullptr),
    mScene(),
    mFullSwapNextFrame(true),
    mDepthBufferRequired(Integration::DepthBufferAvailable::FALSE),
    mStencilBufferRequired(Integration::StencilBufferAvailable::FALSE)
  {
  }

  /**
   * @brief Virtual Destructor.
   * Inlined as this is a pure abstract interface
   */
  virtual ~RenderSurfaceInterface()
  {
  }

  /**
   * @brief Return the size and position of the surface.
   * @return The position and size
   */
  virtual Dali::PositionSize GetPositionSize() const = 0;

  /**
   * @brief Get DPI
   * @param[out] dpiHorizontal set to the horizontal dpi
   * @param[out] dpiVertical set to the vertical dpi
   */
  virtual void GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical) = 0;

  /**
   * @brief Return the orientation of the surface.
   * @return The orientation
   */
  virtual int GetSurfaceOrientation() const = 0;

  /**
   * @brief Return the orientation of the screen.
   * @return The screen orientation
   */
  virtual int GetScreenOrientation() const = 0;

  /**
   * @brief InitializeGraphics the platform specific graphics surface interfaces
   */
  virtual void InitializeGraphics() = 0;

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
   * @param[in] positionSize The dimensions of the new position
   */
  virtual void MoveResize(Dali::PositionSize positionSize) = 0;

  /**
   * @brief Sets the size of the surface.
   * @param[in] size The new size of the surface
   */
  virtual void Resize(Uint16Pair size) = 0;

  /**
   * @brief Called when Render thread has started
   */
  virtual void StartRender() = 0;

  /**
   * @brief Invoked by render thread before Core::RenderScene
   * If the operation fails, then Core::Render should not be called until there is
   * a surface to render onto.
   * @param[in] resizingSurface True if the surface is being resized
   * @param[in] damagedRects List of damaged rects this render pass
   * @return True if the operation is successful, False if the operation failed
   */
  virtual bool PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect) = 0;

  /**
   * @brief Invoked by render thread after Core::RenderScene
   */
  virtual void PostRender() = 0;

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
  virtual void SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization) = 0;

  /**
   * @brief Gets the surface type
   */
  virtual Dali::Integration::RenderSurfaceInterface::Type GetSurfaceType() = 0;

  /**
   * @brief Makes the graphics context current
   */
  virtual void MakeContextCurrent() = 0;

  /**
   * @brief Get whether the depth buffer is required
   * @return TRUE if the depth buffer is required
   */
  virtual Integration::DepthBufferAvailable GetDepthBufferRequired() = 0;

  /**
   * @brief Get whether the stencil buffer is required
   * @return TRUE if the stencil buffer is required
   */
  virtual Integration::StencilBufferAvailable GetStencilBufferRequired() = 0;

public:
  void SetAdaptor(Dali::Internal::Adaptor::AdaptorInternalServices& adaptor)
  {
    mAdaptor = &adaptor;
  }

  void SetGraphicsInterface(Dali::Graphics::GraphicsInterface& graphics)
  {
    mGraphics = &graphics;
  }

  void SetDisplayConnection(Dali::DisplayConnection& displayConnection)
  {
    mDisplayConnection = &displayConnection;
  }

  /**
   * @brief Sets a Scene that is rendered on this surface.
   * @param scene The Scene object
   */
  void SetScene(Dali::Integration::Scene& scene)
  {
    mScene = scene;
  }

  /**
   * @brief Forces full surface swap next frame, resets current partial update state.
   */
  void SetFullSwapNextFrame()
  {
    mFullSwapNextFrame = true;
  }

  bool GetFullSwapNextFrame() const
  {
    return mFullSwapNextFrame;
  }

private:
  /**
   * @brief Undefined copy constructor. RenderSurface cannot be copied
   */
  RenderSurfaceInterface(const RenderSurfaceInterface& rhs);

  /**
   * @brief Undefined assignment operator. RenderSurface cannot be copied
   */
  RenderSurfaceInterface& operator=(const RenderSurfaceInterface& rhs);

protected:
  Dali::Internal::Adaptor::AdaptorInternalServices* mAdaptor;
  Dali::Graphics::GraphicsInterface*                mGraphics;
  Dali::DisplayConnection*                          mDisplayConnection;
  WeakHandle<Dali::Integration::Scene>              mScene;
  bool                                              mFullSwapNextFrame; ///< Whether the full surface swap is required

private:
  Integration::DepthBufferAvailable   mDepthBufferRequired;   ///< Whether the depth buffer is required
  Integration::StencilBufferAvailable mStencilBufferRequired; ///< Whether the stencil buffer is required

  Vector4 mBackgroundColor; ///< The background color of the surface
};

} // Namespace Integration
} // namespace Dali

#endif // DALI_INTEGRATION_RENDER_SURFACE_INTERFACE_H
