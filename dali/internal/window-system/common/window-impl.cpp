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
#include <dali/internal/window-system/common/window-impl.h>

// EXTERNAL HEADERS
#include <dali/integration-api/core.h>
#include <dali/integration-api/system-overlay.h>
#include <dali/public-api/render-tasks/render-task.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <dali/devel-api/adaptor-framework/orientation.h>

// INTERNAL HEADERS
#include <dali/internal/input/common/drag-and-drop-detector-impl.h>
#include <dali/internal/window-system/common/window-visibility-observer.h>
#include <dali/internal/window-system/common/orientation-impl.h>
#include <dali/internal/window-system/common/render-surface-factory.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-render-surface.h>

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
: mSurface( NULL ),
  mWindowBase(),
  mIndicatorVisible( Dali::Window::INVISIBLE ),   // TODO: Enable this after indicator implementation based on tizen 5.
  mIndicatorIsShown( false ),
  mShowRotatedIndicatorOnClose( false ),
  mStarted( false ),
  mIsTransparent( false ),
  mIsFocusAcceptable( true ),
  mVisible( true ),
  mIconified( false ),
  mOpaqueState( false ),
  mResizeEnabled( false ),
  mIndicator(),
  mIndicatorOrientation( Dali::Window::PORTRAIT ),
  mNextIndicatorOrientation( Dali::Window::PORTRAIT ),
  mIndicatorOpacityMode( Dali::Window::OPAQUE ),
  mOverlay( NULL ),
  mAdaptor( NULL ),
  mType( Dali::Window::NORMAL ),
  mPreferredOrientation( Dali::Window::PORTRAIT ),
  mIndicatorVisibilityChangedSignal(),
  mFocusChangedSignal(),
  mResizedSignal(),
  mDeleteRequestSignal()
{
}

Window::~Window()
{
  if( mIndicator )
  {
    mOverlay->Remove( mIndicator->GetActor() );
    Dali::RenderTaskList taskList = mOverlay->GetOverlayRenderTasks();
    Dali::RenderTask indicatorTask = taskList.GetTask(0);
    mOverlay->GetOverlayRenderTasks().RemoveTask(indicatorTask);
    mIndicator->Close();
  }

  if ( mAdaptor )
  {
    mAdaptor->RemoveObserver( *this );
    mAdaptor->SetDragAndDropDetector( NULL );
    mAdaptor = NULL;
  }

  delete mSurface;
}

void Window::Initialize(const PositionSize& positionSize, const std::string& name, const std::string& className)
{
  // Create a window render surface
  Any surface;
  auto renderSurfaceFactory = Dali::Internal::Adaptor::GetRenderSurfaceFactory();
  auto windowRenderSurface = renderSurfaceFactory->CreateWindowRenderSurface( positionSize, surface, mIsTransparent );
  mSurface = windowRenderSurface.release();

  // Get a window base
  mWindowBase = mSurface->GetWindowBase();

  // Connect signals
  mWindowBase->IconifyChangedSignal().Connect( this, &Window::OnIconifyChanged );
  mWindowBase->FocusChangedSignal().Connect( this, &Window::OnFocusChanged );
  mWindowBase->DeleteRequestSignal().Connect( this, &Window::OnDeleteRequest );
  mWindowBase->IndicatorFlickedSignal().Connect( this, &Window::OnIndicatorFlicked );

  mSurface->OutputTransformedSignal().Connect( this, &Window::OnOutputTransformed );

  if( !positionSize.IsEmpty() )
  {
    AddAuxiliaryHint( "wm.policy.win.user.geometry", "1" );
    mResizeEnabled = true;
  }

  SetClass( name, className );

  mSurface->Map();

  mOrientation = Orientation::New( this );
}

void Window::SetAdaptor(Dali::Adaptor& adaptor)
{
  DALI_ASSERT_ALWAYS( !mStarted && "Adaptor already started" );
  mStarted = true;

  // Only create one overlay per window
  Internal::Adaptor::Adaptor& adaptorImpl = Internal::Adaptor::Adaptor::GetImplementation(adaptor);
  Integration::Core& core = adaptorImpl.GetCore();
  mOverlay = &core.GetSystemOverlay();

  Dali::RenderTaskList taskList = mOverlay->GetOverlayRenderTasks();
  taskList.CreateTask();

  mAdaptor = &adaptorImpl;
  mAdaptor->AddObserver( *this );

  // Can only create the detector when we know the Core has been instantiated.
  mDragAndDropDetector = DragAndDropDetector::New();
  mAdaptor->SetDragAndDropDetector( &GetImplementation( mDragAndDropDetector ) );

  if( mOrientation )
  {
    mOrientation->SetAdaptor(adaptor);
  }

  if( mIndicator != NULL )
  {
    mIndicator->SetAdaptor(mAdaptor);
  }
}

WindowRenderSurface* Window::GetSurface()
{
  return mSurface;
}

void Window::ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode )
{
  // TODO: Enable this after indicator implementation based on tizen 5.
//  mIndicatorVisible = visibleMode;

  mWindowBase->ShowIndicator( mIndicatorVisible, mIndicatorOpacityMode );

  DoShowIndicator( mIndicatorOrientation );
}

void Window::SetIndicatorBgOpacity( Dali::Window::IndicatorBgOpacity opacityMode )
{
  mIndicatorOpacityMode = opacityMode;

  if( mIndicator != NULL )
  {
    mIndicator->SetOpacityMode( opacityMode );
  }
}

void Window::SetIndicatorVisibleMode( Dali::Window::IndicatorVisibleMode mode )
{
  // TODO: Enable this after indicator implementation based on tizen 5.
//  mIndicatorVisible = mode;
}

void Window::RotateIndicator( Dali::Window::WindowOrientation orientation )
{
  DALI_LOG_TRACE_METHOD_FMT( gWindowLogFilter, "Orientation: %d\n", orientation );

  DoRotateIndicator( orientation );
}

void Window::SetClass( std::string name, std::string className )
{
  mWindowBase->SetClass( name, className );
}

void Window::Raise()
{
  mWindowBase->Raise();
}

void Window::Lower()
{
  mWindowBase->Lower();
}

void Window::Activate()
{
  mWindowBase->Activate();
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

Dali::DragAndDropDetector Window::GetDragAndDropDetector() const
{
  return mDragAndDropDetector;
}

Dali::Any Window::GetNativeHandle() const
{
  return mSurface->GetNativeWindow();
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
    DALI_LOG_RELEASE_INFO( "Window (%p) ::Show()\n", this );
  }
}

void Window::Hide()
{
  mVisible = false;

  mWindowBase->Hide();

  if( !mIconified )
  {
    WindowVisibilityObserver* observer( mAdaptor );
    observer->OnWindowHidden();
    DALI_LOG_RELEASE_INFO( "Window (%p) ::Hide() \n", this );
  }
}

bool Window::IsVisible() const
{
  return mVisible;
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

  PositionSize positionSize = mSurface->GetPositionSize();

  if( positionSize.width != size.GetWidth() || positionSize.height != size.GetHeight() )
  {
    positionSize.width = size.GetWidth();
    positionSize.height = size.GetHeight();

    mSurface->MoveResize( positionSize );

    mAdaptor->SurfaceResizePrepare( Adaptor::SurfaceSize( positionSize.width, positionSize.height ) );

    // Emit signal
    mResizedSignal.Emit( Dali::Window::WindowSize( positionSize.width, positionSize.height ) );

    mAdaptor->SurfaceResizeComplete( Adaptor::SurfaceSize( positionSize.width, positionSize.height ) );
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

  PositionSize positionSize = mSurface->GetPositionSize();

  if( positionSize.x != position.GetX() || positionSize.y != position.GetY() )
  {
    positionSize.x = position.GetX();
    positionSize.y = position.GetY();

    mSurface->MoveResize( positionSize );
  }
}

Dali::Window::WindowPosition Window::GetPosition() const
{
  PositionSize positionSize = mSurface->GetPositionSize();

  return Dali::Window::WindowPosition( positionSize.x, positionSize.y );
}

void Window::SetTransparency( bool transparent )
{
  mSurface->SetTransparency( transparent );
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

void Window::RotationDone( int orientation, int width, int height )
{
  mSurface->RequestRotation( orientation, width, height );

  mAdaptor->SurfaceResizePrepare( Adaptor::SurfaceSize( width, height ) );

  // Emit signal
  mResizedSignal.Emit( Dali::Window::WindowSize( width, height ) );

  mAdaptor->SurfaceResizeComplete( Adaptor::SurfaceSize( width, height ) );
}

void Window::DoShowIndicator( Dali::Window::WindowOrientation lastOrientation )
{
  if( mIndicator == NULL )
  {
    if( mIndicatorVisible != Dali::Window::INVISIBLE )
    {
      auto windowFactory = Dali::Internal::Adaptor::GetWindowFactory();
      mIndicator = windowFactory->CreateIndicator( mAdaptor, mIndicatorOrientation, this );
      if( mIndicator )
      {
        mIndicator->SetOpacityMode( mIndicatorOpacityMode );
        Dali::Actor actor = mIndicator->GetActor();
        SetIndicatorActorRotation();
        mOverlay->Add(actor);
      }
    }
    // else don't create a hidden indicator
  }
  else // Already have indicator
  {
    if( mIndicatorVisible == Dali::Window::VISIBLE )
    {
      // If we are resuming, and rotation has changed,
      if( mIndicatorIsShown == false && mIndicatorOrientation != mNextIndicatorOrientation )
      {
        // then close current indicator and open new one
        mShowRotatedIndicatorOnClose = true;
        mIndicator->Close(); // May synchronously call IndicatorClosed() callback & 1 level of recursion
        // Don't show actor - will contain indicator for old orientation.
      }
    }
  }

  // set indicator visible mode
  if( mIndicator != NULL )
  {
    mIndicator->SetVisible( mIndicatorVisible );
  }

  bool show = (mIndicatorVisible != Dali::Window::INVISIBLE );
  SetIndicatorProperties( show, lastOrientation );
  mIndicatorIsShown = show;
}

void Window::DoRotateIndicator( Dali::Window::WindowOrientation orientation )
{
  if( mIndicatorIsShown )
  {
    mShowRotatedIndicatorOnClose = true;
    mNextIndicatorOrientation = orientation;
    if( mIndicator )
    {
      mIndicator->Close(); // May synchronously call IndicatorClosed() callback
    }
  }
  else
  {
    // Save orientation for when the indicator is next shown
    mShowRotatedIndicatorOnClose = false;
    mNextIndicatorOrientation = orientation;
  }
}

void Window::SetIndicatorActorRotation()
{
  DALI_LOG_TRACE_METHOD( gWindowLogFilter );
  if( mIndicator )
  {
    Dali::Actor actor = mIndicator->GetActor();
    switch( mIndicatorOrientation )
    {
      case Dali::Window::PORTRAIT:
        actor.SetParentOrigin( ParentOrigin::TOP_CENTER );
        actor.SetAnchorPoint(  AnchorPoint::TOP_CENTER );
        actor.SetOrientation( Degree(0), Vector3::ZAXIS );
        break;
      case Dali::Window::PORTRAIT_INVERSE:
        actor.SetParentOrigin( ParentOrigin::BOTTOM_CENTER );
        actor.SetAnchorPoint(  AnchorPoint::TOP_CENTER );
        actor.SetOrientation( Degree(180), Vector3::ZAXIS );
        break;
      case Dali::Window::LANDSCAPE:
        actor.SetParentOrigin( ParentOrigin::CENTER_LEFT );
        actor.SetAnchorPoint(  AnchorPoint::TOP_CENTER );
        actor.SetOrientation( Degree(270), Vector3::ZAXIS );
        break;
      case Dali::Window::LANDSCAPE_INVERSE:
        actor.SetParentOrigin( ParentOrigin::CENTER_RIGHT );
        actor.SetAnchorPoint(  AnchorPoint::TOP_CENTER );
        actor.SetOrientation( Degree(90), Vector3::ZAXIS );
        break;
    }
  }
}

void Window::SetIndicatorProperties( bool isShow, Dali::Window::WindowOrientation lastOrientation )
{
  mWindowBase->SetIndicatorProperties( isShow, lastOrientation );
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
      DALI_LOG_RELEASE_INFO( "Window (%p) Iconified\n", this );
    }
  }
  else
  {
    mIconified = false;

    if( mVisible )
    {
      WindowVisibilityObserver* observer( mAdaptor );
      observer->OnWindowShown();
      DALI_LOG_RELEASE_INFO( "Window (%p) Deiconified\n", this );
    }
  }
}

void Window::OnFocusChanged( bool focusIn )
{
  mFocusChangedSignal.Emit( focusIn );
}

void Window::OnOutputTransformed()
{
  PositionSize positionSize = mSurface->GetPositionSize();
  mAdaptor->SurfaceResizePrepare( Adaptor::SurfaceSize( positionSize.width, positionSize.height ) );
  mAdaptor->SurfaceResizeComplete( Adaptor::SurfaceSize( positionSize.width, positionSize.height ) );
}

void Window::OnDeleteRequest()
{
  mDeleteRequestSignal.Emit();
}

void Window::OnIndicatorFlicked()
{
  if( mIndicator )
  {
    mIndicator->Flicked();
  }
}

void Window::IndicatorTypeChanged( IndicatorInterface::Type type )
{
  mWindowBase->IndicatorTypeChanged( type );
}

void Window::IndicatorClosed( IndicatorInterface* indicator )
{
  DALI_LOG_TRACE_METHOD( gWindowLogFilter );

  if( mShowRotatedIndicatorOnClose )
  {
    Dali::Window::WindowOrientation currentOrientation = mIndicatorOrientation;
    if( mIndicator )
    {
      mIndicator->Open( mNextIndicatorOrientation );
    }
    mIndicatorOrientation = mNextIndicatorOrientation;
    SetIndicatorActorRotation();
    DoShowIndicator( currentOrientation );
  }
}

void Window::IndicatorVisibilityChanged( bool isVisible )
{
  mIndicatorVisibilityChangedSignal.Emit( isVisible );
}

void Window::OnStart()
{
  DoShowIndicator( mIndicatorOrientation );
}

void Window::OnPause()
{
}

void Window::OnResume()
{
  // resume indicator status
  if( mIndicator != NULL )
  {
    // Restore own indicator opacity
    // Send opacity mode to indicator service when app resumed
    mIndicator->SetOpacityMode( mIndicatorOpacityMode );
  }
}

void Window::OnStop()
{
  if( mIndicator )
  {
    mIndicator->Close();
  }

  mIndicator.release();
}

void Window::OnDestroy()
{
  mAdaptor = NULL;
}

} // Adaptor

} // Internal

} // Dali
