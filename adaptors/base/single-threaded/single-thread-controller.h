#ifndef __DALI_INTERNAL_SINGLE_THREAD_CONTROLLER_H__
#define __DALI_INTERNAL_SINGLE_THREAD_CONTROLLER_H__

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

// EXTERNAL INCLUDES
#include <stdint.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <dali/integration-api/core.h>

// INTERNAL INCLUDES
#include <timer.h>
#include <base/interfaces/performance-interface.h>
#include <base/fps-tracker.h>
#include <base/render-helper.h>
#include <base/thread-controller-interface.h>
#include <base/update-status-logger.h>

namespace Dali
{

class RenderSurface;

namespace Internal
{

namespace Adaptor
{

class AdaptorInternalServices;
class EnvironmentOptions;

/**
 * Single Thread Controller, where events, updates & renders ALL occur on the same thread.
 */
class SingleThreadController : public ConnectionTracker,
                               public ThreadControllerInterface
{
public:

  /**
   * Constructor
   */
  SingleThreadController( AdaptorInternalServices& adaptorInterfaces, const EnvironmentOptions& environmentOptions );

  /**
   * Non virtual destructor. Not intended as base class.
   */
  ~SingleThreadController();

  /**
   * @copydoc ThreadControllerInterface::Initialize()
   */
  void Initialize();

  /**
   * @copydoc ThreadControllerInterface::Start()
   */
  void Start();

  /**
   * @copydoc ThreadControllerInterface::Pause()
   */
  void Pause();

  /**
   * @copydoc ThreadControllerInterface::Resume()
   */
  void Resume();

  /**
   * @copydoc ThreadControllerInterface::Stop()
   */
  void Stop();

  /**
   * @copydoc ThreadControllerInterface::RequestUpdate()
   */
  void RequestUpdate();

  /**
   * @copydoc ThreadControllerInterface::RequestUpdateOnce()
   */
  void RequestUpdateOnce();

  /**
   * @copydoc ThreadControllerInterface::ReplaceSurface()
   */
  void ReplaceSurface( RenderSurface* surface );

  /**
   * @copydoc ThreadControllerInterface::ResizeSurface()
   */
  virtual void ResizeSurface();

  /**
   * @copydoc ThreadControllerInterface::SetRenderRefreshRate()
   */
  void SetRenderRefreshRate( unsigned int refreshRate );

private:

  /**
   * State Machine
   */
  struct State
  {
    enum Type
    {
      STOPPED,
      RUNNING,
      PAUSED,
      SLEEPING
    };
  };

  // Undefined copy constructor.
  SingleThreadController( const SingleThreadController& );

  // Undefined assignment operator.
  SingleThreadController& operator=( const SingleThreadController& );

  /**
   * Ticks whenever the timer expires
   */
  bool OnTimerTick();

  /**
   * Runs the update and render
   *
   * @param[in] incrementTime If true, then the animation times are incremented.
   */
  void UpdateRender( bool incrementTime );

  /**
   * Updates mCurrentTime and gets the time elapsed (in seconds) since last time this function was called.
   *
   * @return time elapsed (in seconds) since last call.
   */
  float UpdateTimeSinceLastRender();

  /**
   * Helper to add a performance marker to the performance server (if it's active)
   * @param type performance marker type
   */
  void AddPerformanceMarker( PerformanceInterface::MarkerType type );

  /**
   * Changes the state and performs any other state-change related functionality.
   * @param[in] state The new state
   */
  void ChangeState( State::Type state );

  /**
   * Performs operations to stop rendering, e.g. informing Core of context being destroyed & shutting down EGL.
   */
  void StopRendering();

private:

  Dali::Timer                       mTimer;                           ///< Ensures an update & render is run every frame.
  FpsTracker                        mFpsTracker;                      ///< Object that tracks the FPS
  UpdateStatusLogger                mUpdateStatusLogger;              ///< Object that logs the update-status as required.

  RenderHelper                      mRenderHelper;                    ///< Helper class for EGL, pre & post rendering

  Integration::Core&                mCore;                            ///< DALi core reference
  PerformanceInterface*             mPerformanceInterface;            ///< The performance logging interface

  uint64_t                          mLastUpdateRenderTime;            ///< Last time we did an update and render
  uint64_t                          mSystemTime;                      ///< The current system time for FPS calculations
  unsigned int                      mRefreshRate;                     ///< Frame skipping count
  State::Type                       mState;                           ///< The state
  bool                              mUpdatingAndRendering:1;          ///< Set to true when we are updating and rendering.
  bool                              mStopRequestedWhileRendering:1;   ///< Set to true if we were told to stop while we were in the middle of a render
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_SINGLE_THREAD_CONTROLLER_H__
