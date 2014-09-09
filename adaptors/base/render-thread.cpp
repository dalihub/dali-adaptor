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


// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <base/interfaces/adaptor-internal-services.h>
#include <base/update-render-synchronization.h>
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
}

RenderThread::~RenderThread()
{
  DALI_ASSERT_ALWAYS( mThread == NULL && "RenderThread is still alive");
  mEglFactory->Destroy();
}

void RenderThread::Start()
{
  DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Start()\n");

  // initialise GL and kick off render thread
  DALI_ASSERT_ALWAYS( !mEGL && "Egl already initialized" );

  // create the render thread, initially we are rendering
  mThread = new boost::thread(boost::bind(&RenderThread::Run, this));

  mSurface->StartRender();
}

void RenderThread::Stop()
{
  DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Stop()\n");

  // shutdown the render thread and destroy the opengl context
  if( mThread )
  {
    // Tell surface we have stopped rendering
    mSurface->StopRender();

    // wait for the thread to finish
    mThread->join();

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

    // Consume any pending events
    ConsumeEvents();

    // Check if we've got any requests from the main thread (e.g. replace surface)
    bool requestProcessed = ProcessRequest( request );

    // perform any pre-render operations
    DALI_LOG_INFO( gRenderLogFilter, Debug::Verbose, "RenderThread::Run. 3 - PreRender\n");
    if( running && PreRender() == true)
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
  mSurface->InitializeEgl( *mEGL );

  // create the OpenGL context
  mEGL->CreateContext();

  // create the OpenGL surface
  mSurface->CreateEglSurface( *mEGL );

  // Make it current
  mEGL->MakeContextCurrent();

  // set the initial sync mode


  // tell core it has a context
  mCore.ContextCreated();

}

void RenderThread::ConsumeEvents()
{
  // tell surface to consume any events to avoid memory leaks
  mSurface->ConsumeEvents();
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
  DALI_ASSERT_ALWAYS( newSurface && "NULL surface" );

  bool contextLost = newSurface->ReplaceEGLSurface( *mEGL );

  if( contextLost )
  {
    DALI_LOG_WARNING("Context lost\n");
    mCore.ContextToBeDestroyed();
    mCore.ContextCreated();
  }

  // if both new and old surface are using the same display, and the display
  // connection was created by Dali, then transfer
  // display owner ship to the new surface.
  mSurface->TransferDisplayOwner( *newSurface );

  // use the new surface from now on
  mSurface = newSurface;
  mSurfaceReplaced = true;
}


void RenderThread::ShutdownEgl()
{
  // inform core the context is about to be destroyed,
  mCore.ContextToBeDestroyed();

  // give a chance to destroy the OpenGL surface that created externally
  mSurface->DestroyEglSurface( *mEGL );

  // delete the GL context / egl surface
  mEGL->TerminateGles();
}

bool RenderThread::PreRender()
{
  bool success = mSurface->PreRender( *mEGL, mGLES );
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

  // Inform the surface that rendering this frame has finished.
  mSurface->PostRender( *mEGL, mGLES, timeDelta, mSurfaceReplaced );
  mSurfaceReplaced = false;
}


} // namespace Adaptor

} // namespace Internal

} // namespace Dali
