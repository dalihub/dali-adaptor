#ifndef DALI_INTERNAL_THREAD_CONTROLLER_INTERFACE_H
#define DALI_INTERNAL_THREAD_CONTROLLER_INTERFACE_H

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

#include <dali/public-api/signals/callback.h>

namespace Dali
{

class RenderSurfaceInterface;

namespace Internal
{

namespace Adaptor
{

enum class UpdateMode
{
  NORMAL,                     ///< Update and render
  SKIP_RENDER                 ///< Update and resource upload but no rendering
};

/**
 * Interface Class for all controlling threads.
 */
class ThreadControllerInterface
{
public:

  /**
   * Virtual destructor. Not intended as base class.
   */
  virtual ~ThreadControllerInterface() { }

  /**
   * Initializes the thread controller
   */
  virtual void Initialize() = 0;

  /**
   * @copydoc Dali::Adaptor::Start()
   */
  virtual void Start() = 0;

  /**
   * @copydoc Dali::Adaptor::Pause()
   */
  virtual void Pause() = 0;

  /**
   * @copydoc Dali::Adaptor::Resume()
   */
  virtual void Resume() = 0;

  /**
   * @copydoc Dali::Adaptor::Stop()
   */
  virtual void Stop() = 0;

  /**
   * Called by the adaptor when core requires another update
   */
  virtual void RequestUpdate() = 0;

  /**
   * Called by the adaptor when core requires one update
   * If Adaptor is paused, we do one update/render and return to pause
   * @param updateMode The update mode (i.e. i.e. either update & render or skip rendering)
   */
  virtual void RequestUpdateOnce( UpdateMode updateMode ) = 0;

  /**
   * Replaces the surface.
   * @param surface new surface
   */
  virtual void ReplaceSurface( Dali::RenderSurfaceInterface* surface ) = 0;

  /**
   * Deletes the surface.
   * @param[in] surface The surface to be deleted
   */
  virtual void DeleteSurface( Dali::RenderSurfaceInterface* surface ) = 0;

  /**
   * Resize the surface.
   */
  virtual void ResizeSurface() = 0;

  /**
   * @copydoc Dali::Adaptor::SetRenderRefreshRate()
   */
  virtual void SetRenderRefreshRate( unsigned int numberOfVSyncsPerRender ) = 0;

  /**
   * @copydoc Dali::Adaptor::SetPreRenderCallback()
   */
  virtual void SetPreRenderCallback( CallbackBase* callback ) = 0;

protected:

  /**
   * Constructor
   */
  ThreadControllerInterface() { }

private:

  // Undefined copy constructor.
  ThreadControllerInterface( const ThreadControllerInterface& );

  // Undefined assignment operator.
  ThreadControllerInterface& operator=( const ThreadControllerInterface& );
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_THREAD_CONTROLLER_INTERFACE_H
