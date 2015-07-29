/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <base/update-render-synchronization.h>
#include <base/environment-options.h>
#include <base/display-connection.h>

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

RenderRequest::RenderRequest(RenderRequest::Request type)
: mRequestType(type)
{
}

RenderRequest::Request RenderRequest::GetType()
{
  return mRequestType;
}

ReplaceSurfaceRequest::ReplaceSurfaceRequest()
: RenderRequest(RenderRequest::REPLACE_SURFACE),
  mNewSurface( NULL ),
  mReplaceCompleted(false)
{
}

void ReplaceSurfaceRequest::SetSurface(RenderSurface* newSurface)
{
  mNewSurface = newSurface;
}

RenderSurface* ReplaceSurfaceRequest::GetSurface()
{
  return mNewSurface;
}

void ReplaceSurfaceRequest::ReplaceCompleted()
{
  mReplaceCompleted = true;
}

bool ReplaceSurfaceRequest::GetReplaceCompleted()
{
  return mReplaceCompleted != 0u;
}


RenderThread::RenderThread( UpdateRenderSynchronization& sync,
                            AdaptorInternalServices& adaptorInterfaces,
                            const EnvironmentOptions& environmentOptions )
: mUpdateRenderSync( sync ),
  mCore( adaptorInterfaces.GetCore() ),
  mGLES( adaptorInterfaces.GetGlesInterface() ),
  mEglFactory( &adaptorInterfaces.GetEGLFactoryInterface()),
  mEGL( NULL ),
  mThread( NULL ),
  mEnvironmentOptions( environmentOptions ),
  mSurfaceReplaced(false)
{
  // set the initial values before render thread starts
  mSurface = adaptorInterfaces.GetRenderSurfaceInterface();

  mDisplayConnection = Dali::DisplayConnection::New();
}

RenderThread::~RenderThread()
{
  if (mDisplayConnection)
  {
    delete mDisplayConnection;
    mDisplayConnection = NULL;
  }

  DALI_ASSERT_ALWAYS( mThread == NULL && "RenderThread is still alive");
  mEglFactory->Destroy();
}

void RenderThread::Start()
{
  DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Start()\n");

  // initialise GL and kick off render thread
  DALI_ASSERT_ALWAYS( !mEGL && "Egl already initialized" );

  // create the render thread, initially we are rendering
  mThread = new pthread_t();
  int error = pthread_create( mThread, NULL, InternalThreadEntryFunc, this );
  DALI_ASSERT_ALWAYS( !error && "Return code from pthread_create() in RenderThread" );

  if( mSurface )
  {
    mSurface->StartRender();
  }
}

void RenderThread::Stop()
{
  DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Stop()\n");

  if( mSurface )
  {
    // Tell surface we have stopped rendering
    mSurface->StopRender();

    // The surface will be destroyed soon; this pointer will become invalid
    mSurface = NULL;
  }

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
  // install a function for logging
  mEnvironmentOptions.InstallLogFunction();

  InitializeEgl();

  bool running( true );

  Dali::Integration::RenderStatus renderStatus;

  uint64_t currentTime( 0 );

  // render loop, we stay inside here when rendering
  while( running )
  {
    // Sync with update thread and get any outstanding requests from UpdateRenderSynchronization
    DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 1 - RenderSyncWithUpdate()\n");
    RenderRequest* request = NULL;
    running = mUpdateRenderSync.RenderSyncWithUpdate( request );

    DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 2 - Process requests\n");

    // Consume any pending events to avoid memory leaks
    mDisplayConnection->ConsumeEvents();

    bool processRequests = true;
    bool requestProcessed = false;
    while( processRequests && running)
    {
      // Check if we've got any requests from the main thread (e.g. replace surface)
      requestProcessed = ProcessRequest( request );

      // perform any pre-render operations
      DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 3 - PreRender\n");
      bool preRendered = PreRender(); // Returns false if no surface onto which to render
      if( preRendered )
      {
        processRequests = false;
      }
      else
      {
        // Block until new surface... - cleared by ReplaceSurface code in UpdateRenderController
        running = mUpdateRenderSync.RenderSyncWithRequest(request);
      }
    }

    if( running )
    {
       // Render
      DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 4 - Core.Render()\n");
      mCore.Render( renderStatus );

      // Notify the update-thread that a render has completed
      DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 5 - Sync.RenderFinished()\n");
      mUpdateRenderSync.RenderFinished( renderStatus.NeedsUpdate(), requestProcessed );

      uint64_t newTime( mUpdateRenderSync.GetTimeMicroseconds() );

      // perform any post-render operations
      if ( renderStatus.HasRendered() )
      {
        DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 6 - PostRender()\n");
        PostRender( static_cast< unsigned int >(newTime - currentTime) );
      }

      currentTime = newTime;
    }
  }

  // shut down egl
  ShutdownEgl();

  // install a function for logging
  mEnvironmentOptions.UnInstallLogFunction();

  return true;
}

void RenderThread::InitializeEgl()
{
  mEGL = mEglFactory->Create();

  DALI_ASSERT_ALWAYS( mSurface && "NULL surface" );

  // initialize egl & OpenGL
  mDisplayConnection->InitializeEgl( *mEGL );
  mSurface->InitializeEgl( *mEGL );

  // create the OpenGL context
  mEGL->CreateContext();

  // create the OpenGL surface
  mSurface->CreateEglSurface(*mEGL);

  // Make it current
  mEGL->MakeContextCurrent();

  // set the initial sync mode

  // tell core it has a context
  mCore.ContextCreated();

}

bool RenderThread::ProcessRequest( RenderRequest* request )
{
  bool processedRequest = false;

  if( request != NULL )
  {
    switch(request->GetType())
    {
      case RenderRequest::REPLACE_SURFACE:
      {
        // change the surface
        ReplaceSurfaceRequest* replaceSurfaceRequest = static_cast<ReplaceSurfaceRequest*>(request);
        ReplaceSurface( replaceSurfaceRequest->GetSurface() );
        replaceSurfaceRequest->ReplaceCompleted();
        processedRequest = true;
        break;
      }
    }
  }
  return processedRequest;
}

void RenderThread::ReplaceSurface( RenderSurface* newSurface )
{
  // This is designed for replacing pixmap surfaces, but should work for window as well
  // we need to delete the egl surface and renderable (pixmap / window)
  // Then create a new pixmap/window and new egl surface
  // If the new surface has a different display connection, then the context will be lost
  DALI_ASSERT_ALWAYS(newSurface && "NULL surface");

  mDisplayConnection->InitializeEgl(*mEGL);

  bool contextLost = newSurface->ReplaceEGLSurface(*mEGL);
  if( contextLost )
  {
    DALI_LOG_WARNING("Context lost\n");
    mCore.ContextDestroyed();
    mCore.ContextCreated();
  }

  // use the new surface from now on
  mSurface = newSurface;
  mSurfaceReplaced = true;
}

void RenderThread::ShutdownEgl()
{
  // inform core of context destruction
  mCore.ContextDestroyed();

  if( mSurface )
  {
    // give a chance to destroy the OpenGL surface that created externally
    mSurface->DestroyEglSurface( *mEGL );
  }

  // delete the GL context / egl surface
  mEGL->TerminateGles();
}

bool RenderThread::PreRender()
{
  bool success( false );
  if( mSurface )
  {
    success = mSurface->PreRender( *mEGL, mGLES );
  }

  if( success )
  {
    mGLES.PreRender();
  }
  return success;
}

void RenderThread::PostRender( unsigned int timeDelta )
{
  // Inform the gl implementation that rendering has finished before informing the surface
  mGLES.PostRender(timeDelta);

  if( mSurface )
  {
    // Inform the surface that rendering this frame has finished.
    mSurface->PostRender( *mEGL, mGLES, mDisplayConnection, timeDelta, mSurfaceReplaced );
  }
  mSurfaceReplaced = false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
