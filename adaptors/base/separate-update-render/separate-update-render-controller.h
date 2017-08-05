#ifndef __DALI_INTERNAL_MULTI_THREAD_CONTROLLER_H__
#define __DALI_INTERNAL_MULTI_THREAD_CONTROLLER_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <base/thread-controller-interface.h>

namespace Dali
{

class RenderSurface;

namespace Internal
{

namespace Adaptor
{

class UpdateThread;
class RenderThread;
class VSyncNotifier;
class ThreadSynchronization;
class AdaptorInternalServices;
class EnvironmentOptions;

/**
 * Class to control multiple threads:
 *  - Main Event Thread
 *  - VSync Thread
 *  - Update Thread
 *  - Render Thread
 */
class SeparateUpdateRenderController : public ThreadControllerInterface
{
public:

  /**
   * Constructor
   */
  SeparateUpdateRenderController( AdaptorInternalServices& adaptorInterfaces, const EnvironmentOptions& environmentOptions );

  /**
   * Non virtual destructor. Not intended as base class.
   */
  ~SeparateUpdateRenderController();

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
  void SetRenderRefreshRate( unsigned int numberOfVSyncsPerRender );

private:

  // Undefined copy constructor.
  SeparateUpdateRenderController( const SeparateUpdateRenderController& );

  // Undefined assignment operator.
  SeparateUpdateRenderController& operator=( const SeparateUpdateRenderController& );

  AdaptorInternalServices&     mAdaptorInterfaces;

  UpdateThread*                mUpdateThread;     ///< The update-thread owned by SeparateUpdateRenderController
  RenderThread*                mRenderThread;     ///< The render-thread owned by SeparateUpdateRenderController
  VSyncNotifier*               mVSyncNotifier;    ///< The vsync-thread owned by SeparateUpdateRenderController
  ThreadSynchronization*       mThreadSync;       ///< Used to synchronize all the threads; owned by SeparateUpdateRenderController
  unsigned int                 mNumberOfVSyncsPerRender; ///< Frame skipping count
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_MULTI_THREAD_CONTROLLER_H__
