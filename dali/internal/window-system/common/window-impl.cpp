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
#include <dali/internal/window-system/common/window-impl.h>

// EXTERNAL HEADERS
#include <dali/integration-api/core.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/actors/camera-actor.h>
#include <dali/public-api/render-tasks/render-task.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <dali/public-api/rendering/frame-buffer.h>
#include <dali/devel-api/adaptor-framework/orientation.h>
#include <dali/integration-api/events/touch-event-integ.h>

#ifdef DALI_ADAPTOR_COMPILATION
#include <dali/integration-api/render-surface-interface.h>
#else
#include <dali/integration-api/adaptors/render-surface-interface.h>
#endif

// INTERNAL HEADERS
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/window-system/common/orientation-impl.h>
#include <dali/internal/window-system/common/render-surface-factory.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-visibility-observer.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_WINDOW" );
#endif

} // unnamed namespace

Window* Window::New( const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent )
{
  Window* window = new Window();
  window->mIsTransparent = isTransparent;
  window->Initialize( positionSize, name, className );
  return window;
}

Window::Window()
: mWindowSurface( nullptr ),
  mWindowBase(),
  mIsTransparent( false ),
  mIsFocusAcceptable( true ),
  mIconified( false ),
  mOpaqueState( false ),
  mResizeEnabled( false ),
  mType( Dali::Window::NORMAL ),
  mParentWindow( NULL ),
  mPreferredOrientation( Dali::Window::PORTRAIT ),
  mRotationAngle( 0 ),
  mWindowWidth( 0 ),
  mWindowHeight( 0 ),
  mFocusChangedSignal(),
  mResizedSignal(),
  mDeleteRequestSignal()
{
}

Window::~Window()
{
  if ( mEventHandler )
  {
    mEventHandler->RemoveObserver( *this );
  }
}

void Window::Initialize(const PositionSize& positionSize, const std::string& name, const std::string& className)
{
  // Create a window render surface
  Any surface;
  auto renderSurfaceFactory = Dali::Internal::Adaptor::GetRenderSurfaceFactory();
  mSurface = renderSurfaceFactory->CreateWindowRenderSurface( positionSize, surface, mIsTransparent );
  mWindowSurface = static_cast<WindowRenderSurface*>( mSurface.get() );

  // Get a window base
  mWindowBase = mWindowSurface->GetWindowBase();

  // Connect signals
  mWindowBase->IconifyChangedSignal().Connect( this, &Window::OnIconifyChanged );
  mWindowBase->FocusChangedSignal().Connect( this, &Window::OnFocusChanged );
  mWindowBase->DeleteRequestSignal().Connect( this, &Window::OnDeleteRequest );

  mWindowSurface->OutputTransformedSignal().Connect( this, &Window::OnOutputTransformed );

  if( !positionSize.IsEmpty() )
  {
    AddAuxiliaryHint( "wm.policy.win.user.geometry", "1" );
    mResizeEnabled = true;
  }

  SetClass( name, className );

  mWindowSurface->Map();

  mOrientation = Orientation::New( this );
}

void Window::OnAdaptorSet(Dali::Adaptor& adaptor)
{
  mEventHandler = EventHandlerPtr(new EventHandler( mWindowSurface, *mAdaptor ) );
  mEventHandler->AddObserver( *this );
}

void Window::OnSurfaceSet( Dali::RenderSurfaceInterface* surface )
{
  mWindowSurface = static_cast<WindowRenderSurface*>( surface );
}

void Window::ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode )
{
}

void Window::SetIndicatorBgOpacity( Dali::Window::IndicatorBgOpacity opacityMode )
{
}

void Window::RotateIndicator( Dali::Window::WindowOrientation orientation )
{
}

void Window::SetClass( std::string name, std::string className )
{
  mName = name;
  mClassName = className;
  mWindowBase->SetClass( name, className );
}

std::string Window::GetClassName() const
{
  return mClassName;
}

void Window::Raise()
{
  mWindowBase->Raise();
  DALI_LOG_RELEASE_INFO( "Window (%p) Raise() \n", this );
}

void Window::Lower()
{
  mWindowBase->Lower();
  DALI_LOG_RELEASE_INFO( "Window (%p) Lower() \n", this );
}

void Window::Activate()
{
  mWindowBase->Activate();
  DALI_LOG_RELEASE_INFO( "Window (%p) Activate() \n", this );
}

uint32_t Window::GetLayerCount() const
{
  return mScene.GetLayerCount();
}

Dali::Layer Window::GetLayer( uint32_t depth ) const
{
  return mScene.GetLayer( depth );
}

Dali::RenderTaskList Window::GetRenderTaskList() const
{
  return mScene.GetRenderTaskList();
}

void Window::AddAvailableOrientation( Dali::Window::WindowOrientation orientation )
{
  bool found = false;

  if( orientation <= Dali::Window::LANDSCAPE_INVERSE )
  {
    for( std::size_t i = 0; i < mAvailableOrientations.size(); i++ )
    {
      if( mAvailableOrientations[i] == orientation )
      {
        found = true;
        break;
      }
    }

    if( !found )
    {
      mAvailableOrientations.push_back( orientation );
      SetAvailableOrientations( mAvailableOrientations );
    }
  }
}

void Window::RemoveAvailableOrientation( Dali::Window::WindowOrientation orientation )
{
  for( std::vector<Dali::Window::WindowOrientation>::iterator iter = mAvailableOrientations.begin();
       iter != mAvailableOrientations.end(); ++iter )
  {
    if( *iter == orientation )
    {
      mAvailableOrientations.erase( iter );
      break;
    }
  }
  SetAvailableOrientations( mAvailableOrientations );
}

void Window::SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations )
{
  if( orientations.size() > 4 )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetAvailableOrientations: Invalid vector size! [%d]\n", orientations.size() );
    return;
  }

  mAvailableOrientations = orientations;

  mWindowBase->SetAvailableOrientations( mAvailableOrientations );
}

const std::vector< Dali::Window::WindowOrientation >& Window::GetAvailableOrientations()
{
  return mAvailableOrientations;
}

void Window::SetPreferredOrientation( Dali::Window::WindowOrientation orientation )
{
  mPreferredOrientation = orientation;

  mWindowBase->SetPreferredOrientation( mPreferredOrientation );
}

Dali::Window::WindowOrientation Window::GetPreferredOrientation()
{
  return mPreferredOrientation;
}

Dali::Any Window::GetNativeHandle() const
{
  return mWindowSurface->GetNativeWindow();
}

void Window::SetAcceptFocus( bool accept )
{
  mIsFocusAcceptable = accept;

  mWindowBase->SetAcceptFocus( accept );
}

bool Window::IsFocusAcceptable() const
{
  return mIsFocusAcceptable;
}

void Window::Show()
{
  mVisible = true;

  mWindowBase->Show();

  if( !mIconified )
  {
    WindowVisibilityObserver* observer( mAdaptor );
    observer->OnWindowShown();
  }

  DALI_LOG_RELEASE_INFO( "Window (%p) Show(): iconified = %d\n", this, mIconified );
}

void Window::Hide()
{
  mVisible = false;

  mWindowBase->Hide();

  if( !mIconified )
  {
    WindowVisibilityObserver* observer( mAdaptor );
    observer->OnWindowHidden();
  }

  DALI_LOG_RELEASE_INFO( "Window (%p) Hide(): iconified = %d\n", this, mIconified );
}

bool Window::IsVisible() const
{
  return mVisible && !mIconified;
}

unsigned int Window::GetSupportedAuxiliaryHintCount() const
{
  return mWindowBase->GetSupportedAuxiliaryHintCount();
}

std::string Window::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  return mWindowBase->GetSupportedAuxiliaryHint( index );
}

unsigned int Window::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return mWindowBase->AddAuxiliaryHint( hint, value );
}

bool Window::RemoveAuxiliaryHint( unsigned int id )
{
  return mWindowBase->RemoveAuxiliaryHint( id );
}

bool Window::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return mWindowBase->SetAuxiliaryHintValue( id, value );
}

std::string Window::GetAuxiliaryHintValue( unsigned int id ) const
{
  return mWindowBase->GetAuxiliaryHintValue( id );
}

unsigned int Window::GetAuxiliaryHintId( const std::string& hint ) const
{
  return mWindowBase->GetAuxiliaryHintId( hint );
}

void Window::SetInputRegion( const Rect< int >& inputRegion )
{
  mWindowBase->SetInputRegion( inputRegion );

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetInputRegion: x = %d, y = %d, w = %d, h = %d\n", inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height );
}

void Window::SetType( Dali::Window::Type type )
{
  if( type != mType )
  {
    mWindowBase->SetType( type );

    mType = type;
  }
}

Dali::Window::Type Window::GetType() const
{
  return mType;
}

bool Window::SetNotificationLevel( Dali::Window::NotificationLevel::Type level )
{
  if( mType != Dali::Window::NOTIFICATION )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetNotificationLevel: Not supported window type [%d]\n", mType );
    return false;
  }

  return mWindowBase->SetNotificationLevel( level );
}

Dali::Window::NotificationLevel::Type Window::GetNotificationLevel() const
{
  if( mType != Dali::Window::NOTIFICATION )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetNotificationLevel: Not supported window type [%d]\n", mType );
    return Dali::Window::NotificationLevel::NONE;
  }

  return mWindowBase->GetNotificationLevel();
}

void Window::SetOpaqueState( bool opaque )
{
  mOpaqueState = opaque;

  mWindowBase->SetOpaqueState( opaque );

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetOpaqueState: opaque = %d\n", opaque );
}

bool Window::IsOpaqueState() const
{
  return mOpaqueState;
}

bool Window::SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode)
{
  return mWindowBase->SetScreenOffMode( screenOffMode );
}

Dali::Window::ScreenOffMode::Type Window::GetScreenOffMode() const
{
  return mWindowBase->GetScreenOffMode();
}

bool Window::SetBrightness( int brightness )
{
  if( brightness < 0 || brightness > 100 )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetBrightness: Invalid brightness value [%d]\n", brightness );
    return false;
  }

  return mWindowBase->SetBrightness( brightness );
}

int Window::GetBrightness() const
{
  return mWindowBase->GetBrightness();
}

void Window::SetSize( Dali::Window::WindowSize size )
{
  if( !mResizeEnabled )
  {
    AddAuxiliaryHint( "wm.policy.win.user.geometry", "1" );
    mResizeEnabled = true;
  }

  PositionSize oldRect = mSurface->GetPositionSize();

  mWindowSurface->MoveResize( PositionSize( oldRect.x, oldRect.y, size.GetWidth(), size.GetHeight() ) );

  PositionSize newRect = mSurface->GetPositionSize();

  // When surface size is updated, inform adaptor of resizing and emit ResizeSignal
  if( ( oldRect.width != newRect.width ) || ( oldRect.height != newRect.height ) )
  {
    Uint16Pair newSize( newRect.width, newRect.height );

    SurfaceResized();

    mAdaptor->SurfaceResizePrepare( mSurface.get(), newSize );

    mResizedSignal.Emit( newSize );

    mAdaptor->SurfaceResizeComplete( mSurface.get(), newSize );
  }
}

Dali::Window::WindowSize Window::GetSize() const
{
  PositionSize positionSize = mSurface->GetPositionSize();

  return Dali::Window::WindowSize( positionSize.width, positionSize.height );
}

void Window::SetPosition( Dali::Window::WindowPosition position )
{
  if( !mResizeEnabled )
  {
    AddAuxiliaryHint( "wm.policy.win.user.geometry", "1" );
    mResizeEnabled = true;
  }

  PositionSize oldRect = mSurface->GetPositionSize();

  mWindowSurface->MoveResize( PositionSize( position.GetX(), position.GetY(), oldRect.width, oldRect.height ) );
}

Dali::Window::WindowPosition Window::GetPosition() const
{
  PositionSize positionSize = mSurface->GetPositionSize();

  return Dali::Window::WindowPosition( positionSize.x, positionSize.y );
}

void Window::SetPositionSize( PositionSize positionSize )
{
  if( !mResizeEnabled )
  {
    AddAuxiliaryHint( "wm.policy.win.user.geometry", "1" );
    mResizeEnabled = true;
  }

  PositionSize oldRect = mSurface->GetPositionSize();

  mWindowSurface->MoveResize( positionSize );

  PositionSize newRect = mSurface->GetPositionSize();

  // When surface size is updated, inform adaptor of resizing and emit ResizeSignal
  if( ( oldRect.width != newRect.width ) || ( oldRect.height != newRect.height ) )
  {
    Uint16Pair newSize( newRect.width, newRect.height );

    SurfaceResized();

    mAdaptor->SurfaceResizePrepare( mSurface.get(), newSize );

    mResizedSignal.Emit( newSize );

    mAdaptor->SurfaceResizeComplete( mSurface.get(), newSize );
  }
}

Dali::Layer Window::GetRootLayer() const
{
  return mScene.GetRootLayer();
}

void Window::SetTransparency( bool transparent )
{
  mWindowSurface->SetTransparency( transparent );
}

bool Window::GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode )
{
  return mWindowBase->GrabKey( key, grabMode );
}

bool Window::UngrabKey( Dali::KEY key )
{
  return mWindowBase->UngrabKey( key );
}

bool Window::GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result )
{
  return mWindowBase->GrabKeyList( key, grabMode, result );
}

bool Window::UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result )
{
  return mWindowBase->UngrabKeyList( key, result );
}

void Window::OnIconifyChanged( bool iconified )
{
  if( iconified )
  {
    mIconified = true;

    if( mVisible )
    {
      WindowVisibilityObserver* observer( mAdaptor );
      observer->OnWindowHidden();
    }

    DALI_LOG_RELEASE_INFO( "Window (%p) Iconified: visible = %d\n", this, mVisible );
  }
  else
  {
    mIconified = false;

    if( mVisible )
    {
      WindowVisibilityObserver* observer( mAdaptor );
      observer->OnWindowShown();
    }

    DALI_LOG_RELEASE_INFO( "Window (%p) Deiconified: visible = %d\n", this, mVisible );
  }
}

void Window::OnFocusChanged( bool focusIn )
{
  mFocusChangedSignal.Emit( focusIn );
}

void Window::OnOutputTransformed()
{
  PositionSize positionSize = mSurface->GetPositionSize();
  SurfaceResized();
  mAdaptor->SurfaceResizePrepare( mSurface.get(), Adaptor::SurfaceSize( positionSize.width, positionSize.height ) );
  mAdaptor->SurfaceResizeComplete( mSurface.get(), Adaptor::SurfaceSize( positionSize.width, positionSize.height ) );
}

void Window::OnDeleteRequest()
{
  mDeleteRequestSignal.Emit();
}

void Window::OnTouchPoint( Dali::Integration::Point& point, int timeStamp )
{
  FeedTouchPoint( point, timeStamp );
}

void Window::OnWheelEvent( Dali::Integration::WheelEvent& wheelEvent )
{
  FeedWheelEvent( wheelEvent );
}

void Window::OnKeyEvent( Dali::Integration::KeyEvent& keyEvent )
{
  FeedKeyEvent( keyEvent );
}

void Window::OnRotation( const RotationEvent& rotation )
{
  mRotationAngle = rotation.angle;
  mWindowWidth = rotation.width;
  mWindowHeight = rotation.height;

  // Notify that the orientation is changed
  mOrientation->OnOrientationChange( rotation );

  mWindowSurface->RequestRotation( mRotationAngle, mWindowWidth, mWindowHeight );

  SurfaceResized();

  mAdaptor->SurfaceResizePrepare( mSurface.get(), Adaptor::SurfaceSize( mRotationAngle, mWindowHeight ) );

  // Emit signal
  mResizedSignal.Emit( Dali::Window::WindowSize( mRotationAngle, mWindowHeight ) );

  mAdaptor->SurfaceResizeComplete( mSurface.get(), Adaptor::SurfaceSize( mRotationAngle, mWindowHeight ) );
}

void Window::OnPause()
{
  if( mEventHandler )
  {
    mEventHandler->Pause();
  }
}

void Window::OnResume()
{
  if( mEventHandler )
  {
    mEventHandler->Resume();
  }
}

void Window::RecalculateTouchPosition( Integration::Point& point )
{
  Vector2 position = point.GetScreenPosition();
  Vector2 convertedPosition;

  switch( mRotationAngle )
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

Dali::Window Window::Get( Dali::Actor actor )
{
  Internal::Adaptor::Window* windowImpl = nullptr;

  if ( Internal::Adaptor::Adaptor::IsAvailable() )
  {
    Dali::Internal::Adaptor::Adaptor& adaptor = Internal::Adaptor::Adaptor::GetImplementation( Internal::Adaptor::Adaptor::Get() );
    windowImpl = static_cast<Internal::Adaptor::Window*>( adaptor.GetWindow( actor ) );
  }

  return Dali::Window( windowImpl );
}


void Window::SetParent( Dali::Window& parent )
{
  if ( DALI_UNLIKELY( parent ) )
  {
    mParentWindow = parent;
    Dali::Window grandParent = Dali::DevelWindow::GetParent( parent );
    // check circular parent window setting
    if ( DALI_UNLIKELY( grandParent ) && mWindowBase->IsMatchedWindow( grandParent.GetNativeHandle() ) )
    {
      Dali::DevelWindow::Unparent( parent );
    }
    mWindowBase->SetParent( parent.GetNativeHandle() );
  }
}

void Window::Unparent()
{
  Any parent;
  mWindowBase->SetParent( parent );
}

Dali::Window Window::GetParent()
{
  return mParentWindow;
}

} // Adaptor

} // Internal

} // Dali
