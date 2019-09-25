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

// CLASS HEADER
#include <dali/internal/window-system/common/window-render-surface.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/trigger-event-factory-interface.h>
#include <dali/integration-api/thread-synchronization-interface.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/system/common/environment-variables.h>


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
: mEGL( nullptr ),
  mDisplayConnection( nullptr ),
  mPositionSize( positionSize ),
  mWindowBase(),
  mThreadSynchronization( NULL ),
  mRenderNotification( NULL ),
  mRotationTrigger( NULL ),
  mGraphics( nullptr ),
  mEGLSurface( nullptr ),
  mEGLContext( nullptr ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mOutputTransformedSignal(),
  mRotationAngle( 0 ),
  mScreenRotationAngle( 0 ),
  mOwnSurface( false ),
  mRotationSupported( false ),
  mRotationFinished( true ),
  mScreenRotationFinished( true ),
  mResizeFinished( true ),
  mDpiHorizontal( 0 ),
  mDpiVertical( 0 )
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

  if ( mEGLSurface )
  {
    DestroySurface();
  }
}

void WindowRenderSurface::Initialize( Any surface )
{
  // If width or height are zero, go full screen.
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
  if( mDpiHorizontal == 0 || mDpiVertical == 0 )
  {
    const char* environmentDpiHorizontal = std::getenv( DALI_ENV_DPI_HORIZONTAL );
    mDpiHorizontal = environmentDpiHorizontal ? std::atoi( environmentDpiHorizontal ) : 0;

    const char* environmentDpiVertical = std::getenv( DALI_ENV_DPI_VERTICAL );
    mDpiVertical = environmentDpiVertical ? std::atoi( environmentDpiVertical ) : 0;

    if( mDpiHorizontal == 0 || mDpiVertical == 0 )
    {
      mWindowBase->GetDpi( mDpiHorizontal, mDpiVertical );
    }
  }

  dpiHorizontal = mDpiHorizontal;
  dpiVertical = mDpiVertical;
}

void WindowRenderSurface::InitializeGraphics()
{

  mGraphics = &mAdaptor->GetGraphicsInterface();

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);
  mEGL = &eglGraphics->GetEglInterface();

  if ( mEGLContext == NULL )
  {
    // Create the OpenGL context for this window
    Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>(*mEGL);
    eglImpl.ChooseConfig(true, mColorDepth);
    eglImpl.CreateWindowContext( mEGLContext );

    // Create the OpenGL surface
    CreateSurface();
  }
}

void WindowRenderSurface::CreateSurface()
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

  // Create the EGL window
  EGLNativeWindowType window = mWindowBase->CreateEglWindow( width, height );

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
  mEGLSurface = eglImpl.CreateSurfaceWindow( window, mColorDepth );

  // Check rotation capability
  mRotationSupported = mWindowBase->IsEglWindowRotationSupported();

  DALI_LOG_RELEASE_INFO("WindowRenderSurface::CreateSurface: w = %d h = %d angle = %d screen rotation = %d\n",
      mPositionSize.width, mPositionSize.height, mRotationAngle, mScreenRotationAngle );
}

void WindowRenderSurface::DestroySurface()
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
  eglImpl.DestroySurface( mEGLSurface );

  mWindowBase->DestroyEglWindow();
}

bool WindowRenderSurface::ReplaceGraphicsSurface()
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

  // Create the EGL window
  EGLNativeWindowType window = mWindowBase->CreateEglWindow( width, height );

  // Set screen rotation
  mScreenRotationFinished = false;

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
  return eglImpl.ReplaceSurfaceWindow( window, mEGLSurface, mEGLContext );
}

void WindowRenderSurface::MoveResize( Dali::PositionSize positionSize )
{
  bool needToMove = false;
  bool needToResize = false;

  // Check moving
  if( (fabs(positionSize.x - mPositionSize.x) > MINIMUM_DIMENSION_CHANGE) ||
      (fabs(positionSize.y - mPositionSize.y) > MINIMUM_DIMENSION_CHANGE) )
  {
    needToMove = true;
  }

  // Check resizing
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

void WindowRenderSurface::StartRender()
{
}

bool WindowRenderSurface::PreRender( bool resizingSurface )
{
  MakeContextCurrent();

  if( resizingSurface )
  {
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

    // Resize case
    if( !mResizeFinished )
    {
      mWindowBase->ResizeEglWindow( mPositionSize );
      mResizeFinished = true;

      DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::PreRender: Set resize\n" );
    }
  }

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);
  if ( eglGraphics )
  {
    GlImplementation& mGLES = eglGraphics->GetGlesInterface();
    mGLES.PreRender();
  }

  return true;
}

void WindowRenderSurface::PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface )
{
  // Inform the gl implementation that rendering has finished before informing the surface
  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);
  if ( eglGraphics )
  {
    GlImplementation& mGLES = eglGraphics->GetGlesInterface();
    mGLES.PostRender();

    if( renderToFbo )
    {
      mGLES.Flush();
      mGLES.Finish();
    }
    else
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
    }

    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
    eglImpl.SwapBuffers( mEGLSurface );

    if( mRenderNotification )
    {
      mRenderNotification->Trigger();
    }
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

Integration::RenderSurface::Type WindowRenderSurface::GetSurfaceType()
{
  return RenderSurface::WINDOW_RENDER_SURFACE;
}

void WindowRenderSurface::MakeContextCurrent()
{
  if ( mEGL != nullptr )
  {
    mEGL->MakeContextCurrent( mEGLSurface, mEGLContext );
  }
}

Integration::DepthBufferAvailable WindowRenderSurface::GetDepthBufferRequired()
{
  return mGraphics ? mGraphics->GetDepthBufferRequired() : Integration::DepthBufferAvailable::FALSE;
}

Integration::StencilBufferAvailable WindowRenderSurface::GetStencilBufferRequired()
{
  return mGraphics ? mGraphics->GetStencilBufferRequired() : Integration::StencilBufferAvailable::FALSE;
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
