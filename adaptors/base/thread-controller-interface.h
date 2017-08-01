#ifndef __DALI_INTERNAL_THREAD_CONTROLLER_INTERFACE_H__
#define __DALI_INTERNAL_THREAD_CONTROLLER_INTERFACE_H__

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

namespace Dali
{

class RenderSurface;

namespace Internal
{

namespace Adaptor
{

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
   * If Adaptor is paused, we do one update and return to pause
   */
  virtual void RequestUpdateOnce() = 0;

  /**
   * Replaces the surface.
   * @param surface new surface
   */
  virtual void ReplaceSurface( RenderSurface* surface ) = 0;

  /**
   * Resize the surface.
   */
  virtual void ResizeSurface() = 0;

  /**
   * @copydoc Dali::Adaptor::SetRenderRefreshRate()
   */
  virtual void SetRenderRefreshRate( unsigned int numberOfVSyncsPerRender ) = 0;

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

#endif // __DALI_INTERNAL_THREAD_CONTROLLER_INTERFACE_H__
