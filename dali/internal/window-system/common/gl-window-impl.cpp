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
#include <dali/internal/window-system/common/gl-window-impl.h>

// EXTERNAL HEADERS
#include <dali/integration-api/core.h>
#include <dali/devel-api/adaptor-framework/orientation.h>
#include <dali/devel-api/adaptor-framework/gl-window.h>
#include <dali/integration-api/events/touch-integ.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/devel-api/events/key-event-devel.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/window-system/common/orientation-impl.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-visibility-observer.h>
#include <dali/internal/graphics/gles/egl-graphics-factory.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace
{
const int MINIMUM_DIMENSION_CHANGE( 1 );

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_WINDOW" );
#endif

} // unnamed namespace

GlWindow* GlWindow::New( const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent )
{
  GlWindow* window = new GlWindow();
  window->mIsTransparent = isTransparent;
  window->Initialize( positionSize, name, className );
  return window;
}

GlWindow::GlWindow()
: mWindowBase(),
  mGraphics(),
  mDisplayConnection( nullptr ),
  mEventHandler( nullptr ),
  mPositionSize(),
  mColorDepth( COLOR_DEPTH_24 ),
  mIsTransparent( false ),
  mIsFocusAcceptable( false ),
  mIconified( false ),
  mOpaqueState( false ),
  mResizeEnabled( false ),
  mVisible( false ),
  mIsRotated( false ),
  mIsWindowRotated( false ),
  mIsTouched( false ),
  mAvailableAngles(),
  mPreferredAngle( 0 ),
  mTotalRotationAngle( 0 ),
  mWindowRotationAngle( 0 ),
  mScreenRotationAngle( 0 ),
  mOrientationMode( 0 ),
  mWindowWidth( 0 ),
  mWindowHeight( 0 ),
  mNativeWindowId( -1 ),
  mKeyEventSignal(),
  mTouchedSignal(),
  mFocusChangeSignal(),
  mResizeSignal(),
  mVisibilityChangedSignal(),
  mGLInitCallback(),
  mGLRenderFrameCallback(),
  mGLTerminateCallback(),
  mGLRenderCallback( nullptr ),
  mEGLSurface( nullptr ),
  mEGLContext( nullptr ),
  mGLESVersion( Dali::GlWindow::GlesVersion::VERSION_3_0 ),
  mInitCallback( false ),
  mDepth( false ),
  mStencil( false ),
  mIsEGLInitialize( false ),
  mMSAA( 0 )
{
}

GlWindow::~GlWindow()
{
  if( mEventHandler )
  {
    mEventHandler->RemoveObserver( *this );
  }

  if( mGLTerminateCallback )
  {
    CallbackBase::Execute(*mGLTerminateCallback);
  }

  if( mIsEGLInitialize )
  {
    GraphicsInterface* graphics = mGraphics.get();
    EglGraphics *eglGraphics = static_cast<EglGraphics*>( graphics );
    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

    if( mEGLSurface )
    {
      eglImpl.DestroySurface( mEGLSurface );
      mEGLSurface = nullptr;
    }

    if( mEGLContext )
    {
      eglImpl.DestroyContext( mEGLContext );
      mEGLContext = nullptr;
    }

    eglImpl.TerminateGles();

    mGraphics->Destroy();
  }
}

void GlWindow::Initialize( const PositionSize& positionSize, const std::string& name, const std::string& className )
{
  int screenWidth, screenHeight;

  mPositionSize = positionSize;
  WindowSystem::GetScreenSize( screenWidth, screenHeight );
  if ( (mPositionSize.width == 0) || (mPositionSize.height == 0) )
  {
    mPositionSize.x = 0;
    mPositionSize.y = 0;
    mPositionSize.width = screenWidth;
    mPositionSize.height = screenHeight;
  }

  if( screenWidth > screenHeight )
  {
    mOrientationMode = 1; // Default mode is landscape
  }
  else
  {
    mOrientationMode = 0; // Default mode is portrate
  }

  // Create a window base
  auto windowFactory = Dali::Internal::Adaptor::GetWindowFactory();
  Any surface;
  mWindowBase = windowFactory->CreateWindowBase( mPositionSize, surface, ( mIsTransparent ? true : false ) );
  mWindowBase->IconifyChangedSignal().Connect( this, &GlWindow::OnIconifyChanged );
  mWindowBase->FocusChangedSignal().Connect( this, &GlWindow::OnFocusChanged );

  if( Dali::Adaptor::IsAvailable() )
  {
    SetEventHandler();
  }

  if( !mPositionSize.IsEmpty() )
  {
    AddAuxiliaryHint( "wm.policy.win.user.geometry", "1" );
    mResizeEnabled = true;
  }

  mWindowBase->Show();

  if( mIsTransparent )
  {
    mColorDepth = COLOR_DEPTH_32;
  }
  else
  {
    mColorDepth = COLOR_DEPTH_24;
  }

  SetClass( name, className );

  // For Debugging
  mNativeWindowId = mWindowBase->GetNativeWindowId();
}

void GlWindow::SetEventHandler()
{
  mEventHandler = EventHandlerPtr( new EventHandler( mWindowBase.get(), *this ) );
  mEventHandler->AddObserver( *this );
}

void GlWindow::SetClass( const std::string& name, const std::string className )
{
  mName = name;
  mClassName = className;
  mWindowBase->SetClass( name, className );
}

void GlWindow::SetEglConfig( bool depth, bool stencil, int msaa, Dali::GlWindow::GlesVersion version )
{
  // Init Graphics
  mDepth = depth;
  mStencil = stencil;
  mMSAA = msaa;
  mGLESVersion = version;

  InitializeGraphics();
}

void GlWindow::Raise()
{
  mWindowBase->Raise();
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Raise() \n", this, mNativeWindowId );
}

void GlWindow::Lower()
{
  mWindowBase->Lower();
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Lower() \n", this, mNativeWindowId );
}

void GlWindow::Activate()
{
  mWindowBase->Activate();
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Activate() \n", this, mNativeWindowId );
}

void GlWindow::Show()
{
  mVisible = true;

  mWindowBase->Show();

  if( !mIconified )
  {
    Dali::GlWindow handle( this );
    mVisibilityChangedSignal.Emit( handle, true );
  }

  if( mEventHandler )
  {
    mEventHandler->Resume();
  }

  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Show(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible );
}

void GlWindow::Hide()
{
  mVisible = false;

  mWindowBase->Hide();

  if( !mIconified )
  {
    Dali::GlWindow handle( this );
    mVisibilityChangedSignal.Emit( handle, false );
  }

  if( mEventHandler )
  {
    mEventHandler->Pause();
  }

  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Hide(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible );
}

unsigned int GlWindow::GetSupportedAuxiliaryHintCount() const
{
  return mWindowBase->GetSupportedAuxiliaryHintCount();
}

std::string GlWindow::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  return mWindowBase->GetSupportedAuxiliaryHint( index );
}

unsigned int GlWindow::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return mWindowBase->AddAuxiliaryHint( hint, value );
}

bool GlWindow::RemoveAuxiliaryHint( unsigned int id )
{
  return mWindowBase->RemoveAuxiliaryHint( id );
}

bool GlWindow::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return mWindowBase->SetAuxiliaryHintValue( id, value );
}

std::string GlWindow::GetAuxiliaryHintValue( unsigned int id ) const
{
  return mWindowBase->GetAuxiliaryHintValue( id );
}

unsigned int GlWindow::GetAuxiliaryHintId( const std::string& hint ) const
{
  return mWindowBase->GetAuxiliaryHintId( hint );
}

void GlWindow::SetInputRegion( const Rect< int >& inputRegion )
{
  mWindowBase->SetInputRegion( inputRegion );

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "GlWindow::SetInputRegion: x = %d, y = %d, w = %d, h = %d\n", inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height );
}

void GlWindow::SetOpaqueState( bool opaque )
{
  mOpaqueState = opaque;

  mWindowBase->SetOpaqueState( opaque );

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "GlWindow::SetOpaqueState: opaque = %d\n", opaque );
}

bool GlWindow::IsOpaqueState() const
{
  return mOpaqueState;
}

void GlWindow::SetPositionSize( PositionSize positionSize )
{
  if( !mResizeEnabled )
  {
    AddAuxiliaryHint( "wm.policy.win.user.geometry", "1" );
    mResizeEnabled = true;
  }

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

  // If window's size or position is changed, the signal will be emitted to user.
  if( ( needToMove ) || ( needToResize ) )
  {
    Uint16Pair newSize( mPositionSize.width, mPositionSize.height );
    Dali::GlWindow handle( this );
    mResizeSignal.Emit( newSize );
  }
}

PositionSize GlWindow::GetPositionSize() const
{
  PositionSize positionSize( mPositionSize );
  if( mTotalRotationAngle == 90 || mTotalRotationAngle == 270 )
  {
    positionSize.height = mPositionSize.width;
    positionSize.width = mPositionSize.height;
  }

  return positionSize;
}

void GlWindow::OnIconifyChanged( bool iconified )
{
  if( iconified )
  {
    mIconified = true;

    if( mVisible )
    {
      Dali::GlWindow handle( this );
      mVisibilityChangedSignal.Emit( handle, false );
    }

    if( mEventHandler )
    {
      mEventHandler->Pause();
    }

    DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Iconified: visible = %d\n", this, mNativeWindowId, mVisible );
  }
  else
  {
    mIconified = false;

    if( mVisible )
    {
      Dali::GlWindow handle( this );
      mVisibilityChangedSignal.Emit( handle, true );
    }

    if( mEventHandler )
    {
      mEventHandler->Resume();
    }

    DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Deiconified: visible = %d\n", this, mNativeWindowId, mVisible );
  }
}

void GlWindow::OnFocusChanged( bool focusIn )
{
  Dali::GlWindow handle( this );
  mFocusChangeSignal.Emit( handle, focusIn );
}

void GlWindow::OnOutputTransformed()
{
  int screenRotationAngle = mWindowBase->GetScreenRotationAngle();
  if( screenRotationAngle != mScreenRotationAngle )
  {
    mScreenRotationAngle = screenRotationAngle;
    mTotalRotationAngle = (mWindowRotationAngle + mScreenRotationAngle) % 360;

    if( mTotalRotationAngle == 90 || mTotalRotationAngle == 270 )
    {
      mWindowWidth = mPositionSize.height;
      mWindowHeight = mPositionSize.width;
    }
    else
    {
      mWindowWidth = mPositionSize.width;
      mWindowHeight = mPositionSize.height;
    }

    // Emit Resize signal
    Dali::GlWindow handle( this );
    mResizeSignal.Emit( Dali::Uint16Pair( mWindowWidth, mWindowHeight ) );
  }
}

void GlWindow::OnTouchPoint( Dali::Integration::Point& point, int timeStamp )
{
  PointState::Type state = point.GetState();

  if( state == PointState::DOWN )
  {
    mIsTouched = true;
  }

  if( state == PointState::UP )
  {
    mIsTouched = false;
  }

  if( !mIsTouched && state == PointState::MOTION )
  {
    return;
  }

  RecalculateTouchPosition( point );
  Dali::TouchEvent touchEvent = Dali::Integration::NewTouchEvent( timeStamp, point );
  Dali::GlWindow handle( this );
  mTouchedSignal.Emit( touchEvent );
}

void GlWindow::OnWheelEvent( Dali::Integration::WheelEvent& wheelEvent )
{
  // TODO:
  //FeedWheelEvent( wheelEvent );
}

void GlWindow::OnKeyEvent( Dali::Integration::KeyEvent& keyEvent )
{
  Dali::KeyEvent event = Dali::DevelKeyEvent::New( keyEvent.keyName, keyEvent.logicalKey, keyEvent.keyString, keyEvent.keyCode,
                                                   keyEvent.keyModifier, keyEvent.time, static_cast<Dali::KeyEvent::State>(keyEvent.state),
                                                   keyEvent.compose, keyEvent.deviceName, keyEvent.deviceClass, keyEvent.deviceSubclass );
  Dali::GlWindow handle( this );
  mKeyEventSignal.Emit( event );
}

void GlWindow::OnRotation( const RotationEvent& rotation )
{
  mWindowRotationAngle = rotation.angle;
  mTotalRotationAngle = ( mWindowRotationAngle + mScreenRotationAngle ) % 360;
  if( mTotalRotationAngle == 90 || mTotalRotationAngle == 270 )
  {
    mWindowWidth = mPositionSize.height;
    mWindowHeight = mPositionSize.width;
  }
  else
  {
    mWindowWidth = mPositionSize.width;
    mWindowHeight = mPositionSize.height;
  }

  mIsRotated = true;
  mIsWindowRotated = true;
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), OnRotation(): resize signal emit [%d x %d]\n", this, mNativeWindowId, mWindowWidth, mWindowHeight );

  // Emit Resize signal
  Dali::GlWindow handle( this );
  mResizeSignal.Emit( Dali::Uint16Pair( mWindowWidth, mWindowHeight ) );
}

void GlWindow::RecalculateTouchPosition( Integration::Point& point )
{
  Vector2 position = point.GetScreenPosition();
  Vector2 convertedPosition;

  switch( mTotalRotationAngle )
  {
    case 90:
    {
      convertedPosition.x = static_cast<float>( mWindowWidth ) - position.y;
      convertedPosition.y = position.x;
      break;
    }
    case 180:
    {
      convertedPosition.x = static_cast<float>( mWindowWidth ) - position.x;
      convertedPosition.y = static_cast<float>( mWindowHeight ) - position.y;
      break;
    }
    case 270:
    {
      convertedPosition.x = position.y;
      convertedPosition.y = static_cast<float>( mWindowHeight ) - position.x;
      break;
    }
    default:
    {
      convertedPosition = position;
      break;
    }
  }

  point.SetScreenPosition( convertedPosition );
}

void GlWindow::SetAvailableAnlges( const std::vector< int >& angles )
{
  if( angles.size() > 4 )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetAvailableAnlges: Invalid vector size! [%d]\n", angles.size() );
    return;
  }

  mWindowBase->SetAvailableAnlges( angles );
}

bool GlWindow::IsOrientationAvailable( WindowOrientation orientation ) const
{
  if( orientation <= WindowOrientation::NO_ORIENTATION_PREFERENCE
      || orientation > WindowOrientation::LANDSCAPE_INVERSE )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::IsOrientationAvailable: Invalid input orientation [%d]\n", orientation );
    return false;
  }
  return true;
}

int GlWindow::ConvertToAngle(  WindowOrientation  orientation )
{
  int convertAngle = 0;
  if ( mOrientationMode == 0 )
  {
    convertAngle = static_cast< int >( orientation );
  }
  else if( mOrientationMode == 1)
  {
    switch( orientation )
    {
      case WindowOrientation::LANDSCAPE:
      {
        convertAngle = 0;
        break;
      }
      case WindowOrientation::PORTRAIT:
      {
        convertAngle = 90;
        break;
      }
      case WindowOrientation::LANDSCAPE_INVERSE:
      {
        convertAngle = 180;
        break;
      }
      case WindowOrientation::PORTRAIT_INVERSE:
      {
        convertAngle = 270;
        break;
      }
      case WindowOrientation::NO_ORIENTATION_PREFERENCE:
      {
        convertAngle = -1;
        break;
      }
    }
  }
  return convertAngle;
}

WindowOrientation GlWindow::ConvertToOrientation( int angle ) const
{
  WindowOrientation orientation = WindowOrientation::NO_ORIENTATION_PREFERENCE;
  if ( mOrientationMode == 0 ) // Portrate mode
  {
    orientation = static_cast< WindowOrientation >( angle );
  }
  else if( mOrientationMode == 1 ) // Landscape mode
  {
    switch( angle )
    {
      case 0:
      {
        orientation = WindowOrientation::LANDSCAPE;
        break;
      }
      case 90:
      {
        orientation = WindowOrientation::PORTRAIT;
        break;
      }
      case 180:
      {
        orientation = WindowOrientation::LANDSCAPE_INVERSE;
        break;
      }
      case 270:
      {
        orientation = WindowOrientation::PORTRAIT_INVERSE;
        break;
      }
      case -1:
      {
        orientation = WindowOrientation::NO_ORIENTATION_PREFERENCE;
        break;
      }
    }
  }
  return orientation;
}

WindowOrientation GlWindow::GetCurrentOrientation() const
{
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), GetCurrentOrientation(): %d\n", this, mNativeWindowId, mTotalRotationAngle );
  return ConvertToOrientation( mTotalRotationAngle );
}

void GlWindow::SetAvailableOrientations( const Dali::Vector< WindowOrientation >& orientations )
{
  Dali::Vector<float>::SizeType count = orientations.Count();
  for( Dali::Vector<float>::SizeType index = 0; index < count; ++index )
  {
    if( IsOrientationAvailable( orientations[index] ) == false )
    {
      DALI_LOG_ERROR("Window::SetAvailableRotationAngles, invalid angle: %d\n", orientations[index]);
      continue;
    }

    bool found = false;
    int angle = ConvertToAngle( orientations[index] );

    for( std::size_t i = 0; i < mAvailableAngles.size(); i++ )
    {
      if( mAvailableAngles[i] == angle )
      {
        found = true;
        break;
      }
    }

    if( !found )
    {
      DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), SetAvailableOrientations: %d\n", this, mNativeWindowId, angle );
      mAvailableAngles.push_back( angle );
    }
  }
  SetAvailableAnlges( mAvailableAngles );
}

void GlWindow::SetPreferredOrientation( WindowOrientation orientation  )
{
  if( IsOrientationAvailable( orientation ) == false )
  {
    DALI_LOG_ERROR( "Window::SetPreferredOrientation, invalid orientation: %d\n", orientation );
    return;
  }
  mPreferredAngle = ConvertToAngle( orientation );
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), SetPreferredOrientation: %d\n", this, mNativeWindowId, mPreferredAngle );
  mWindowBase->SetPreferredAngle( mPreferredAngle );
}

void GlWindow::SetChild( Dali::Window& child )
{
  if( DALI_UNLIKELY( child ) )
  {
    mChildWindow = child;
    Internal::Adaptor::Window& windowImpl = Dali::GetImplementation( mChildWindow );
    WindowRenderSurface* renderSurface = static_cast<WindowRenderSurface*>( windowImpl.GetSurface() );
    if( renderSurface )
    {
      WindowBase* childWindowBase = renderSurface->GetWindowBase();
      if( childWindowBase )
      {
        childWindowBase->SetParent( mWindowBase.get() );
      }
    }
  }
}

void GlWindow::RegisterGlCallback( CallbackBase* initCallback, CallbackBase* renderFrameCallback, CallbackBase* terminateCallback )
{
  if( mIsEGLInitialize == false )
  {
    InitializeGraphics();
  }
  mGLInitCallback = std::unique_ptr< CallbackBase >(initCallback);
  mGLRenderFrameCallback = std::unique_ptr< CallbackBase >( renderFrameCallback );
  mGLTerminateCallback = std::unique_ptr< CallbackBase >( terminateCallback );

  mInitCallback = false;

  if( !mGLRenderCallback )
  {
    mGLRenderCallback = MakeCallback( this, &GlWindow::RunCallback );

    if( Dali::Adaptor::IsAvailable() )
    {
      Dali::Adaptor::Get().AddIdle( mGLRenderCallback, true );
    }
    else
    {
      DALI_LOG_RELEASE_INFO( "RegisterGlCallback: Adaptor is not avaiable\n" );
    }

  }
}

bool GlWindow::RunCallback()
{
  GraphicsInterface* graphics = mGraphics.get();
  EglGraphics *eglGraphics = static_cast<EglGraphics*>( graphics );
  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  eglImpl.MakeContextCurrent( mEGLSurface, mEGLContext );

  int renderFrameResult = 0;

  if( mIsRotated )
  {
    mWindowBase->SetEglWindowBufferTransform( mTotalRotationAngle );
    if( mIsWindowRotated )
    {
      mWindowBase->SetEglWindowTransform( mWindowRotationAngle );
    }
    mIsRotated = false;
  }

  if( !mInitCallback )
  {
    if( mGLInitCallback )
    {
      CallbackBase::Execute(*mGLInitCallback);
    }
    mInitCallback = true;
  }

  if( mGLRenderFrameCallback )
  {
    renderFrameResult = CallbackBase::ExecuteReturn<int>(*mGLRenderFrameCallback);
  }

  if( mIsWindowRotated )
  {
    mWindowBase->WindowRotationCompleted( mWindowRotationAngle, mPositionSize.width, mPositionSize.height );
    mIsWindowRotated = false;
  }

  if(renderFrameResult)
  {
    eglImpl.SwapBuffers( mEGLSurface );
  }

  return true;
}

void GlWindow::RenderOnce()
{
  RunCallback();
}

void GlWindow::InitializeGraphics()
{
  if( !mIsEGLInitialize )
  {
    // Init Graphics
    std::unique_ptr< GraphicsFactory > graphicsFactoryPtr = Utils::MakeUnique< GraphicsFactory >();
    auto graphicsFactory = *graphicsFactoryPtr.get();

    mGraphics = std::unique_ptr< GraphicsInterface >( &graphicsFactory.Create() );
    GraphicsInterface* graphics = mGraphics.get();
    EglGraphics *eglGraphics = static_cast<EglGraphics*>( graphics );
    eglGraphics->Initialize( mDepth, mStencil, mMSAA );
    eglGraphics->Create();

    mDisplayConnection = std::unique_ptr< Dali::DisplayConnection >( Dali::DisplayConnection::New( *graphics, Dali::RenderSurfaceInterface::Type::WINDOW_RENDER_SURFACE ) );
    mDisplayConnection->Initialize();

    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
    if( mGLESVersion == Dali::GlWindow::GlesVersion::VERSION_2_0 )
    {
      eglImpl.SetGlesVersion( 20 );
    }
    else if( mGLESVersion == Dali::GlWindow::GlesVersion::VERSION_3_0 )
    {
      eglImpl.SetGlesVersion( 30 );
    }

    if( eglImpl.ChooseConfig(true, mColorDepth) == false )
    {
      if( mGLESVersion == Dali::GlWindow::GlesVersion::VERSION_3_0 )
      {
        DALI_LOG_RELEASE_INFO( "InitializeGraphics: Fail to choose config with GLES30, retry with GLES20\n" );
        eglImpl.SetGlesVersion( 20 );
        mGLESVersion = Dali::GlWindow::GlesVersion::VERSION_2_0;
        if( eglImpl.ChooseConfig(true, mColorDepth) == false )
        {
          DALI_LOG_ERROR("InitializeGraphics: Fail to choose config with GLES20");
          return;
        }
      }
      else
      {
        DALI_LOG_ERROR("InitializeGraphics: Fail to choose config with GLES20");
        return;
      }
    }
    eglImpl.CreateWindowContext( mEGLContext );

   // Create the EGL window
    EGLNativeWindowType window = mWindowBase->CreateEglWindow( mPositionSize.width, mPositionSize.height );
    mEGLSurface = eglImpl.CreateSurfaceWindow( window, mColorDepth );

    mIsEGLInitialize = true;
  }
}

void GlWindow::OnDamaged(  const DamageArea& area )
{

}

} // Adaptor

} // Internal

} // Dali
