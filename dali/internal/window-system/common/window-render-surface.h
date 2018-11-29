#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H

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
#include <dali/integration-api/render-surface.h>
#include <dali/integration-api/egl-interface.h>
#include <dali/internal/graphics/common/graphics-interface.h>

// EXTERNAL INCLUDES
#include <dali/public-api/signals/connection-tracker.h>
#include <dali/public-api/signals/dali-signal.h>


namespace Dali
{

class TriggerEventInterface;

namespace Internal
{
namespace Adaptor
{

class WindowBase;

/**
 * Window interface of render surface.
 */
class WindowRenderSurface : public Dali::RenderSurface, public ConnectionTracker
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
  virtual ~WindowRenderSurface();

public: // API

  /**
   * @brief Get the native window handle
   * @return The native window handle
   */
  Any GetNativeWindow();

  /**
   * @brief Get the native window id
   * @return The native window id
   */
  int GetNativeWindowId();

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

public: // from Dali::RenderSurface

  /**
   * @copydoc Dali::RenderSurface::GetPositionSize()
   */
  virtual PositionSize GetPositionSize() const override;

  /**
   */
  virtual void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) override;

  /**
   * @copydoc Dali::RenderSurface::InitializeGraphics()
   */
  virtual void InitializeGraphics( GraphicsInterface& graphics, Dali::DisplayConnection& displayConnection ) override;

  /**
   * @copydoc Dali::RenderSurface::CreateSurface()
   */
  virtual void CreateSurface() override;

  /**
   * @copydoc Dali::RenderSurface::DestroySurface()
   */
  virtual void DestroySurface() override;

  /**
   * @copydoc Dali::RenderSurface::ReplaceGraphicsSurface()
   */
  virtual bool ReplaceGraphicsSurface() override;

  /**
   * @copydoc Dali::RenderSurface::MoveResize()
   */
  virtual void MoveResize( Dali::PositionSize positionSize) override;

  /**
   * @copydoc Dali::RenderSurface::SetViewMode()
   */
  virtual void SetViewMode( ViewMode viewMode ) override;

  /**
   * @copydoc Dali::RenderSurface::StartRender()
   */
  virtual void StartRender() override;

  /**
   * @copydoc Dali::RenderSurface::PreRender()
   */
  virtual bool PreRender( bool resizingSurface ) override;

  /**
   * @copydoc Dali::RenderSurface::PostRender()
   */
  virtual void PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface );

  /**
   * @copydoc Dali::RenderSurface::StopRender()
   */
  virtual void StopRender() override;

  /**
   * @copydoc Dali::RenderSurface::SetThreadSynchronization
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization ) override;

  /**
   * @copydoc Dali::RenderSurface::ReleaseLock()
   */
  virtual void ReleaseLock() override;

  /**
   * @copydoc Dali::RenderSurface::GetSurfaceType()
   */
  virtual RenderSurface::Type GetSurfaceType() override;

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

}; // class WindowRenderSurface

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
