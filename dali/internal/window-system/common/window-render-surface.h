#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H

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

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/scene.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <dali/public-api/signals/dali-signal.h>
#include <unistd.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/common/surface-factory.h>
#include <dali/internal/system/common/file-descriptor-monitor.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class WindowBase;
class AdaptorInternalServices;

/**
 * Window interface of render surface.
 *
 * Needs exporting as it's called by the Graphics Libraries
 */
class DALI_ADAPTOR_API WindowRenderSurface : public Dali::Integration::RenderSurfaceInterface,
                                             public ConnectionTracker,
                                             public Graphics::NativeWindowInterface
{
public:
  using RotationFinishedSignalType = Signal<void()>; ///< The signal of window rotation's finished.
  using DamagedRectsContainer      = std::vector<Rect<int>>;

  /**
   * @brief Uses an window surface to render to.
   *
   * @param [in] positionSize the position and size of the surface
   * @param [in] surface can be a window or pixmap.
   * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
   */
  WindowRenderSurface(Dali::PositionSize positionSize, Any surface, bool isTransparent = false);

  /**
   * @brief Destructor
   */
  virtual ~WindowRenderSurface();

  /**
   * @brief Second phase initialize after adaptor created
   * @note Could be called multiple times
   */
  void Initialize();

public: // API
  /**
   * @brief Get the native window handle
   *
   * @return The native window handle
   */
  Any GetNativeWindow();

  /**
   * @brief Get the native window id
   *
   * @return The native window id
   */
  int GetNativeWindowId();

public: // API
  Graphics::SurfaceId GetSurfaceId() const
  {
    return mSurfaceId;
  }

  /**
   * @brief Map window
   */
  void Map();

  /**
   * @brief Sets the render notification trigger to call when render thread is completed a frame
   *
   * @param renderNotification to use
   */
  void SetRenderNotification(TriggerEventInterface* renderNotification);

  /**
   * @brief Sets whether the surface is transparent or not.
   *
   * @param[in] transparent Whether the surface is transparent
   */
  void SetTransparency(bool transparent);

  /**
   * @brief Request surface rotation
   *
   * @param[in] angle A new angle of the surface
   * @param[in] positionSize A new position and size of the surface
   */
  void RequestRotation(int angle, PositionSize positionSize);

  /**
   * @brief Gets the window base object
   *
   * @return The window base object
   */
  WindowBase* GetWindowBase();

  /**
   * @brief Intiailize Ime Surface for Ime window rendering.
   * It sets one flag and callback function for Ime window rendering
   * This callback function calls one special native window function for ready to commit buffer.
   * The special function notify to display server.
   * It is only used for Ime window.
   */
  void InitializeImeSurface();

  /**
   * @brief Sets the necessary for window rotation acknowledge.
   * After this function called, SendRotationCompletedAcknowledgement() should be called to complete window rotation.
   * More detail description is written in DevelWindow::SetNeedsRotationCompletedAcknowledgement().
   *
   * @param[in] window The window instance.
   * @param[in] needAcknowledgement the flag is true if window rotation acknowledgement is sent.
   */
  void SetNeedsRotationCompletedAcknowledgement(bool needAcknowledgement);

  /**
   * @brief Updates window surface's position and size.
   * It is just to update the local variable in window surface.
   * This function is only called when window's position or size is changed by display server.
   *
   * @param[in] positionSize The updated window surface's position and size.
   */
  void UpdatePositionSize(Dali::PositionSize positionSize);

  /**
   * @brief Moves the window to the specified position.
   *
   * @param positionSize The new position of the window.
   */
  void Move(Dali::PositionSize positionSize);

  /**
   * @brief This signal is emitted when the output is transformed.
   */
  WindowBase::OutputSignalType& OutputTransformedSignal();

  /**
   * @brief This signal is emitted when a rotation job is finished.
   */
  RotationFinishedSignalType& RotationFinishedSignal();

  /**
   * @brief Sets front buffer rendering flag
   *
   * @param[in] enable The flag for front buffer rendering
   */
  void SetFrontBufferRendering(bool enable);

public: // from Dali::Integration::RenderSurfaceInterface
  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetPositionSize()
   */
  PositionSize GetPositionSize() const override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetDpi()
   */
  void GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetSurfaceOrientation()
   */
  int GetSurfaceOrientation() const override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetScreenOrientation()
   */
  int GetScreenOrientation() const override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::InitializeGraphics()
   */
  void InitializeGraphics() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::CreateSurface()
   */
  void CreateSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::DestroySurface()
   */
  void DestroySurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::ReplaceGraphicsSurface()
   */
  bool ReplaceGraphicsSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::MoveResize()
   */
  void MoveResize(Dali::PositionSize positionSize) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::Resize()
   */
  void Resize(Uint16Pair size) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::StartRender()
   */
  void StartRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::PreRender()
   */
  bool PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::PostRender()
   */
  void PostRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::StopRender()
   */
  void StopRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::SetThreadSynchronization
   */
  void SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::ReleaseLock()
   */
  void ReleaseLock() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetSurfaceType()
   */
  Dali::Integration::RenderSurfaceInterface::Type GetSurfaceType() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::MakeContextCurrent()
   */
  void MakeContextCurrent() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetDepthBufferRequired()
   */
  Integration::DepthBufferAvailable GetDepthBufferRequired() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetStencilBufferRequired()
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired() override;

private:
  /**
   * @brief Second stage construction
   */
  void Initialize(Any surface);

  /**
   * @brief Notify output is transformed.
   *
   * @param screenRotationAngle The new screen rotation angle.
   */
  void OutputTransformed(int screenRotationAngle);

  /**
   * @brief Used as the callback for the post render.
   * It is used both window rotation and supporting Ime window
   */
  void ProcessPostRender();

  /**
   * @brief Used as the callback for the frame rendered / presented.
   */
  void ProcessFrameCallback();

  /**
   * @brief Called when our event file descriptor has been written to.
   *
   * @param[in] eventBitMask bit mask of events that occured on the file descriptor
   * @param[in] fileDescriptor The file descriptor
   */
  void OnFileDescriptorEventDispatched(FileDescriptorMonitor::EventType eventBitMask, int fileDescriptor);

  /**
   * @brief Set the buffer damage rects.
   *
   * @param[in] damagedRects List of damaged rects
   * @param[in] clippingRect The rect to clip rendered scene
   */
  void SetBufferDamagedRects(const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect);

  /**
   * @brief Swap buffers.
   *
   * @param[in] damagedRects List of damaged rects
   */
  void SwapBuffers(const std::vector<Rect<int>>& damagedRects);

protected:
  // Undefined
  WindowRenderSurface(const WindowRenderSurface&) = delete;

  // Undefined
  WindowRenderSurface& operator=(const WindowRenderSurface& rhs) = delete;

private:
  struct FrameCallbackInfo
  {
    FrameCallbackInfo(Dali::Integration::Scene::FrameCallbackContainer& callbackList, int fd)
    : callbacks(),
      fileDescriptorMonitor(),
      fileDescriptor(fd)
    {
      // Transfer owership of the CallbackBase
      for(auto&& iter : callbackList)
      {
        callbacks.push_back(std::make_pair(std::move(iter.first), iter.second));
      }
    }

    ~FrameCallbackInfo()
    {
      // Delete FileDescriptorMonitor before close fd.
      fileDescriptorMonitor.reset();
      close(fileDescriptor);
    }

    Dali::Integration::Scene::FrameCallbackContainer callbacks;
    std::unique_ptr<FileDescriptorMonitor>           fileDescriptorMonitor;
    int                                              fileDescriptor;
  };

  using FrameCallbackInfoContainer = std::vector<std::unique_ptr<FrameCallbackInfo>>;

private: // Data
  Dali::DisplayConnection*             mDisplayConnection;
  PositionSize                         mPositionSize; ///< Position
  std::unique_ptr<WindowBase>          mWindowBase;
  ThreadSynchronizationInterface*      mThreadSynchronization;
  TriggerEventInterface*               mRenderNotification; ///< Render notification trigger
  TriggerEventFactory::TriggerEventPtr mPostRenderTrigger;  ///< Post render callback function
  TriggerEventFactory::TriggerEventPtr mFrameRenderedTrigger;
  Dali::Graphics::GraphicsInterface*   mGraphics;                     ///< Graphics interface
  ColorDepth                           mColorDepth;                   ///< Color depth of surface (32 bit or 24 bit)
  WindowBase::OutputSignalType         mOutputTransformedSignal;      ///< The signal of screen rotation occurs
  RotationFinishedSignalType           mWindowRotationFinishedSignal; ///< The signal of window rotation's finished
  FrameCallbackInfoContainer           mFrameCallbackInfoContainer;
  DamagedRectsContainer                mBufferDamagedRects;
  Dali::Mutex                          mMutex;
  Graphics::SurfaceId                  mSurfaceId{Graphics::INVALID_SURFACE_ID};
  int                                  mWindowRotationAngle;
  int                                  mScreenRotationAngle;
  uint32_t                             mDpiHorizontal;
  uint32_t                             mDpiVertical;
  std::vector<Rect<int>>               mDamagedRects{}; ///< Keeps collected damaged render items rects for one render pass. These rects are rotated by scene orientation.
  bool                                 mOwnSurface;     ///< Whether we own the surface (responsible for deleting it)
  bool                                 mIsImeWindowSurface;
  bool                                 mNeedWindowRotationAcknowledgement;
  bool                                 mIsWindowOrientationChanging;
  bool                                 mIsFrontBufferRendering;
  bool                                 mIsFrontBufferRenderingChanged;

}; // class WindowRenderSurface

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
