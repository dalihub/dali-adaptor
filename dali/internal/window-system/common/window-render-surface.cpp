/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/adaptor-framework/thread-synchronization-interface.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-system.h>
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
const float FULL_UPDATE_RATIO( 0.8f );   ///< Force full update when the dirty area is larget than this ratio

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_WINDOW_RENDER_SURFACE");
#endif

void MergeRects( Rect< int >& mergingRect, const std::vector< Rect< int > >& rects )
{
  uint32_t i = 0;
  if( mergingRect.IsEmpty() )
  {
    for( ; i < rects.size(); i++ )
    {
      if( !rects[i].IsEmpty() )
      {
        mergingRect = rects[i];
        break;
      }
    }
  }

  for( ; i < rects.size(); i++ )
  {
    mergingRect.Merge( rects[i] );
  }
}

void InsertRects( WindowRenderSurface::DamagedRectsContainer& damagedRectsList, const std::vector< Rect< int > >& damagedRects )
{
  damagedRectsList.push_front( damagedRects );
  if( damagedRectsList.size() > 4 ) // past triple buffers + current
  {
    damagedRectsList.pop_back();
  }
}

} // unnamed namespace

WindowRenderSurface::WindowRenderSurface( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mEGL( nullptr ),
  mDisplayConnection( nullptr ),
  mPositionSize( positionSize ),
  mWindowBase(),
  mThreadSynchronization( NULL ),
  mRenderNotification( NULL ),
  mRotationTrigger( NULL ),
  mFrameRenderedTrigger(),
  mGraphics( nullptr ),
  mEGLSurface( nullptr ),
  mEGLContext( nullptr ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mOutputTransformedSignal(),
  mFrameCallbackInfoContainer(),
  mBufferDamagedRects(),
  mMutex(),
  mRotationAngle( 0 ),
  mScreenRotationAngle( 0 ),
  mDpiHorizontal( 0 ),
  mDpiVertical( 0 ),
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
    mRotationTrigger = TriggerEventFactory::CreateTriggerEvent( MakeCallback( this, &WindowRenderSurface::ProcessRotationRequest ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER );
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

  DALI_ASSERT_ALWAYS( mGraphics && "Graphics interface is not created" );

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

  DALI_LOG_RELEASE_INFO("WindowRenderSurface::CreateSurface: WinId (%d), w = %d h = %d angle = %d screen rotation = %d\n",
      mWindowBase->GetNativeWindowId(), mPositionSize.width, mPositionSize.height, mRotationAngle, mScreenRotationAngle );
}

void WindowRenderSurface::DestroySurface()
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);
  if( eglGraphics )
  {
    DALI_LOG_RELEASE_INFO("WindowRenderSurface::DestroySurface: WinId (%d)\n", mWindowBase->GetNativeWindowId() );

    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

    eglImpl.DestroySurface( mEGLSurface );
    mEGLSurface = nullptr;

    // Destroy context also
    eglImpl.DestroyContext( mEGLContext );
    mEGLContext = nullptr;

    mWindowBase->DestroyEglWindow();
  }
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

bool WindowRenderSurface::PreRender( bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect )
{
  Dali::Integration::Scene::FrameCallbackContainer callbacks;

  Dali::Integration::Scene scene = mScene.GetHandle();
  if( scene )
  {
    bool needFrameRenderedTrigger = false;

    scene.GetFrameRenderedCallback( callbacks );
    if( !callbacks.empty() )
    {
      int frameRenderedSync = mWindowBase->CreateFrameRenderedSyncFence();
      if( frameRenderedSync != -1 )
      {
        Dali::Mutex::ScopedLock lock( mMutex );

        DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::PreRender: CreateFrameRenderedSyncFence [%d]\n", frameRenderedSync );

        mFrameCallbackInfoContainer.push_back( std::unique_ptr< FrameCallbackInfo >( new FrameCallbackInfo( callbacks, frameRenderedSync ) ) );

        needFrameRenderedTrigger = true;
      }
      else
      {
        DALI_LOG_ERROR( "WindowRenderSurface::PreRender: CreateFrameRenderedSyncFence is failed\n" );
      }

      // Clear callbacks
      callbacks.clear();
    }

    scene.GetFramePresentedCallback( callbacks );
    if( !callbacks.empty() )
    {
      int framePresentedSync = mWindowBase->CreateFramePresentedSyncFence();
      if( framePresentedSync != -1 )
      {
        Dali::Mutex::ScopedLock lock( mMutex );

        DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::PreRender: CreateFramePresentedSyncFence [%d]\n", framePresentedSync );

        mFrameCallbackInfoContainer.push_back( std::unique_ptr< FrameCallbackInfo >( new FrameCallbackInfo( callbacks, framePresentedSync ) ) );

        needFrameRenderedTrigger = true;
      }
      else
      {
        DALI_LOG_ERROR( "WindowRenderSurface::PreRender: CreateFramePresentedSyncFence is failed\n" );
      }

      // Clear callbacks
      callbacks.clear();
    }

    if( needFrameRenderedTrigger )
    {
      if( !mFrameRenderedTrigger )
      {
        mFrameRenderedTrigger = std::unique_ptr< TriggerEventInterface >( TriggerEventFactory::CreateTriggerEvent( MakeCallback( this, &WindowRenderSurface::ProcessFrameCallback ),
                                                                                                                   TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER ) );
      }
      mFrameRenderedTrigger->Trigger();
    }
  }

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

    SetFullSwapNextFrame();
  }

  SetBufferDamagedRects( damagedRects, clippingRect );

  return true;
}

void WindowRenderSurface::PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface, const std::vector<Rect<int>>& damagedRects )
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
          if( mThreadSynchronization )
          {
            // Enable PostRender flag
            mThreadSynchronization->PostRenderStarted();
          }

          DALI_LOG_RELEASE_INFO("WindowRenderSurface::PostRender: Trigger rotation event\n" );

          mRotationTrigger->Trigger();

          if( mThreadSynchronization )
          {
            // Wait until the event-thread complete the rotation event processing
            mThreadSynchronization->PostRenderWaitForCompletion();
          }
        }
      }
    }

    SwapBuffers( damagedRects );

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

Dali::RenderSurfaceInterface::Type WindowRenderSurface::GetSurfaceType()
{
  return Dali::RenderSurfaceInterface::WINDOW_RENDER_SURFACE;
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

void WindowRenderSurface::ProcessFrameCallback()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  for( auto&& iter : mFrameCallbackInfoContainer )
  {
    if( !iter->fileDescriptorMonitor )
    {
      iter->fileDescriptorMonitor = std::unique_ptr< FileDescriptorMonitor >( new FileDescriptorMonitor( iter->fileDescriptor,
                                                                             MakeCallback( this, &WindowRenderSurface::OnFileDescriptorEventDispatched ), FileDescriptorMonitor::FD_READABLE ) );

      DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::ProcessFrameCallback: Add handler [%d]\n", iter->fileDescriptor );
    }
  }
}

void WindowRenderSurface::OnFileDescriptorEventDispatched( FileDescriptorMonitor::EventType eventBitMask, int fileDescriptor )
{
  if( !( eventBitMask & FileDescriptorMonitor::FD_READABLE ) )
  {
    DALI_LOG_ERROR( "WindowRenderSurface::OnFileDescriptorEventDispatched: file descriptor error [%d]\n", eventBitMask );
    close( fileDescriptor );
    return;
  }

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::OnFileDescriptorEventDispatched: Frame rendered [%d]\n", fileDescriptor );

  std::unique_ptr< FrameCallbackInfo > callbackInfo;
  {
    Dali::Mutex::ScopedLock lock( mMutex );
    auto frameCallbackInfo = std::find_if( mFrameCallbackInfoContainer.begin(), mFrameCallbackInfoContainer.end(),
                                      [fileDescriptor]( std::unique_ptr< FrameCallbackInfo >& callbackInfo )
                                      {
                                        return callbackInfo->fileDescriptor == fileDescriptor;
                                      } );
    if( frameCallbackInfo != mFrameCallbackInfoContainer.end() )
    {
      callbackInfo = std::move( *frameCallbackInfo );

      mFrameCallbackInfoContainer.erase( frameCallbackInfo );
    }
  }

  // Call the connected callback
  if( callbackInfo )
  {
    for( auto&& iter : ( callbackInfo )->callbacks )
    {
      CallbackBase::Execute( *( iter.first ), iter.second );
    }
  }
}

void WindowRenderSurface::SetBufferDamagedRects( const std::vector< Rect< int > >& damagedRects, Rect< int >& clippingRect )
{
  auto eglGraphics = static_cast< EglGraphics* >( mGraphics );
  if ( eglGraphics )
  {
    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
    if( !eglImpl.IsPartialUpdateRequired() )
    {
      return;
    }

    Rect< int > surfaceRect( 0, 0, mPositionSize.width, mPositionSize.height );

    if( mFullSwapNextFrame )
    {
      InsertRects( mBufferDamagedRects, std::vector< Rect< int > >( 1, surfaceRect ) );
      clippingRect = Rect< int >();
      return;
    }

    EGLint bufferAge = eglImpl.GetBufferAge( mEGLSurface );

    // Buffer age 0 means the back buffer in invalid and requires full swap
    if( !damagedRects.size() || bufferAge == 0 )
    {
      InsertRects( mBufferDamagedRects, std::vector< Rect< int > >( 1, surfaceRect ) );
      clippingRect = Rect< int >();
      return;
    }

    // We push current frame damaged rects here, zero index for current frame
    InsertRects( mBufferDamagedRects, damagedRects );

    // Merge damaged rects into clipping rect
    auto bufferDamagedRects = mBufferDamagedRects.begin();
    while( bufferAge-- >= 0 && bufferDamagedRects != mBufferDamagedRects.end() )
    {
      const std::vector< Rect< int > >& rects = *bufferDamagedRects++;
      MergeRects( clippingRect, rects );
    }

    if( !clippingRect.Intersect( surfaceRect ) || clippingRect.Area() > surfaceRect.Area() * FULL_UPDATE_RATIO )
    {
      // clipping area too big or doesn't intersect surface rect
      clippingRect = Rect< int >();
      return;
    }

    std::vector< Rect< int > > damagedRegion;
    damagedRegion.push_back( clippingRect );

    eglImpl.SetDamageRegion( mEGLSurface, damagedRegion );
  }
}

void WindowRenderSurface::SwapBuffers( const std::vector<Rect<int>>& damagedRects )
{
  auto eglGraphics = static_cast< EglGraphics* >( mGraphics );
  if( eglGraphics )
  {
    Rect< int > surfaceRect( 0, 0, mPositionSize.width, mPositionSize.height );

    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

    if( !eglImpl.IsPartialUpdateRequired() || mFullSwapNextFrame || !damagedRects.size() || (damagedRects[0].Area() > surfaceRect.Area() * FULL_UPDATE_RATIO) )
    {
      mFullSwapNextFrame = false;
      eglImpl.SwapBuffers( mEGLSurface );
      return;
    }

    mFullSwapNextFrame = false;

    std::vector< Rect< int > > mergedRects = damagedRects;

    // Merge intersecting rects, form an array of non intersecting rects to help driver a bit
    // Could be optional and can be removed, needs to be checked with and without on platform
    const int n = mergedRects.size();
    for( int i = 0; i < n - 1; i++ )
    {
      if( mergedRects[i].IsEmpty() )
      {
        continue;
      }

      for( int j = i + 1; j < n; j++ )
      {
        if( mergedRects[j].IsEmpty() )
        {
          continue;
        }

        if( mergedRects[i].Intersects( mergedRects[j] ) )
        {
          mergedRects[i].Merge( mergedRects[j] );
          mergedRects[j].width = 0;
          mergedRects[j].height = 0;
        }
      }
    }

    int j = 0;
    for( int i = 0; i < n; i++ )
    {
      if( !mergedRects[i].IsEmpty() )
      {
        mergedRects[j++] = mergedRects[i];
      }
    }

    if( j != 0 )
    {
      mergedRects.resize( j );
    }

    if( !mergedRects.size() || ( mergedRects[0].Area() > surfaceRect.Area() * FULL_UPDATE_RATIO ) )
    {
      eglImpl.SwapBuffers( mEGLSurface );
    }
    else
    {
      eglImpl.SwapBuffers( mEGLSurface, mergedRects );
    }
  }
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
