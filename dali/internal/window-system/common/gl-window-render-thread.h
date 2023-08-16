#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_RENDER_THREAD_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_RENDER_THREAD_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/devel-api/threading/thread.h>
#include <dali/integration-api/adaptor-framework/log-factory-interface.h>
#include <dali/integration-api/adaptor-framework/trace-factory-interface.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/common/window-base.h>

namespace Dali
{
class Adaptor;
class TriggerEventInterface;

namespace Internal
{
namespace Adaptor
{
class WindowBase;
class GlWindow;

/**
 * @brief It is for render thread for GlWindow.
 * User callbacks works in the thread.
 *
 * Key Points:
 *  1. Two Threads:
 *    a. Main/Event Thread.
 *    b. Render Thread.
 *  2. There is NO VSync thread:
 *    a. We calculate the difference between these two times and if:
 *      i.  The difference is less than the default frame time, we sleep.
 *      ii. If it’s more or the same, we continue.
 *  3. Support Rendering mode
 *    a. CONTINUOUS mode
 *      i. The rendering loop works continuously.
 *    b. ON_DEMAND mode
 *      i. The rendering works by user's request.
 *      ii. User's request is the renderOnce()'s function calling.
 */

class GlWindowRenderThread : public Dali::Thread
{
public:
  /**
   * @brief Enumeration for GlWindow Surface status type
   * It has the status as resized, window is rotated and screen is rotated.
   *
   */
  enum class SurfaceStatus
  {
    NO_CHANGED     = 0x00, ///< no changed,
    RESIZED        = 0x01, ///< When surface is resized,
    WINDOW_ROTATED = 0x02, ///< When window is rotated,
    SCREEN_ROTATED = 0x04  ///< When screen is rotated,
  };

  /**
   * Constructor
   *
   * @param[in] positionSize The position and size of the physical window
   * @param[in] depth color depth of the physical window
   */
  GlWindowRenderThread(PositionSize positionSize, ColorDepth colorDepth);

  /**
   * destructor.
   */
  virtual ~GlWindowRenderThread();

  /**
   * Sets the GraphicsInterface instance.
   * This graphics instance is used to create and initialize graphics resource
   *
   * @param[in]  graphics           The graphice instance
   */
  void SetGraphicsInterface(GraphicsInterface* graphics);

  /**
   * Sets the WindowBase instance
   * This WindowBase instance is used to call wl egl window APIs.
   *
   * @param[in]  windowBase           The WindowBase instance
   */
  void SetWindowBase(WindowBase* windowBase);

  /**
   * @brief Sets graphics configuration for GlWindow
   *
   * @param[in] depth the flag of depth buffer. If true is set, 24bit depth buffer is enabled.
   * @param[in] stencil the flag of stencil. it true is set, 8bit stencil buffer is enabled.
   * @param[in] msaa the bit of msaa.
   * @param[in] version the GLES version.
   *
   */
  void SetGraphicsConfig(bool depth, bool stencil, int msaa, int version);

  /**
   * Pauses the Render Thread.
   * It is called when GlWindow is iconified or hidden.
   *
   * This will lock the mutex in mRenderThreadWaitCondition.
   */
  void Pause();

  /**
   * Resumes the Render Thread.
   * It is called when GlWindow is de-iconified or shown.
   *
   * This will lock the mutex in mRenderThreadWaitCondition.
   */
  void Resume();

  /**
   * Stops the Render Thread.
   * This will lock the mutex in mRenderThreadWaitCondition.
   *
   * @note Should only be called in Stop as calling this will kill the render thread.
   */
  void Stop();

  /**
   * @copydoc Dali::GlWindow::RegisterGlCallbacks()
   */
  void RegisterGlCallbacks(CallbackBase* initCallback, CallbackBase* renderFrameCallback, CallbackBase* terminateCallback);

  /**
   * Enable OnDemand Rendering Mode
   *
   * @param[in] onDemand the flag of OnDemand Rendering Mode. If the flag is true, rendering mode is OnDemand, otherwise the flag is false, rendering mode is continuous mode.
   */
  void SetOnDemandRenderMode(bool onDemand);

  /**
   * @copydoc Dali::GlWindow::RenderOnce()
   */
  void RenderOnce();

  /**
   * Requests the window resize to GlWindow's render thread.
   *
   * @param[in] width new width.
   * @param[in] height new height.
   */
  void RequestWindowResize(int width, int height);

  /**
   * Requests the window rotation to GlWindow's render thread.
   *
   * @param[in] windowAngle the window rotation's angle as 0, 90, 180 and 270.
   */
  void RequestWindowRotate(int windowAngle);

  /**
   * Requests the screen rotation to GlWindow's render thread.
   *
   * @param[in] screenAngle the screen rotation's angle as 0, 90, 180 and 270.
   */
  void RequestScreenRotate(int screenAngle);

protected:
  /**
   * The Render thread loop. This thread will be destroyed on exit from this function.
   */
  virtual void Run();

private:
  /**
   * @brief Initialize and create EGL resource
   */
  void InitializeGraphics(EglGraphics* eglGraphics);

  /**
   * Called by the Render Thread which ensures a wait if required.
   *
   * @param[out] timeToSleepUntil  The time remaining in nanoseconds to keep the thread sleeping before resuming.
   * @return false, if the thread should stop.
   */
  bool RenderReady(uint64_t& timeToSleepUntil);

  /**
   * In the Tizen world, when GlWindow rotation is finished in client side,
   * the completed message should be sent to display server.
   * This function should be called in the event thread after buffer is committed.
   */
  void WindowRotationCompleted();

  /**
   * Gets window's current surface status
   *
   * @brief This function return window's current surface status.
   * The status has the  the information of window resized, window rotated and screen rotated.
   * After called, the status value is reset
   *
   * @[output] windowRotationAngle return current window rotation angle.
   * @[output] screenRotationAngle return current screen rotation angle.
   * @return the window's current surface status.
   */
  unsigned int GetSurfaceStatus(int& windowRotationAngle, int& screenRotationAngle);

  /**
   * Starts post rendering process
   *
   * @brief Starts post rendering process for window rotation
   * It is to pause the render thread until maint thread finishes the window rotation work.
   */
  void PostRenderStart();

  /**
   * Finishs post rendering process
   *
   * @brief Finishes post rendering process for window rotation
   * It set the resume flag for resume the render thread.
   */
  void PostRenderFinish();

  /**
   * Pauses the render thread unitil post rendering process
   *
   * @brief Pauses the render thread until main thread works window rotation.
   */
  void PostRenderWaitForFinished();

private:
  GraphicsInterface*                     mGraphics; ///< Graphics interface
  WindowBase*                            mWindowBase;
  std::unique_ptr<TriggerEventInterface> mWindowRotationTrigger;

  const Dali::LogFactoryInterface&   mLogFactory;
  const Dali::TraceFactoryInterface& mTraceFactory;

  PositionSize mPositionSize; ///< Position
  ColorDepth   mColorDepth;

  // EGL, GL Resource
  std::unique_ptr<CallbackBase> mGLInitCallback;
  std::unique_ptr<CallbackBase> mGLRenderFrameCallback;
  std::unique_ptr<CallbackBase> mGLTerminateCallback;
  EGLSurface                    mEGLSurface;
  EGLContext                    mEGLContext;
  bool                          mDepth : 1;
  bool                          mStencil : 1;
  bool                          mIsEGLInitialize : 1;
  int                           mGLESVersion;
  int                           mMSAA;
  int                           mWindowRotationAngle; ///< The angle of window rotation angle
  int                           mScreenRotationAngle; ///< The angle of screen rotation angle

  // To manage the render/main thread
  ConditionalWait       mRenderThreadWaitCondition; ///< The wait condition for the update-render-thread.
  volatile unsigned int mDestroyRenderThread;       ///< Stop render thread. It means this rendter thread will be destoried.
  volatile unsigned int mPauseRenderThread;         ///< Sleep render thread by pause.
  volatile unsigned int mRenderingMode;             ///< Rendering Mode, 0: continuous, 1:OnDemad
  volatile unsigned int mRequestRenderOnce;         ///< Request rendering once
  volatile unsigned int mSurfaceStatus;             ///< When surface is changed as resized or rotated, this flag is set. 0: No changed, 1:resized, 2:window rotation, 4:screen rotation
  volatile unsigned int mPostRendering;             ///< Whether post-rendering is taking place (set by the event & render threads, read by the render-thread).

  uint64_t mDefaultFrameDurationNanoseconds; ///< Default duration of a frame (used for sleeping if not enough time elapsed). Not protected by lock, but written to rarely so not worth adding a lock when reading.

}; // GlWindowRenderThread

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif
