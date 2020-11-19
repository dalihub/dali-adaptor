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
#include <dali/internal/window-system/common/window-impl.h>

// EXTERNAL HEADERS
#include <thread>
#include <dali/integration-api/core.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/actors/camera-actor.h>
#include <dali/public-api/render-tasks/render-task.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <dali/public-api/rendering/frame-buffer.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/devel-api/adaptor-framework/orientation.h>
#include <dali/integration-api/events/touch-event-integ.h>

// INTERNAL HEADERS
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/window-system/common/orientation-impl.h>
#include <dali/internal/window-system/common/render-surface-factory.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-visibility-observer.h>
#include <dali/devel-api/adaptor-framework/accessibility-impl.h>

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

Window* Window::New(const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent)
{
  Any surface;
  return Window::New(surface, positionSize, name, className, isTransparent);
}

Window* Window::New(Any surface, const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent)
{
  Window* window = new Window();
  window->mIsTransparent = isTransparent;
  window->Initialize(surface, positionSize, name, className);
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
  mType( WindowType::NORMAL ),
  mParentWindow( NULL ),
  mPreferredAngle( static_cast< int >( WindowOrientation::NO_ORIENTATION_PREFERENCE ) ),
  mRotationAngle( -1 ),
  mWindowWidth( 0 ),
  mWindowHeight( 0 ),
  mOrientationMode( Internal::Adaptor::Window::OrientationMode::PORTRAIT ),
  mNativeWindowId( -1 ),
  mDeleteRequestSignal(),
  mFocusChangeSignal(),
  mResizeSignal(),
  mVisibilityChangedSignal(),
  mTransitionEffectEventSignal(),
  mKeyboardRepeatSettingsChangedSignal()
{
}

Window::~Window()
{
  if ( mAdaptor )
  {
    auto bridge = Accessibility::Bridge::GetCurrentBridge();
    auto accessible2 = mScene.GetRootLayer();
    auto accessible = Accessibility::Accessible::Get( accessible2 );
    bridge->RemoveTopLevelWindow( accessible );

    mAdaptor->RemoveWindow( this );
  }

  if ( mEventHandler )
  {
    mEventHandler->RemoveObserver( *this );
  }
}

void Window::Initialize(Any surface, const PositionSize& positionSize, const std::string& name, const std::string& className)
{
  // Create a window render surface
  auto renderSurfaceFactory = Dali::Internal::Adaptor::GetRenderSurfaceFactory();
  mSurface = renderSurfaceFactory->CreateWindowRenderSurface( positionSize, surface, mIsTransparent );
  mWindowSurface = static_cast<WindowRenderSurface*>( mSurface.get() );

  // Get a window base
  mWindowBase = mWindowSurface->GetWindowBase();

  // Connect signals
  mWindowBase->IconifyChangedSignal().Connect( this, &Window::OnIconifyChanged );
  mWindowBase->FocusChangedSignal().Connect( this, &Window::OnFocusChanged );
  mWindowBase->DeleteRequestSignal().Connect( this, &Window::OnDeleteRequest );
  mWindowBase->TransitionEffectEventSignal().Connect( this, &Window::OnTransitionEffectEvent );
  mWindowBase->KeyboardRepeatSettingsChangedSignal().Connect( this, &Window::OnKeyboardRepeatSettingsChanged );
  mWindowBase->WindowRedrawRequestSignal().Connect( this, &Window::OnWindowRedrawRequest );

  mWindowSurface->OutputTransformedSignal().Connect( this, &Window::OnOutputTransformed );

  if( !positionSize.IsEmpty() )
  {
    AddAuxiliaryHint( "wm.policy.win.user.geometry", "1" );
    mResizeEnabled = true;
  }

  SetClass( name, className );

  mWindowSurface->Map();

  mOrientation = Orientation::New( this );

  // Get OrientationMode
  int screenWidth, screenHeight;
  WindowSystem::GetScreenSize( screenWidth, screenHeight );
  if( screenWidth > screenHeight )
  {
    mOrientationMode = Internal::Adaptor::Window::OrientationMode::LANDSCAPE;
  }
  else
  {
    mOrientationMode = Internal::Adaptor::Window::OrientationMode::PORTRAIT;
  }
  // For Debugging
  mNativeWindowId = mWindowBase->GetNativeWindowId();
}

void Window::OnAdaptorSet(Dali::Adaptor& adaptor)
{
  mEventHandler = EventHandlerPtr(new EventHandler( mWindowSurface->GetWindowBase(), *mAdaptor ) );
  mEventHandler->AddObserver( *this );

  auto bridge = Accessibility::Bridge::GetCurrentBridge();
  auto v = mScene.GetRootLayer();
  auto accessible = Accessibility::Accessible::Get( v, true );
  bridge->AddTopLevelWindow( accessible );

  //FIXME: line below is temporary solution for missing "activate" signal and should be removed
  Show();
}

void Window::OnSurfaceSet( Dali::RenderSurfaceInterface* surface )
{
  mWindowSurface = static_cast<WindowRenderSurface*>( surface );
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

  mSurface->SetFullSwapNextFrame();

  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Raise() \n", this, mNativeWindowId );
}

void Window::Lower()
{
  mWindowBase->Lower();

  mSurface->SetFullSwapNextFrame();

  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Lower() \n", this, mNativeWindowId );
}

void Window::Activate()
{
  mWindowBase->Activate();

  mSurface->SetFullSwapNextFrame();

  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Activate() \n", this, mNativeWindowId );
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

void Window::AddAvailableOrientation( WindowOrientation orientation )
{
  if( IsOrientationAvailable( orientation ) == false )
  {
    return;
  }

  bool found = false;
  int convertedAngle = ConvertToAngle( orientation );
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), AddAvailableOrientation: %d\n", this, mNativeWindowId, convertedAngle );
  for( std::size_t i = 0; i < mAvailableAngles.size(); i++ )
  {
    if( mAvailableAngles[i] == convertedAngle )
    {
      found = true;
      break;
    }
  }

  if( !found )
  {
    mAvailableAngles.push_back( convertedAngle );
    SetAvailableAnlges( mAvailableAngles );
  }
}

void Window::RemoveAvailableOrientation( WindowOrientation orientation )
{
  if( IsOrientationAvailable( orientation ) == false )
  {
    return;
  }

  int convertedAngle = ConvertToAngle( orientation );
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), RemoveAvailableOrientation: %d\n", this, mNativeWindowId, convertedAngle );
  for( std::vector< int >::iterator iter = mAvailableAngles.begin();
       iter != mAvailableAngles.end(); ++iter )
  {
    if( *iter == convertedAngle )
    {
      mAvailableAngles.erase( iter );
      break;
    }
  }

  SetAvailableAnlges( mAvailableAngles );
}

void Window::SetPreferredOrientation( WindowOrientation orientation )
{
  if( orientation < WindowOrientation::NO_ORIENTATION_PREFERENCE || orientation > WindowOrientation::LANDSCAPE_INVERSE )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::CheckOrientation: Invalid input orientation [%d]\n", orientation );
    return;
  }
  mPreferredAngle = ConvertToAngle( orientation );
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), SetPreferredOrientation: %d\n", this, mNativeWindowId, mPreferredAngle );
  mWindowBase->SetPreferredAngle( mPreferredAngle );
}

WindowOrientation Window::GetPreferredOrientation()
{
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), GetPreferredOrientation: %d\n", this, mNativeWindowId, mPreferredAngle );
  WindowOrientation preferredOrientation = ConvertToOrientation( mPreferredAngle );
  return preferredOrientation;
}

void Window::SetAvailableAnlges( const std::vector< int >& angles )
{
  if( angles.size() > 4 )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetAvailableAnlges: Invalid vector size! [%d]\n", angles.size() );
    return;
  }

  mWindowBase->SetAvailableAnlges( angles );
}

int Window::ConvertToAngle( WindowOrientation orientation )
{
  int convertAngle = static_cast< int >( orientation );
  if( mOrientationMode == Internal::Adaptor::Window::OrientationMode::LANDSCAPE )
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

WindowOrientation Window::ConvertToOrientation( int angle ) const
{
  WindowOrientation orientation = static_cast< WindowOrientation >( angle );
  if( mOrientationMode == Internal::Adaptor::Window::OrientationMode::LANDSCAPE )
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

bool Window::IsOrientationAvailable( WindowOrientation orientation ) const
{
  if( orientation <= WindowOrientation::NO_ORIENTATION_PREFERENCE || orientation > WindowOrientation::LANDSCAPE_INVERSE )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::IsOrientationAvailable: Invalid input orientation [%d]\n", orientation );
    return false;
  }
  return true;
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

    Dali::Window handle( this );
    mVisibilityChangedSignal.Emit( handle, true );
  }

  mSurface->SetFullSwapNextFrame();

  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Show(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible );
}

void Window::Hide()
{
  mVisible = false;

  mWindowBase->Hide();

  if( !mIconified )
  {
    WindowVisibilityObserver* observer( mAdaptor );
    observer->OnWindowHidden();

    Dali::Window handle( this );
    mVisibilityChangedSignal.Emit( handle, false );
  }

  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Hide(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible );
}

bool Window::IsVisible() const
{
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), IsVisible(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible );
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

void Window::SetType( WindowType type )
{
  if( type != mType )
  {
    mWindowBase->SetType( type );

    mType = type;
  }
}

WindowType Window::GetType() const
{
  return mType;
}

bool Window::SetNotificationLevel( WindowNotificationLevel level )
{
  if( mType != WindowType::NOTIFICATION )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetNotificationLevel: Not supported window type [%d]\n", mType );
    return false;
  }

  return mWindowBase->SetNotificationLevel( level );
}

WindowNotificationLevel Window::GetNotificationLevel() const
{
  if( mType != WindowType::NOTIFICATION )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetNotificationLevel: Not supported window type [%d]\n", mType );
    return WindowNotificationLevel::NONE;
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

bool Window::SetScreenOffMode(WindowScreenOffMode screenOffMode)
{
  return mWindowBase->SetScreenOffMode( screenOffMode );
}

WindowScreenOffMode Window::GetScreenOffMode() const
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

    DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), SetSize(): resize signal [%d x %d]\n", this, mNativeWindowId, newRect.width, newRect.height );

    Dali::Window handle( this );
    mResizeSignal.Emit( handle, newSize );

    mAdaptor->SurfaceResizeComplete( mSurface.get(), newSize );
  }

  mSurface->SetFullSwapNextFrame();
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

  mSurface->SetFullSwapNextFrame();
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

    DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), SetPositionSize():resize signal [%d x %d]\n", this, mNativeWindowId, newRect.width, newRect.height );
    Dali::Window handle( this );
    mResizeSignal.Emit( handle, newSize );
    mAdaptor->SurfaceResizeComplete( mSurface.get(), newSize );
  }

  mSurface->SetFullSwapNextFrame();
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

      Dali::Window handle( this );
      mVisibilityChangedSignal.Emit( handle, false );
    }

    DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Iconified: visible = %d\n", this, mNativeWindowId, mVisible );
  }
  else
  {
    mIconified = false;

    if( mVisible )
    {
      WindowVisibilityObserver* observer( mAdaptor );
      observer->OnWindowShown();

      Dali::Window handle( this );
      mVisibilityChangedSignal.Emit( handle, true );
    }

    DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), Deiconified: visible = %d\n", this, mNativeWindowId, mVisible );
  }

  mSurface->SetFullSwapNextFrame();
}

void Window::OnFocusChanged( bool focusIn )
{
  Dali::Window handle( this );
  mFocusChangeSignal.Emit( handle, focusIn );

  mSurface->SetFullSwapNextFrame();

  if (auto b = Dali::Accessibility::Bridge::GetCurrentBridge())
  {
    if (focusIn)
    {
      b->ApplicationShown();
    }
    else
    {
      b->ApplicationHidden();
    }
  }
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

void Window::OnTransitionEffectEvent( WindowEffectState state, WindowEffectType type )
{
  Dali::Window handle( this );
  mTransitionEffectEventSignal.Emit( handle, state, type );
}

void Window::OnKeyboardRepeatSettingsChanged()
{
  Dali::Window handle( this );
  mKeyboardRepeatSettingsChangedSignal.Emit();
}

void Window::OnWindowRedrawRequest()
{
  mAdaptor->RenderOnce();
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

  mAdaptor->SurfaceResizePrepare( mSurface.get(), Adaptor::SurfaceSize( mWindowWidth, mWindowHeight ) );

  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), OnRotation(): resize signal emit [%d x %d]\n", this, mNativeWindowId, mWindowWidth, mWindowHeight );
  // Emit signal
  Dali::Window handle( this );
  mResizeSignal.Emit( handle, Dali::Window::WindowSize( mWindowWidth, mWindowHeight ) );

  mAdaptor->SurfaceResizeComplete( mSurface.get(), Adaptor::SurfaceSize( mWindowWidth, mWindowHeight ) );
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

  mSurface->SetFullSwapNextFrame();
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
    windowImpl = dynamic_cast<Internal::Adaptor::Window*>( adaptor.GetWindow( actor ) );
    if( windowImpl )
    {
      return Dali::Window( windowImpl );
    }
  }

  return Dali::Window();
}

void Window::SetParent( Dali::Window& parent )
{
  if ( DALI_UNLIKELY( parent ) )
  {
    mParentWindow = parent;
    Dali::Window self = Dali::Window( this );
    // check circular parent window setting
    if ( Dali::DevelWindow::GetParent( parent ) == self )
    {
      Dali::DevelWindow::Unparent( parent );
    }
    mWindowBase->SetParent( GetImplementation( mParentWindow ).mWindowBase );
  }
}

void Window::Unparent()
{
  mWindowBase->SetParent( nullptr );
  mParentWindow.Reset();
}

Dali::Window Window::GetParent()
{
  return mParentWindow;
}

WindowOrientation Window::GetCurrentOrientation() const
{
  DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), GetCurrentOrientation(): %d\n", this, mNativeWindowId, mRotationAngle );
  return ConvertToOrientation( mRotationAngle );
}

void Window::SetAvailableOrientations( const Dali::Vector<WindowOrientation>& orientations )
{
  Dali::Vector<float>::SizeType count = orientations.Count();
  for( Dali::Vector<float>::SizeType index = 0; index < count; ++index )
  {
    if( IsOrientationAvailable( orientations[index] ) == false )
    {
      DALI_LOG_ERROR("Window::SetAvailableOrientations, invalid orientation: %d\n", orientations[index]);
      continue;
    }

    bool found = false;
    int convertedAngle = ConvertToAngle( orientations[index] );

    for( std::size_t i = 0; i < mAvailableAngles.size(); i++ )
    {
      if( mAvailableAngles[i] == convertedAngle )
      {
        found = true;
        break;
      }
    }

    if( !found )
    {
      DALI_LOG_RELEASE_INFO( "Window (%p), WinId (%d), SetAvailableOrientations: %d\n", this, mNativeWindowId, convertedAngle );
      mAvailableAngles.push_back( convertedAngle );
    }
  }
  SetAvailableAnlges( mAvailableAngles );
}

int32_t Window::GetNativeId() const
{
  return mWindowBase->GetNativeWindowId();
}

} // Adaptor

} // Internal

} // Dali
