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

const unsigned int TIME_PER_FRAME_IN_MICROSECONDS = 16667;

} // unnamed namespace

RenderThread::RenderThread( UpdateRenderSynchronization& sync,
                            AdaptorInternalServices& adaptorInterfaces,
                            const EnvironmentOptions& environmentOptions )
: mUpdateRenderSync( sync ),
  mCore( adaptorInterfaces.GetCore() ),
  mGLES( adaptorInterfaces.GetGlesInterface() ),
  mEglFactory( &adaptorInterfaces.GetEGLFactoryInterface()),
  mEGL( NULL ),
  mThread( NULL ),
  mSurfaceReplacing( false ),
  mNewDataAvailable( false ),
  mSurfaceReplaceCompleted( false ),
  mEnvironmentOptions( environmentOptions )
{
  // set the initial values before render thread starts
  mCurrent.surface = adaptorInterfaces.GetRenderSurfaceInterface();
}

RenderThread::~RenderThread()
{
  DALI_ASSERT_ALWAYS( mThread == NULL && "RenderThread is still alive");
  mEglFactory->Destroy();
}

void RenderThread::Start()
{
  // initialise GL and kick off render thread
  DALI_ASSERT_ALWAYS( !mEGL && "Egl already initialized" );

  // Tell core what the minimum frame interval is
  mCore.SetMinimumFrameTimeInterval( mCurrent.syncMode * TIME_PER_FRAME_IN_MICROSECONDS );

  // create the render thread, initially we are rendering
  mThread = new boost::thread(boost::bind(&RenderThread::Run, this));

  // Inform surface to block waiting for RenderSync
  mCurrent.surface->SetSyncMode( RenderSurface::SYNC_MODE_WAIT );
 }

void RenderThread::Stop()
{
  // shutdown the render thread and destroy the opengl context
  if( mThread )
  {
    // Tell surface we have stopped rendering
    mCurrent.surface->StopRender();

    // wait for the thread to finish
    mThread->join();

    delete mThread;
    mThread = NULL;
  }
}

void RenderThread::ReplaceSurface( RenderSurface* surface )
{
  // Make sure it's a new surface. Note! we are reading the current value of render thread here, but reading is ok
  DALI_ASSERT_ALWAYS( surface != mCurrent.surface && "Trying to replace surface with itself" );

  // lock and set to false
  {
    boost::unique_lock<boost::mutex> lock( mSurfaceChangedMutex );
    mSurfaceReplaceCompleted = false;
  }

  // lock cache and set update flag at the end of function
  {
    SendMessageGuard msg( *this );
    // set new values to cache
    mNewValues.replaceSurface = true;
    mNewValues.surface = surface;
  }

  /*
   * Reset the mPixmapFlushed condition if surface was changed.
   * : in this case, client can not handle the previous damage because surface was changed.
   */
  RenderSync();
}

void RenderThread::WaitForSurfaceReplaceComplete()
{
  boost::unique_lock<boost::mutex> lock( mSurfaceChangedMutex );

  // if already completed no need to wait
  while( !mSurfaceReplaceCompleted )
  {
    mSurfaceChangedNotify.wait( lock ); // Block the main thread here and releases mSurfaceChangedMutex so the render-thread can notify us
  }
}

void RenderThread::SetVSyncMode( EglInterface::SyncMode syncMode )
{
  // lock cache and set update flag at the end of function
  SendMessageGuard msg( *this );
  // set new values to cache
  mNewValues.syncMode = syncMode;
}

void RenderThread::RenderSync()
{
  mCurrent.surface->RenderSync();
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

  // Wait for first update
  mUpdateRenderSync.RenderSyncWithUpdate();

  Dali::Integration::RenderStatus renderStatus;

  uint64_t currentTime( 0 );

  // render loop, we stay inside here when rendering
  while( running )
  {
    // Consume any pending events
    ConsumeEvents();

    // Check if we've got updates from the main thread
    CheckForUpdates();

    // perform any pre-render operations
    if(PreRender() == true)
    {
       // Render
      mCore.Render( renderStatus );

      // Notify the update-thread that a render has completed
      mUpdateRenderSync.RenderFinished( renderStatus.NeedsUpdate() );

      uint64_t newTime( mUpdateRenderSync.GetTimeMicroseconds() );

      // perform any post-render operations
      if ( renderStatus.HasRendered() )
      {
        PostRender( static_cast< unsigned int >(newTime - currentTime) );
      }

      if(mSurfaceReplacing)
      {
        // Notify main thread that surface was changed so it can release the old one
        NotifySurfaceChangeCompleted();
        mSurfaceReplacing = false;
      }

      currentTime = newTime;
    }

    // Wait until another frame has been updated
    running = mUpdateRenderSync.RenderSyncWithUpdate();
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

  DALI_ASSERT_ALWAYS( mCurrent.surface && "NULL surface" );

  // initialize egl & OpenGL
  mCurrent.surface->InitializeEgl( *mEGL );

  // create the OpenGL context
  mEGL->CreateContext();

  // create the OpenGL surface
  mCurrent.surface->CreateEglSurface( *mEGL );

  // Make it current
  mEGL->MakeContextCurrent();

  // set the initial sync mode
  mEGL->SetRefreshSync( mCurrent.syncMode );

  // tell core it has a context
  mCore.ContextCreated();

}

void RenderThread::ConsumeEvents()
{
  // tell surface to consume any events to avoid memory leaks
  mCurrent.surface->ConsumeEvents();
}

void RenderThread::CheckForUpdates()
{
  // atomic check to see if we've got updates, resets the flag int
  if( __sync_fetch_and_and( &mNewDataAvailable, 0 ) )
  {
    // scope for lock
    // NOTE! This block is the only place in render side where mNewValues can be used inside render thread !!!
    {
      // need to lock to access new values
      boost::unique_lock< boost::mutex > lock( mThreadDataLock );

      // did the sync mode change
      if( mCurrent.syncMode != mNewValues.syncMode )
      {
        mCurrent.syncMode = mNewValues.syncMode;
        mEGL->SetRefreshSync( mCurrent.syncMode );
      }

      // check if the surface needs replacing
      if( mNewValues.replaceSurface )
      {
        mNewValues.replaceSurface = false; // reset the flag
        // change the surface
        ChangeSurface( mNewValues.surface );
        mNewValues.surface = NULL;
      }
    }
  }
}

void RenderThread::ChangeSurface( RenderSurface* newSurface )
{
  // This is designed for replacing pixmap surfaces, but should work for window as well
  // we need to delete the egl surface and renderable (pixmap / window)
  // Then create a new pixmap/window and new egl surface
  // If the new surface has a different display connection, then the context will be lost
  DALI_ASSERT_ALWAYS( newSurface && "NULL surface" )
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
  mCurrent.surface->TransferDisplayOwner( *newSurface );

  // use the new surface from now on
  mCurrent.surface = newSurface;

  // after rendering, NotifySurfaceChangeCompleted will be called
  mSurfaceReplacing = true;
}

void RenderThread::NotifySurfaceChangeCompleted()
{
  {
    boost::unique_lock< boost::mutex > lock( mSurfaceChangedMutex );
    mSurfaceReplaceCompleted = true;
  }
  // notify main thread
  mSurfaceChangedNotify.notify_all();
}

void RenderThread::ShutdownEgl()
{
  // inform core the context is about to be destroyed,
  mCore.ContextToBeDestroyed();

  // give a chance to destroy the OpenGL surface that created externally
  mCurrent.surface->DestroyEglSurface( *mEGL );

  // delete the GL context / egl surface
  mEGL->TerminateGles();
}

bool RenderThread::PreRender()
{
  bool success = mCurrent.surface->PreRender( *mEGL, mGLES );
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
  mCurrent.surface->PostRender( *mEGL, mGLES, timeDelta,
                                mSurfaceReplacing ? RenderSurface::SYNC_MODE_NONE : RenderSurface::SYNC_MODE_WAIT );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
