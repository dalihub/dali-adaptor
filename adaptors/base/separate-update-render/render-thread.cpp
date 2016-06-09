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

// CLASS HEADER
#include "render-thread.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <base/interfaces/adaptor-internal-services.h>
#include <base/separate-update-render/thread-synchronization.h>
#include <base/environment-options.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gRenderLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_RENDER_THREAD");
#endif
}

RenderThread::RenderThread( ThreadSynchronization& sync,
                            AdaptorInternalServices& adaptorInterfaces,
                            const EnvironmentOptions& environmentOptions )
: mThreadSynchronization( sync ),
  mCore( adaptorInterfaces.GetCore() ),
  mThread( NULL ),
  mEnvironmentOptions( environmentOptions ),
  mRenderHelper( adaptorInterfaces )
{
}

RenderThread::~RenderThread()
{
}

void RenderThread::Start()
{
  DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Start()\n");

  // create the render thread, initially we are rendering
  mThread = new pthread_t();
  int error = pthread_create( mThread, NULL, InternalThreadEntryFunc, this );
  DALI_ASSERT_ALWAYS( !error && "Return code from pthread_create() in RenderThread" );

  mRenderHelper.Start();
}

void RenderThread::Stop()
{
  DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Stop()\n");

  mRenderHelper.Stop();

  // shutdown the render thread and destroy the opengl context
  if( mThread )
  {
    // wait for the thread to finish
    pthread_join(*mThread, NULL);

    delete mThread;
    mThread = NULL;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The following methods are all executed inside render thread !!!
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RenderThread::Run()
{
  DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run\n");

  // Install a function for logging
  mEnvironmentOptions.InstallLogFunction();

  mRenderHelper.InitializeEgl();

  // tell core it has a context
  mCore.ContextCreated();

  Dali::Integration::RenderStatus renderStatus;
  RenderRequest* request = NULL;

  // Render loop, we stay inside here when rendering
  while( mThreadSynchronization.RenderReady( request ) )
  {
    DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 1 - RenderReady\n");

    // Consume any pending events to avoid memory leaks
    DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 2 - ConsumeEvents\n");
    mRenderHelper.ConsumeEvents();

    // Check if we've got a request from the main thread (e.g. replace surface)
    if( request )
    {
      // Process the request, we should NOT render when we have a request
      DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 3 - Process requests\n");
      ProcessRequest( request );
    }
    else
    {
      // No request to process so we render
      if( mRenderHelper.PreRender() ) // Returns false if no surface onto which to render
      {
        // Render
        DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 3 - Core.Render()\n");

        mThreadSynchronization.AddPerformanceMarker( PerformanceInterface::RENDER_START );
        mCore.Render( renderStatus );
        mThreadSynchronization.AddPerformanceMarker( PerformanceInterface::RENDER_END );

        // Decrement the count of how far update is ahead of render
        mThreadSynchronization.RenderFinished();

        // Perform any post-render operations
        DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 4 - PostRender()\n");
        mRenderHelper.PostRender();
      }
    }

    request = NULL; // Clear the request if it was set, no need to release memory
  }

  // Inform core of context destruction & shutdown EGL
  mCore.ContextDestroyed();
  mRenderHelper.ShutdownEgl();

  // Uninstall the logging function
  mEnvironmentOptions.UnInstallLogFunction();

  return true;
}

void RenderThread::ProcessRequest( RenderRequest* request )
{
  if( request != NULL )
  {
    switch(request->GetType())
    {
      case RenderRequest::REPLACE_SURFACE:
      {
        // change the surface
        ReplaceSurfaceRequest* replaceSurfaceRequest = static_cast<ReplaceSurfaceRequest*>(request);
        mRenderHelper.ReplaceSurface( replaceSurfaceRequest->GetSurface() );
        replaceSurfaceRequest->ReplaceCompleted();
        mThreadSynchronization.RenderInformSurfaceReplaced();
        break;
      }
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
