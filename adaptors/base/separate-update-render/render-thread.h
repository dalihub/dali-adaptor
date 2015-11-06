#ifndef __DALI_INTERNAL_RENDER_THREAD_H__
#define __DALI_INTERNAL_RENDER_THREAD_H__

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
#include <pthread.h>

// INTERNAL INCLUDES
#include <base/render-helper.h>
#include <base/separate-update-render/render-request.h>
#include <egl-interface.h>
#include <render-surface.h> // needed for Dali::RenderSurface

namespace Dali
{

class RenderSurface;

namespace Integration
{
class Core;
}

namespace Internal
{
namespace Adaptor
{

class AdaptorInternalServices;
class ThreadSynchronization;
class EnvironmentOptions;

/**
 * The render-thread is responsible for calling Core::Render() after each update.
 */
class RenderThread
{
public:

  /**
   * Create the render-thread; this will not do anything until Start() is called.
   * @param[in] sync thread synchronization object
   * @param[in] adaptorInterfaces base adaptor interface
   * @param[in] environmentOptions environment options
   */
  RenderThread( ThreadSynchronization& sync,
                AdaptorInternalServices& adaptorInterfaces,
                const EnvironmentOptions& environmentOptions );

  /**
   * Destructor
   */
  ~RenderThread();

public:

  /**
   * Starts the render-thread
   */
  void Start();

  /**
   * Stops the render-thread
   */
  void Stop();

private: // Render thread side helpers

  /**
   * This method is used by the Render thread for rendering the Core to the screen.
   * Called from render thread
   * @return true, if the thread finishes properly.
   */
  bool Run();

  /**
   * Check if main thread made any requests, e.g. ReplaceSurface
   * Called from render thread
   */
  void ProcessRequest( RenderRequest* request );

  /**
   * Helper for the thread calling the entry function.
   * @param[in] This A pointer to the current RenderThread object
   */
  static inline void* InternalThreadEntryFunc( void* This )
  {
    ( static_cast<RenderThread*>( This ) )->Run();
    return NULL;
  }

private:

  // Undefined
  RenderThread( const RenderThread& renderThread );

  // Undefined
  RenderThread& operator=( const RenderThread& renderThread );

private: // Data

  ThreadSynchronization&        mThreadSynchronization;  ///< Used to synchronize the all threads
  Dali::Integration::Core&      mCore;                   ///< Dali core reference
  pthread_t*                    mThread;                 ///< render thread
  const EnvironmentOptions&     mEnvironmentOptions;     ///< Environment options
  RenderHelper                  mRenderHelper;           ///< Helper class for EGL, pre & post rendering
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_RENDER_THREAD_H__
