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

// CLASS HEADER
#include <dali/internal/window-system/common/window-render-surface.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/trigger-event-factory-interface.h>
#include <dali/integration-api/thread-synchronization-interface.h>
#include <dali/internal/graphics/gles20/egl-implementation.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-system.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace
{

const int MINIMUM_DIMENSION_CHANGE( 1 ); ///< Minimum change for window to be considered to have moved

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_WINDOW_RENDER_SURFACE");
#endif

} // unnamed namespace

WindowRenderSurface::WindowRenderSurface( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mPositionSize( positionSize ),
  mWindowBase(),
  mThreadSynchronization( NULL ),
  mRenderNotification( NULL ),
  mRotationTrigger( NULL ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mOutputTransformedSignal(),
  mRotationAngle( 0 ),
  mScreenRotationAngle( 0 ),
  mOwnSurface( false ),
  mRotationSupported( false ),
  mRotationFinished( true ),
  mScreenRotationFinished( true ),
  mResizeFinished( true )
{
  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "Creating Window\n" );
  Initialize( surface );
}

WindowRenderSurface::~WindowRenderSurface()
{
  if( mRotationTrigger )
  {
    delete mRotationTrigger;
  }
}

void WindowRenderSurface::Initialize( Any surface )
{
  // if width or height are zero, go full screen.
 if ( (mPositionSize.width == 0) || (mPositionSize.height == 0) )
 {
   // Default window size == screen size
   mPositionSize.x = 0;
   mPositionSize.y = 0;

   WindowSystem::GetScreenSize( mPositionSize.width, mPositionSize.height );
 }

  // Create a window base
  auto windowFactory = Dali::Internal::Adaptor::GetWindowFactory();
  mWindowBase = windowFactory->CreateWindowBase( mPositionSize, surface, ( mColorDepth == COLOR_DEPTH_32 ? true : false ) );

  // Connect signals
  mWindowBase->OutputTransformedSignal().Connect( this, &WindowRenderSurface::OutputTransformed );

  // Check screen rotation
  mScreenRotationAngle = mWindowBase->GetScreenRotationAngle();
  if( mScreenRotationAngle != 0 )
  {
    mScreenRotationFinished = false;
  }
}

Any WindowRenderSurface::GetNativeWindow()
{
  return mWindowBase->GetNativeWindow();
}

int WindowRenderSurface::GetNativeWindowId()
{
  return mWindowBase->GetNativeWindowId();
}

void WindowRenderSurface::Map()
{
  mWindowBase->Show();
}

void WindowRenderSurface::SetRenderNotification( TriggerEventInterface* renderNotification )
{
  mRenderNotification = renderNotification;
}

void WindowRenderSurface::SetTransparency( bool transparent )
{
  mWindowBase->SetTransparency( transparent );
}

void WindowRenderSurface::RequestRotation( int angle, int width, int height )
{
  if( !mRotationSupported )
  {
    DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::Rotate: Rotation is not supported!\n" );
    return;
  }

  if( !mRotationTrigger )
  {
    TriggerEventFactoryInterface& triggerFactory = Internal::Adaptor::Adaptor::GetImplementation( Adaptor::Get() ).GetTriggerEventFactoryInterface();
    mRotationTrigger = triggerFactory.CreateTriggerEvent( MakeCallback( this, &WindowRenderSurface::ProcessRotationRequest ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER );
  }

  mPositionSize.width = width;
  mPositionSize.height = height;

  mRotationAngle = angle;
  mRotationFinished = false;

  mWindowBase->SetWindowRotationAngle( mRotationAngle );

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::Rotate: angle = %d screen rotation = %d\n", mRotationAngle, mScreenRotationAngle );
}

WindowBase* WindowRenderSurface::GetWindowBase()
{
  return mWindowBase.get();
}

WindowBase::OutputSignalType& WindowRenderSurface::OutputTransformedSignal()
{
  return mOutputTransformedSignal;
}

PositionSize WindowRenderSurface::GetPositionSize() const
{
  return mPositionSize;
}

void WindowRenderSurface::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  mWindowBase->GetDpi( dpiHorizontal, dpiVertical );
}

void WindowRenderSurface::InitializeEgl( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );

  eglImpl.ChooseConfig(true, mColorDepth);
}

void WindowRenderSurface::CreateEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  int width, height;
  if( mScreenRotationAngle == 0 || mScreenRotationAngle == 180 )
  {
    width = mPositionSize.width;
    height = mPositionSize.height;
  }
  else
  {
    width = mPositionSize.height;
    height = mPositionSize.width;
  }

  // create the EGL window
  EGLNativeWindowType window = mWindowBase->CreateEglWindow( width, height );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );
  eglImpl.CreateSurfaceWindow( window, mColorDepth );

  // Check rotation capability
  mRotationSupported = mWindowBase->IsEglWindowRotationSupported();

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::CreateEglSurface: w = %d h = %d angle = %d screen rotation = %d\n", mPositionSize.width, mPositionSize.height, mRotationAngle, mScreenRotationAngle );
}

void WindowRenderSurface::DestroyEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );
  eglImpl.DestroySurface();

  mWindowBase->DestroyEglWindow();
}

bool WindowRenderSurface::ReplaceEGLSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  // Destroy the old one
  mWindowBase->DestroyEglWindow();

  int width, height;
  if( mScreenRotationAngle == 0 || mScreenRotationAngle == 180 )
  {
    width = mPositionSize.width;
    height = mPositionSize.height;
  }
  else
  {
    width = mPositionSize.height;
    height = mPositionSize.width;
  }

  // create the EGL window
  EGLNativeWindowType window = mWindowBase->CreateEglWindow( width, height );

  // Set screen rotation
  mScreenRotationFinished = false;

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );
  return eglImpl.ReplaceSurfaceWindow( window );
}

void WindowRenderSurface::MoveResize( Dali::PositionSize positionSize )
{
  bool needToMove = false;
  bool needToResize = false;

  // check moving
  if( (fabs(positionSize.x - mPositionSize.x) > MINIMUM_DIMENSION_CHANGE) ||
      (fabs(positionSize.y - mPositionSize.y) > MINIMUM_DIMENSION_CHANGE) )
  {
    needToMove = true;
  }

  // check resizing
  if( (fabs(positionSize.width - mPositionSize.width) > MINIMUM_DIMENSION_CHANGE) ||
      (fabs(positionSize.height - mPositionSize.height) > MINIMUM_DIMENSION_CHANGE) )
  {
    needToResize = true;
  }

  if( needToResize )
  {
    if( needToMove )
    {
      mWindowBase->MoveResize( positionSize );
    }
    else
    {
      mWindowBase->Resize( positionSize );
    }

    mResizeFinished = false;
    mPositionSize = positionSize;
  }
  else
  {
    if( needToMove )
    {
      mWindowBase->Move( positionSize );

      mPositionSize = positionSize;
    }
  }

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::MoveResize: %d, %d, %d, %d\n", mPositionSize.x, mPositionSize.y, mPositionSize.width, mPositionSize.height );
}

void WindowRenderSurface::SetViewMode( ViewMode viewMode )
{
  mWindowBase->SetViewMode( viewMode );
}

void WindowRenderSurface::StartRender()
{
}

bool WindowRenderSurface::PreRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, bool resizingSurface )
{
  if( resizingSurface )
  {
#ifdef OVER_TIZEN_VERSION_4
    // Window rotate or screen rotate
    if( !mRotationFinished || !mScreenRotationFinished )
    {
      int totalAngle = (mRotationAngle + mScreenRotationAngle) % 360;

      mWindowBase->SetEglWindowRotation( totalAngle );
      mWindowBase->SetEglWindowBufferTransform( totalAngle );

      // Reset only screen rotation flag
      mScreenRotationFinished = true;

      DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::PreRender: Set rotation [%d] [%d]\n", mRotationAngle, mScreenRotationAngle );
    }

    // Only window rotate
    if( !mRotationFinished )
    {
      mWindowBase->SetEglWindowTransform( mRotationAngle );
    }
#endif

    // Resize case
    if( !mResizeFinished )
    {
      mWindowBase->ResizeEglWindow( mPositionSize );
      mResizeFinished = true;

      DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::PreRender: Set resize\n" );
    }
  }

  return true;
}

void WindowRenderSurface::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, Dali::DisplayConnection* displayConnection, bool replacingSurface, bool resizingSurface )
{
  if( resizingSurface )
  {
    if( !mRotationFinished )
    {
      DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::PostRender: Trigger rotation event\n" );

      mRotationTrigger->Trigger();

      if( mThreadSynchronization )
      {
        // Wait until the event-thread complete the rotation event processing
        mThreadSynchronization->PostRenderWaitForCompletion();
      }
    }
  }

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );
  eglImpl.SwapBuffers();

  if( mRenderNotification )
  {
    mRenderNotification->Trigger();
  }
}

void WindowRenderSurface::StopRender()
{
}

void WindowRenderSurface::SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )
{
  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::SetThreadSynchronization: called\n" );

  mThreadSynchronization = &threadSynchronization;
}

void WindowRenderSurface::ReleaseLock()
{
  // Nothing to do.
}

RenderSurface::Type WindowRenderSurface::GetSurfaceType()
{
  return RenderSurface::WINDOW_RENDER_SURFACE;
}

void WindowRenderSurface::OutputTransformed()
{
  int screenRotationAngle = mWindowBase->GetScreenRotationAngle();

  if( mScreenRotationAngle != screenRotationAngle )
  {
    mScreenRotationAngle = screenRotationAngle;
    mScreenRotationFinished = false;

    mOutputTransformedSignal.Emit();

    DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::OutputTransformed: angle = %d screen rotation = %d\n", mRotationAngle, mScreenRotationAngle );
  }
  else
  {
    DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::OutputTransformed: Ignore output transform [%d]\n", mScreenRotationAngle );
  }
}

void WindowRenderSurface::ProcessRotationRequest()
{
  mRotationFinished = true;

  mWindowBase->WindowRotationCompleted( mRotationAngle, mPositionSize.width, mPositionSize.height );

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::ProcessRotationRequest: Rotation Done\n" );

  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
