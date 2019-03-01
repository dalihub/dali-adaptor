#ifndef __DALI_INTERNAL_THREAD_CONTROLLER_H__
#define __DALI_INTERNAL_THREAD_CONTROLLER_H__

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
#include <dali/public-api/signals/callback.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/display-connection.h>

namespace Dali
{

class RenderSurfaceInterface;

namespace Internal
{

namespace Adaptor
{

class AdaptorInternalServices;
class EnvironmentOptions;
class ThreadControllerInterface;

/**
 * Class to control all the threads.
 */
class ThreadController
{
public:

  /**
   * Constructor
   */
  ThreadController( AdaptorInternalServices& adaptorInterfaces, const EnvironmentOptions& environmentOptions );

  /**
   * Non virtual destructor. Not intended as base class.
   */
  ~ThreadController();

  /**
   * @brief Initializes the thread controller
   *
   * Will do any required initialiszation, e.g. initialize EGL, create threads (if required), etc.
   *
   * @note When this function returns, the application Init signal should be emitted
   */
  void Initialize();

  /**
   * @brief Called AFTER the Init signal has been emitted.
   *
   * In other words, should be called AFTER the Init signal has been emitted and all messages for the first scene
   * have been queued for update to process.
   */
  void Start();

  /**
   * @brief When called, update and rendering is paused.
   */
  void Pause();

  /**
   * @brief Resumes update/rendering after a previous pause.
   */
  void Resume();

  /**
   * @brief Stops update/rendering altogether.
   *
   * Will shutdown EGL, destroy threads (if required) etc.
   */
  void Stop();

  /**
   * @brief Called by the adaptor when core requires another update
   */
  void RequestUpdate();

  /**
   * @brief Called by the adaptor when core requires one update
   *
   * @note If Adaptor is paused, we do one update/render only
   */
  void RequestUpdateOnce();

  /**
   * @brief Replaces the surface.
   *
   * @param surface new surface
   */
  void ReplaceSurface( Dali::RenderSurfaceInterface* surface );

  /**
   * Resize the surface.
   */
  void ResizeSurface();

  /**
   * @copydoc Dali::Adaptor::SetRenderRefreshRate()
   */
  void SetRenderRefreshRate( unsigned int numberOfVSyncsPerRender );

  /**
   * @copydoc Dali::Adaptor::SetPreRenderCallback
   */
  void SetPreRenderCallback( CallbackBase* callback );

private:

  // Undefined copy constructor.
  ThreadController( const ThreadController& ) = delete;

  // Undefined assignment operator.
  ThreadController& operator=( const ThreadController& ) = delete;

private:

  ThreadControllerInterface* mThreadControllerInterface;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_THREAD_CONTROLLER_H__
