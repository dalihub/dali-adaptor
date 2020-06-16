#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/signals/connection-tracker.h>
#include <dali/public-api/signals/dali-signal.h>
#include <dali/integration-api/scene.h>
#include <unistd.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/egl-interface.h>
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/system/common/file-descriptor-monitor.h>

namespace Dali
{

class TriggerEventInterface;

namespace Internal
{
namespace Adaptor
{

class WindowBase;
class AdaptorInternalServices;

/**
 * Window interface of render surface.
 */
class WindowRenderSurface : public Dali::RenderSurfaceInterface, public ConnectionTracker
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

public: // from Dali::RenderSurfaceInterface

  /**
   * @copydoc Dali::RenderSurfaceInterface::GetPositionSize()
   */
  virtual PositionSize GetPositionSize() const override;

  /**
   */
  virtual void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::InitializeGraphics()
   */
  virtual void InitializeGraphics() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::CreateSurface()
   */
  virtual void CreateSurface() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::DestroySurface()
   */
  virtual void DestroySurface() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::ReplaceGraphicsSurface()
   */
  virtual bool ReplaceGraphicsSurface() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::MoveResize()
   */
  virtual void MoveResize( Dali::PositionSize positionSize) override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::StartRender()
   */
  virtual void StartRender() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::PreRender()
   */
  virtual bool PreRender( bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect ) override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::PostRender()
   */
  virtual void PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface, const std::vector<Rect<int>>& damagedRects ) override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::StopRender()
   */
  virtual void StopRender() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::SetThreadSynchronization
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization ) override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::ReleaseLock()
   */
  virtual void ReleaseLock() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::GetSurfaceType()
   */
  virtual Dali::RenderSurfaceInterface::Type GetSurfaceType() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::MakeContextCurrent()
   */
  virtual void MakeContextCurrent() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::GetDepthBufferRequired()
   */
  virtual Integration::DepthBufferAvailable GetDepthBufferRequired() override;

  /**
   * @copydoc Dali::RenderSurfaceInterface::GetStencilBufferRequired()
   */
  virtual Integration::StencilBufferAvailable GetStencilBufferRequired() override;

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

  /**
   * @brief Used as the callback for the frame rendered / presented.
   */
  void ProcessFrameCallback();

  /**
   * @brief Called when our event file descriptor has been written to.
   * @param[in] eventBitMask bit mask of events that occured on the file descriptor
   * @param[in] fileDescriptor The file descriptor
   */
  void OnFileDescriptorEventDispatched( FileDescriptorMonitor::EventType eventBitMask, int fileDescriptor );

protected:

  // Undefined
  WindowRenderSurface(const WindowRenderSurface&) = delete;

  // Undefined
  WindowRenderSurface& operator=(const WindowRenderSurface& rhs) = delete;

private:

  struct FrameCallbackInfo
  {
    FrameCallbackInfo( Dali::Integration::Scene::FrameCallbackContainer& callbackList, int fd )
    : callbacks(),
      fileDescriptorMonitor(),
      fileDescriptor( fd )
    {
      // Transfer owership of the CallbackBase
      for( auto&& iter : callbackList )
      {
        callbacks.push_back( std::make_pair( std::move( iter.first ), iter.second ) );
      }
    }

    ~FrameCallbackInfo()
    {
      // Delete FileDescriptorMonitor before close fd.
      fileDescriptorMonitor.release();
      close( fileDescriptor );
    }

    Dali::Integration::Scene::FrameCallbackContainer callbacks;
    std::unique_ptr< FileDescriptorMonitor > fileDescriptorMonitor;
    int fileDescriptor;
  };

  using FrameCallbackInfoContainer = std::vector< std::unique_ptr< FrameCallbackInfo > >;

private: // Data

  EglInterface*                   mEGL;
  Dali::DisplayConnection*        mDisplayConnection;
  PositionSize                    mPositionSize;       ///< Position
  std::unique_ptr< WindowBase >   mWindowBase;
  ThreadSynchronizationInterface* mThreadSynchronization;
  TriggerEventInterface*          mRenderNotification; ///< Render notification trigger
  TriggerEventInterface*          mRotationTrigger;
  std::unique_ptr< TriggerEventInterface > mFrameRenderedTrigger;
  GraphicsInterface*              mGraphics;           ///< Graphics interface
  EGLSurface                      mEGLSurface;
  EGLContext                      mEGLContext;
  ColorDepth                      mColorDepth;         ///< Color depth of surface (32 bit or 24 bit)
  OutputSignalType                mOutputTransformedSignal;
  FrameCallbackInfoContainer      mFrameCallbackInfoContainer;
  int                             mRotationAngle;
  int                             mScreenRotationAngle;
  bool                            mOwnSurface;         ///< Whether we own the surface (responsible for deleting it)
  bool                            mRotationSupported;
  bool                            mRotationFinished;
  bool                            mScreenRotationFinished;
  bool                            mResizeFinished;

  uint32_t                        mDpiHorizontal;
  uint32_t                        mDpiVertical;

}; // class WindowRenderSurface

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
