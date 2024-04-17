#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H

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

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>


// EXTERNAL INCLUDES
#if !defined(VULKAN_ENABLED)
#include <EGL/egl.h>
#endif
#include <dali/graphics/surface-factory.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <dali/public-api/signals/dali-signal.h>
#include <memory>

namespace Dali
{
namespace Integration
{
class Surface;
} // namespace Integration

class TriggerEventInterface;

namespace Internal::Adaptor
{
class WindowBase;
class AdaptorInternalServices;

/**
 * Window interface of render surface.
 */
class WindowRenderSurface : public Dali::RenderSurfaceInterface, public ConnectionTracker, public Graphics::NativeWindowInterface
{
public:

  typedef Signal< void ( ) > OutputSignalType;

  /**
    * Uses an window surface to render to.
    * @param [in] positionSize the position and size of the surface
    * @param [in] surface can be a window or pixmap.
    * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
    */
  WindowRenderSurface( Dali::PositionSize positionSize, Any surface, bool isTransparent = false );

  /**
   * @brief Destructor
   */
  ~WindowRenderSurface() override;

public: // API

  /**
   * @brief Get the native window handle
   * @return The native window handle
   */
  Any GetNativeWindow() override;

  /**
   * @brief Get the native window id
   * @return The native window id
   */
  int GetNativeWindowId() override;

  /**
   * @brief Map window
   */
  void Map();

  /**
   * @brief Sets the render notification trigger to call when render thread is completed a frame
   * @param renderNotification to use
   */
  void SetRenderNotification( TriggerEventInterface* renderNotification );

  /**
   * @brief Sets whether the surface is transparent or not.
   * @param[in] transparent Whether the surface is transparent
   */
  void SetTransparency( bool transparent );

  /**
   * Request surface rotation
   * @param[in] angle A new angle of the surface
   * @param[in] width A new width of the surface
   * @param[in] height A new height of the surface
   */
  void RequestRotation( int angle, int width, int height );

  /**
   * @brief Gets the window base object
   * @return The window base object
   */
  WindowBase* GetWindowBase();

  /**
   * @brief This signal is emitted when the output is transformed.
   */
  OutputSignalType& OutputTransformedSignal();

public: // from Dali::Integration::RenderSurface

  /**
   * @copydoc Dali::Integration::RenderSurface::GetPositionSize()
   */
  PositionSize GetPositionSize() const override;

  /**
   */
  void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::InitializeGraphics()
   */
  void InitializeGraphics( GraphicsInterface& graphics ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::CreateSurface()
   */
  void CreateSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::DestroySurface()
   */
  void DestroySurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::ReplaceGraphicsSurface()
   */
  bool ReplaceGraphicsSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::MoveResize()
   */
  void MoveResize( Dali::PositionSize positionSize) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::StartRender()
   */
  void StartRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::PreRender()
   */
  bool PreRender( bool resizingSurface ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::PostRender()
   */
  void PostRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::StopRender()
   */
   void StopRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::SetThreadSynchronization
   */
   void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::ReleaseLock()
   */
   void ReleaseLock() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::GetSurfaceType()
   */
   Integration::RenderSurface::Type GetSurfaceType() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::MakeContextCurrent()
   */
   void MakeContextCurrent() override;

  /**
   * @copydoc Dali::Integration::RenderSurfacsudo e::GetDepthBufferRequired()
   */
   Integration::DepthBufferAvailable GetDepthBufferRequired() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::GetStencilBufferRequired()
   */
   Integration::StencilBufferAvailable GetStencilBufferRequired() override;

private:

  /**
   * @brief Second stage construction
   */
  void Initialize( Any surface );

  /**
   * Notify output is transformed.
   */
  void OutputTransformed();

  /**
   * @brief Used as the callback for the rotation-trigger.
   */
  void ProcessRotationRequest();

protected:

  // Undefined
  WindowRenderSurface(const WindowRenderSurface&) = delete;

  // Undefined
  WindowRenderSurface& operator=(const WindowRenderSurface& rhs) = delete;

private: // Data

  Dali::DisplayConnection*        mDisplayConnection;
  PositionSize                    mPositionSize;       ///< Position
  std::unique_ptr< WindowBase >   mWindowBase;
  ThreadSynchronizationInterface* mThreadSynchronization;
  TriggerEventInterface*          mRenderNotification; ///< Render notification trigger
  TriggerEventInterface*          mRotationTrigger;
  GraphicsInterface*              mGraphics;           ///< Graphics interface
  ColorDepth                      mColorDepth;         ///< Color depth of surface (32 bit or 24 bit)
  OutputSignalType                mOutputTransformedSignal;
  int                             mRotationAngle;
  int                             mScreenRotationAngle;
  bool                            mOwnSurface;         ///< Whether we own the surface (responsible for deleting it)
  bool                            mRotationSupported;
  bool                            mRotationFinished;
  bool                            mScreenRotationFinished;
  bool                            mResizeFinished;
#if defined(VULKAN_ENABLED)
  std::unique_ptr<Dali::Graphics::Surface> mGraphicsSurface;
#else
  EGLSurface                      mEGLSurface;         ///< EGL surface
  EGLContext                      mEGLContext;
#endif
}; // class WindowRenderSurface

} // namespace Internal::Adaptor

// namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
