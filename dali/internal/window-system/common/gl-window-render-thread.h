#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_RENDER_THREAD_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_RENDER_THREAD_H

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
#include <dali/devel-api/threading/conditional-wait.h>
#include <dali/devel-api/threading/thread.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/common/window-base.h>

namespace Dali
{
class Adaptor;

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
   * @brief Sets egl configuration for GlWindow
   *
   * @param[in] depth the flag of depth buffer. If true is set, 24bit depth buffer is enabled.
   * @param[in] stencil the flag of stencil. it true is set, 8bit stencil buffer is enabled.
   * @param[in] msaa the bit of msaa.
   * @param[in] version the GLES version.
   *
   */
  void SetEglConfig(bool depth, bool stencil, int msaa, int version);

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
   * @copydoc Dali::GlWindow::RegisterGlCallback()
   */
  void RegisterGlCallback(CallbackBase* initCallback, CallbackBase* renderFrameCallback, CallbackBase* terminateCallback);

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

private:
  GraphicsInterface* mGraphics; ///< Graphics interface
  WindowBase*        mWindowBase;

  const Dali::LogFactoryInterface& mLogFactory;

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

  // To manage the render/main thread
  ConditionalWait       mRenderThreadWaitCondition; ///< The wait condition for the update-render-thread.
  volatile unsigned int mDestroyRenderThread;       ///< Stop render thread. It means this rendter thread will be destoried.
  volatile unsigned int mPauseRenderThread;         ///< Sleep render thread by pause.
  volatile unsigned int mRenderingMode;             ///< Rendering Mode, 0: continuous, 1:OnDemad
  volatile unsigned int mRequestRenderOnce;         ///< Request rendering once

  uint64_t mDefaultFrameDurationNanoseconds; ///< Default duration of a frame (used for sleeping if not enough time elapsed). Not protected by lock, but written to rarely so not worth adding a lock when reading.

}; // GlWindowRenderThread

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif
