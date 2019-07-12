#ifndef DALI_INTERNAL_COMBINED_UPDATE_RENDER_CONTROLLER_H
#define DALI_INTERNAL_COMBINED_UPDATE_RENDER_CONTROLLER_H

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
#include <semaphore.h>
#include <stdint.h>
#include <dali/integration-api/core.h>
#include <dali/devel-api/threading/conditional-wait.h>

// INTERNAL INCLUDES
#include <dali/integration-api/thread-synchronization-interface.h>
#include <dali/internal/system/common/performance-interface.h>
#include <dali/internal/system/common/fps-tracker.h>
#include <dali/internal/adaptor/common/thread-controller-interface.h>
#include <dali/internal/system/common/update-status-logger.h>
#include <dali/internal/window-system/common/display-connection.h>


namespace Dali
{

class RenderSurfaceInterface;
class TriggerEventInterface;

namespace Internal
{

namespace Adaptor
{

class AdaptorInternalServices;
class EnvironmentOptions;

/**
 * @brief Two threads where events/application interaction is handled on the main/event thread and the Update & Render
 * happen on the other thread.
 *
 * Key Points:
 *  1. Two Threads:
 *    a. Main/Event Thread.
 *    b. Update/Render Thread.
 *  2. There is NO VSync thread:
 *    a. We retrieve the time before Update.
 *    b. Then retrieve the time after Render.
 *    c. We calculate the difference between these two times and if:
 *      i.  The difference is less than the default frame time, we sleep.
 *      ii. If it’s more or the same, we continue.
 *  3. On the update/render thread, if we discover that we do not need to do any more updates, we use a trigger-event
 *     to inform the main/event thread. This is then processed as soon as the event thread is able to do so where it
 *     is easier to make a decision about whether we should stop the update/render thread or not (depending on any
 *     update requests etc.).
 *  4. The main thread is blocked while the surface is being replaced.
 *  5. When we resume from paused, elapsed time is used for the animations, i.e. the could have finished while we were paused.
 *     However, FinishedSignal emission will only happen upon resumption.
 *  6. Elapsed time is NOT used while if we are waking up from a sleep state or doing an UpdateOnce.
 */
class CombinedUpdateRenderController : public ThreadControllerInterface,
                                       public ThreadSynchronizationInterface
{
public:

  /**
   * Constructor
   */
  CombinedUpdateRenderController( AdaptorInternalServices& adaptorInterfaces, const EnvironmentOptions& environmentOptions );

  /**
   * Non virtual destructor. Not intended as base class.
   */
  ~CombinedUpdateRenderController();

  /**
   * @copydoc ThreadControllerInterface::Initialize()
   */
  virtual void Initialize();

  /**
   * @copydoc ThreadControllerInterface::Start()
   */
  virtual void Start();

  /**
   * @copydoc ThreadControllerInterface::Pause()
   */
  virtual void Pause();

  /**
   * @copydoc ThreadControllerInterface::Resume()
   */
  virtual void Resume();

  /**
   * @copydoc ThreadControllerInterface::Stop()
   */
  virtual void Stop();

  /**
   * @copydoc ThreadControllerInterface::RequestUpdate()
   */
  virtual void RequestUpdate();

  /**
   * @copydoc ThreadControllerInterface::RequestUpdateOnce()
   */
  virtual void RequestUpdateOnce();

  /**
   * @copydoc ThreadControllerInterface::ReplaceSurface()
   */
  virtual void ReplaceSurface( Dali::RenderSurfaceInterface* surface );

  /**
   * @copydoc ThreadControllerInterface::DeleteSurface()
   */
  virtual void DeleteSurface( Dali::RenderSurfaceInterface* surface );

  /**
   * @copydoc ThreadControllerInterface::ResizeSurface()
   */
  virtual void ResizeSurface();

  /**
   * @copydoc ThreadControllerInterface::SetRenderRefreshRate()
   */
  virtual void SetRenderRefreshRate( unsigned int numberOfFramesPerRender );

  /**
   * @copydoc ThreadControllerInterface::SetPreRenderCallback
   */
  void SetPreRenderCallback( CallbackBase* callback ) override;

private:

  // Undefined copy constructor.
  CombinedUpdateRenderController( const CombinedUpdateRenderController& );

  // Undefined assignment operator.
  CombinedUpdateRenderController& operator=( const CombinedUpdateRenderController& );

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // EventThread
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Runs the Update/Render Thread.
   * This will lock the mutex in mUpdateRenderThreadWaitCondition.
   *
   * @param[in]  numberOfCycles           The number of times the update/render cycle should run. If -1, then it will run continuously.
   * @param[in]  useElapsedTimeAfterWait  If true, then the elapsed time during wait is used for animations, otherwise no animation progression is made.
   */
  inline void RunUpdateRenderThread( int numberOfCycles, bool useElapsedTimeAfterWait );

  /**
   * Pauses the Update/Render Thread.
   * This will lock the mutex in mUpdateRenderThreadWaitCondition.
   */
  inline void PauseUpdateRenderThread();

  /**
   * Stops the Update/Render Thread.
   * This will lock the mutex in mUpdateRenderThreadWaitCondition.
   *
   * @note Should only be called in Stop as calling this will kill the update-thread.
   */
  inline void StopUpdateRenderThread();

  /**
   * Checks if the the Update/Render Thread is paused.
   * This will lock the mutex in mUpdateRenderThreadWaitCondition.
   *
   * @return true if paused, false otherwise
   */
  inline bool IsUpdateRenderThreadPaused();

  /**
   * Used as the callback for the sleep-trigger.
   *
   * Will sleep when enough requests are made without any requests.
   */
  void ProcessSleepRequest();

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // UpdateRenderThread
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * The Update/Render thread loop. This thread will be destroyed on exit from this function.
   */
  void UpdateRenderThread();

  /**
   * Called by the Update/Render Thread which ensures a wait if required.
   *
   * @param[out] useElapsedTime    If true when returned, then the actual elapsed time will be used for animation.
   *                               If false when returned, then there should NOT be any animation progression in the next Update.
   * @param[in]  updateRequired    Whether another update is required.
   * @param[out] timeToSleepUntil  The time remaining in nanoseconds to keep the thread sleeping before resuming.
   * @return false, if the thread should stop.
   */
  bool UpdateRenderReady( bool& useElapsedTime, bool updateRequired, uint64_t& timeToSleepUntil );

  /**
   * Checks to see if the surface needs to be replaced.
   * This will lock the mutex in mUpdateRenderThreadWaitCondition.
   *
   * @return Pointer to the new surface, NULL otherwise
   */
  Integration::RenderSurface* ShouldSurfaceBeReplaced();

  /**
   * Called by the Update/Render thread after a surface has been replaced.
   *
   * This will lock the mutex in mEventThreadWaitCondition
   */
  void SurfaceReplaced();

  /**
   * Checks to see if the surface needs to be deleted.
   * This will lock the mutex in mUpdateRenderThreadWaitCondition.
   *
   * @return Pointer to the deleted surface, nullptr otherwise
   */
  Integration::RenderSurface* ShouldSurfaceBeDeleted();

  /**
   * Called by the Update/Render thread after a surface has been deleted.
   *
   * This will lock the mutex in mEventThreadWaitCondition
   */
  void SurfaceDeleted();

  /**
   * Checks to see if the surface needs to be resized.
   * This will lock the mutex in mUpdateRenderThreadWaitCondition.
   *
   * @return true if the surface should be resized, false otherwise
   */
  bool ShouldSurfaceBeResized();

  /**
   * Called by the Update/Render thread after a surface has been resized.
   *
   * This will lock the mutex in mEventThreadWaitCondition
   */
  void SurfaceResized();

  /**
   * Helper for the thread calling the entry function
   * @param[in] This A pointer to the current object
   */
  static void* InternalUpdateRenderThreadEntryFunc( void* This )
  {
    ( static_cast<CombinedUpdateRenderController*>( This ) )->UpdateRenderThread();
    return NULL;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // ALL Threads
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Called by the update-render & v-sync threads when they up and running.
   *
   * This will lock the mutex in mEventThreadWaitCondition.
   */
  void NotifyThreadInitialised();

  /**
   * Helper to add a performance marker to the performance server (if it's active)
   * @param[in]  type  performance marker type
   */
  void AddPerformanceMarker( PerformanceInterface::MarkerType type );

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // POST RENDERING - ThreadSynchronizationInterface overrides
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////////////////////////
  //// Called by the Event Thread if post-rendering is required
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @copydoc ThreadSynchronizationInterface::PostRenderComplete()
   */
  virtual void PostRenderComplete();

  /////////////////////////////////////////////////////////////////////////////////////////////////
  //// Called by the Render Thread if post-rendering is required
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @copydoc ThreadSynchronizationInterface::PostRenderStarted()
   */
  virtual void PostRenderStarted();

  /**
   * @copydoc ThreadSynchronizationInterface::PostRenderStarted()
   */
  virtual void PostRenderWaitForCompletion();

private:

  FpsTracker                        mFpsTracker;                       ///< Object that tracks the FPS
  UpdateStatusLogger                mUpdateStatusLogger;               ///< Object that logs the update-status as required.

  sem_t                             mEventThreadSemaphore;             ///< Used by the event thread to ensure all threads have been initialised, and when replacing the surface.

  ConditionalWait                   mUpdateRenderThreadWaitCondition;  ///< The wait condition for the update-render-thread.

  AdaptorInternalServices&          mAdaptorInterfaces;                ///< The adaptor internal interface
  PerformanceInterface*             mPerformanceInterface;             ///< The performance logging interface
  Integration::Core&                mCore;                             ///< Dali core reference
  const EnvironmentOptions&         mEnvironmentOptions;               ///< Environment options
  TriggerEventInterface&            mNotificationTrigger;              ///< Reference to notification event trigger
  TriggerEventInterface*            mSleepTrigger;                     ///< Used by the update-render thread to trigger the event thread when it no longer needs to do any updates
  CallbackBase*                     mPreRenderCallback;                ///< Used by Update/Render thread when PreRender is about to be called on graphics.

  pthread_t*                        mUpdateRenderThread;               ///< The Update/Render thread.

  float                             mDefaultFrameDelta;                ///< Default time delta between each frame (used for animations). Not protected by lock, but written to rarely so not worth adding a lock when reading.
  // TODO: mDefaultFrameDurationMilliseconds is defined as uint64_t, the only place where it is used, it is converted to an unsigned int!!!
  uint64_t                          mDefaultFrameDurationMilliseconds; ///< Default duration of a frame (used for predicting the time of the next frame). Not protected by lock, but written to rarely so not worth adding a lock when reading.
  uint64_t                          mDefaultFrameDurationNanoseconds;  ///< Default duration of a frame (used for sleeping if not enough time elapsed). Not protected by lock, but written to rarely so not worth adding a lock when reading.
  uint64_t                          mDefaultHalfFrameNanoseconds;      ///< Is half of mDefaultFrameDurationNanoseconds. Using a member variable avoids having to do the calculation every frame. Not protected by lock, but written to rarely so not worth adding a lock when reading.

  unsigned int                      mUpdateRequestCount;               ///< Count of update-requests we have received to ensure we do not go to sleep too early.
  unsigned int                      mRunning;                          ///< Read and set on the event-thread only to state whether we are running.

  //
  // NOTE: cannot use booleans as these are used from multiple threads, must use variable with machine word size for atomic read/write
  //

  volatile int                      mUpdateRenderRunCount;             ///< The number of times Update/Render cycle should run. If -1, then will run continuously (set by the event-thread, read by v-sync-thread).
  volatile unsigned int             mDestroyUpdateRenderThread;        ///< Whether the Update/Render thread be destroyed (set by the event-thread, read by the update-render-thread).
  volatile unsigned int             mUpdateRenderThreadCanSleep;       ///< Whether the Update/Render thread can sleep (set by the event-thread, read by the update-render-thread).
  volatile unsigned int             mPendingRequestUpdate;             ///< Is set as soon as an RequestUpdate is made and unset when the next update happens (set by the event-thread and update-render thread, read by the update-render-thread).
                                                                       ///< Ensures we do not go to sleep if we have not processed the most recent update-request.

  volatile unsigned int             mUseElapsedTimeAfterWait;          ///< Whether we should use the elapsed time after waiting (set by the event-thread, read by the update-render-thread).

  Integration::RenderSurface* volatile mNewSurface;                    ///< Will be set to the new-surface if requested (set by the event-thread, read & cleared by the update-render thread).
  Integration::RenderSurface* volatile mDeletedSurface;                ///< Will be set to the deleted surface if requested (set by the event-thread, read & cleared by the update-render thread).

  volatile unsigned int             mPostRendering;                    ///< Whether post-rendering is taking place (set by the event & render threads, read by the render-thread).
  volatile unsigned int             mSurfaceResized;                   ///< Will be set to resize the surface (set by the event-thread, read & cleared by the update-render thread).
  volatile unsigned int             mForceClear;                       ///< Will be set to clear forcibly
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_COMBINED_UPDATE_RENDER_CONTROLLER_H
