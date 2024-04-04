#pragma once

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
#include <dali/integration-api/render-surface.h>
#include <dali/integration-api/core-enumerations.h>
#include <dali/public-api/math/vector4.h>

// INTERNAL INCLUDES

namespace Dali
{

enum ColorDepth
{
  COLOR_DEPTH_24 = 24,
  COLOR_DEPTH_32 = 32
};

class DisplayConnection;
class ThreadSynchronizationInterface;

namespace Internal::Adaptor
{
class AdaptorInternalServices;
class GraphicsInterface;
}

class RenderSurfaceInterface : public Dali::Integration::RenderSurface
{
public:

  /**
   * @brief Constructor
   * Inlined as this is a pure abstract interface
   */
  RenderSurfaceInterface()
  : mAdaptor( nullptr ),
    mGraphics( nullptr ),
    mDisplayConnection( nullptr ),
    mDepthBufferRequired( Integration::DepthBufferAvailable::FALSE ),
    mStencilBufferRequired( Integration::StencilBufferAvailable::FALSE ),
    mBackgroundColor()
  {}

  /**
   * @brief Virtual Destructor.
   * Inlined as this is a pure abstract interface
   */
  ~RenderSurfaceInterface() override = default;

  /**
   * @brief Return the size and position of the surface.
   * @return The position and size
   */
  [[nodiscard]] Dali::PositionSize GetPositionSize() const override = 0;

  /**
   * @brief Get DPI
   * @param[out] dpiHorizontal set to the horizontal dpi
   * @param[out] dpiVertical set to the vertical dpi
   */
  void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) override = 0;

  /**
   * @brief InitializeGraphics the platform specific graphics surface interfaces
   */
  virtual void InitializeGraphics( Internal::Adaptor::GraphicsInterface& graphics ) = 0;

  /**
   * @brief Creates the Surface
   */
  void CreateSurface() override = 0;

  /**
   * @brief Destroys the Surface
   */
  void DestroySurface() override = 0;

  /**
   * @brief Replace the Surface
   * @return true if context was lost
   */
  bool ReplaceGraphicsSurface() override = 0;

  /**
   * @brief Resizes the underlying surface.
   * @param[in] The dimensions of the new position
   */
  void MoveResize( Dali::PositionSize positionSize ) override = 0;

  /**
   * @brief Called when Render thread has started
   */
  void StartRender() override = 0;

  /**
   * @brief Invoked by render thread before Core::Render
   * If the operation fails, then Core::Render should not be called until there is
   * a surface to render onto.
   * @param[in] resizingSurface True if the surface is being resized
   * @return True if the operation is successful, False if the operation failed
   */
  bool PreRender( bool resizingSurface ) override = 0;

  /**
   * @brief Invoked by render thread after Core::Render
   */
  void PostRender( ) override = 0;
  /**
   * @brief Invoked by render thread when the thread should be stop
   */
  void StopRender() override = 0;

  /**
   * @brief Invoked by Event Thread when the compositor lock should be released and rendering should resume.
   */
  void ReleaseLock() override = 0;

  /**
   * @brief Sets the ThreadSynchronizationInterface
   *
   * @param threadSynchronization The thread-synchronization implementation.
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization ) = 0;

  /**
   * @brief Gets the surface type
   */
  Dali::Integration::RenderSurface::Type GetSurfaceType() override = 0;

  /**
   * @brief Makes the graphics context current
   */
  void MakeContextCurrent() override = 0;

  /**
   * @brief Get whether the depth buffer is required
   * @return TRUE if the depth buffer is required
   */
  Integration::DepthBufferAvailable GetDepthBufferRequired() override = 0;

  /**
   * @brief Get whether the stencil buffer is required
   * @return TRUE if the stencil buffer is required
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired() override = 0;

  /**
   * @brief Sets the background color of the surface.
   * @param[in] color The new background color
   */
  void SetBackgroundColor( Vector4 color ) override
  {
    mBackgroundColor = color;
  }

  /**
   * @brief Gets the background color of the surface.
   * @return The background color
   */
  Vector4 GetBackgroundColor() override
  {
    return mBackgroundColor;
  }

public:

  void SetAdaptor( Dali::Internal::Adaptor::AdaptorInternalServices& adaptor )
  {
    mAdaptor = &adaptor;
  }

  void SetGraphicsInterface( Internal::Adaptor::GraphicsInterface& graphics )
  {
    mGraphics = &graphics;
  }

  void SetDisplayConnection( Dali::DisplayConnection& displayConnection )
  {
    mDisplayConnection = &displayConnection;
  }

public:
  /**
   * @brief Undefined copy constructor. RenderSurface cannot be copied
   */
  RenderSurfaceInterface( const RenderSurfaceInterface& rhs )=delete;

  /**
   * @brief Undefined assignment operator. RenderSurface cannot be copied
   */
  RenderSurfaceInterface& operator=( const RenderSurfaceInterface& rhs )=delete;

protected:

  Dali::Internal::Adaptor::AdaptorInternalServices* mAdaptor;
  Dali::Internal::Adaptor::GraphicsInterface* mGraphics;
  Dali::DisplayConnection* mDisplayConnection;

private:

  Integration::DepthBufferAvailable mDepthBufferRequired;       ///< Whether the depth buffer is required
  Integration::StencilBufferAvailable mStencilBufferRequired;   ///< Whether the stencil buffer is required

  Vector4 mBackgroundColor;                                     ///< The background color of the surface
};

} // namespace Dali
