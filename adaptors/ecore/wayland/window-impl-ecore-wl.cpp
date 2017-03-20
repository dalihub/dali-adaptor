/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <window-impl.h>

// EXTERNAL HEADERS
#include <Ecore.h>
#include <Ecore_Wayland.h>

#include <dali/integration-api/core.h>
#include <dali/integration-api/system-overlay.h>
#include <dali/public-api/render-tasks/render-task.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <orientation.h>

// INTERNAL HEADERS
#include <window-render-surface.h>
#include <drag-and-drop-detector-impl.h>
#include <ecore-indicator-impl.h>
#include <window-visibility-observer.h>
#include <orientation-impl.h>

namespace
{
const float INDICATOR_ANIMATION_DURATION( 0.18f ); // 180 milli seconds
const float INDICATOR_SHOW_Y_POSITION( 0.0f );
const float INDICATOR_HIDE_Y_POSITION( -52.0f );
}

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowLogFilter = Debug::Filter::New(Debug::Concise, false, "LOG_WINDOW");
#endif

/**
 * TODO: Abstract Window class out and move this into a window implementation for Ecore
 */
struct Window::EventHandler
{
  /**
   * Constructor
   * @param[in]  window  A pointer to the window class.
   */
  EventHandler( Window* window )
  : mWindow( window ),
    mWindowPropertyHandler( NULL ),
    mWindowIconifyStateHandler( NULL ),
    mWindowFocusInHandler( NULL ),
    mWindowFocusOutHandler( NULL ),
    mEcoreWindow( 0 )
  {
    // store ecore window handle
    ECore::WindowRenderSurface* wlWindow( dynamic_cast< ECore::WindowRenderSurface * >( mWindow->mSurface ) );
    if( wlWindow )
    {
      mEcoreWindow = wlWindow->GetWlWindow();
    }
    DALI_ASSERT_ALWAYS( mEcoreWindow != 0 && "There is no ecore Wl window");

    if( mWindow->mEcoreEventHander )
    {
      mWindowIconifyStateHandler = ecore_event_handler_add( ECORE_WL_EVENT_WINDOW_ICONIFY_STATE_CHANGE, EcoreEventWindowIconifyStateChanged, this );
      mWindowFocusInHandler = ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_IN, EcoreEventWindowFocusIn, this );
      mWindowFocusOutHandler = ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_OUT, EcoreEventWindowFocusOut, this );
    }
  }

  /**
   * Destructor
   */
  ~EventHandler()
  {
    if ( mWindowPropertyHandler )
    {
      ecore_event_handler_del( mWindowPropertyHandler );
    }
    if ( mWindowIconifyStateHandler )
    {
      ecore_event_handler_del( mWindowIconifyStateHandler );
    }
    if( mWindowFocusInHandler )
    {
      ecore_event_handler_del( mWindowFocusInHandler );
    }
    if( mWindowFocusOutHandler )
    {
      ecore_event_handler_del( mWindowFocusOutHandler );
    }
  }

  // Static methods

  /// Called when the window properties are changed.
  static Eina_Bool EcoreEventWindowPropertyChanged( void* data, int type, void* event )
  {
    return EINA_FALSE;
  }

  /// Called when the window iconify state is changed.
  static Eina_Bool EcoreEventWindowIconifyStateChanged( void* data, int type, void* event )
  {
    Ecore_Wl_Event_Window_Iconify_State_Change* iconifyChangedEvent( static_cast< Ecore_Wl_Event_Window_Iconify_State_Change* >( event ) );
    EventHandler* handler( static_cast< EventHandler* >( data ) );
    Eina_Bool handled( ECORE_CALLBACK_PASS_ON );

    if ( handler && handler->mWindow )
    {
      WindowVisibilityObserver* observer( handler->mWindow->mAdaptor );
      if ( observer && ( iconifyChangedEvent->win == static_cast< unsigned int> ( ecore_wl_window_id_get( handler->mEcoreWindow ) ) ) )
      {
        if( iconifyChangedEvent->iconified == EINA_TRUE )
        {
          observer->OnWindowHidden();
          DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window (%d) Iconfied\n", handler->mEcoreWindow );
        }
        else
        {
          observer->OnWindowShown();
          DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window (%d) Shown\n", handler->mEcoreWindow );
        }
        handled = ECORE_CALLBACK_DONE;
      }
    }

    return handled;
  }

  /// Called when the window gains focus
  static Eina_Bool EcoreEventWindowFocusIn( void* data, int type, void* event )
  {
    Ecore_Wl_Event_Focus_In* focusInEvent( static_cast< Ecore_Wl_Event_Focus_In* >( event ) );
    EventHandler* handler( static_cast< EventHandler* >( data ) );

    if ( handler && handler->mWindow && focusInEvent->win == static_cast< unsigned int >( ecore_wl_window_id_get( handler->mEcoreWindow ) ) )
    {
      DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window EcoreEventWindowFocusIn\n" );

      handler->mWindow->mFocusChangedSignal.Emit( true );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /// Called when the window loses focus
  static Eina_Bool EcoreEventWindowFocusOut( void* data, int type, void* event )
  {
    Ecore_Wl_Event_Focus_Out* focusOutEvent( static_cast< Ecore_Wl_Event_Focus_Out* >( event ) );
    EventHandler* handler( static_cast< EventHandler* >( data ) );

    if ( handler && handler->mWindow && focusOutEvent->win == static_cast< unsigned int >(ecore_wl_window_id_get( handler->mEcoreWindow ) ) )
    {
      DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window EcoreEventWindowFocusOut\n" );

      handler->mWindow->mFocusChangedSignal.Emit( false );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  // Data
  Window* mWindow;
  Ecore_Event_Handler* mWindowPropertyHandler;
  Ecore_Event_Handler* mWindowIconifyStateHandler;
  Ecore_Event_Handler* mWindowFocusInHandler;
  Ecore_Event_Handler* mWindowFocusOutHandler;
  Ecore_Wl_Window* mEcoreWindow;
};

Window* Window::New(const PositionSize& posSize, const std::string& name, const std::string& className, bool isTransparent)
{
  Window* window = new Window();
  window->mIsTransparent = isTransparent;
  window->Initialize(posSize, name, className);
  return window;
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

RenderSurface* Window::GetSurface()
{
  return mSurface;
}

void Window::ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode )
{
  DALI_LOG_TRACE_METHOD_FMT( gWindowLogFilter, "visible : %d\n", visibleMode );
  DALI_ASSERT_DEBUG(mOverlay);

  ECore::WindowRenderSurface* wlSurface( dynamic_cast< ECore::WindowRenderSurface * >( mSurface ) );
  DALI_ASSERT_DEBUG(wlSurface);
  Ecore_Wl_Window* wlWindow = wlSurface->GetWlWindow();

  mIndicatorVisible = visibleMode;

  if ( mIndicatorVisible == Dali::Window::VISIBLE )
  {
    // when the indicator is visible, set proper mode for indicator server according to bg mode
    if ( mIndicatorOpacityMode == Dali::Window::OPAQUE )
    {
      ecore_wl_window_indicator_opacity_set(wlWindow, ECORE_WL_INDICATOR_OPAQUE);
    }
    else if ( mIndicatorOpacityMode == Dali::Window::TRANSLUCENT )
    {
      ecore_wl_window_indicator_opacity_set(wlWindow, ECORE_WL_INDICATOR_TRANSLUCENT);
    }
    else if ( mIndicatorOpacityMode == Dali::Window::TRANSPARENT )
    {
      ecore_wl_window_indicator_opacity_set(wlWindow, ECORE_WL_INDICATOR_OPAQUE);
    }
  }
  else
  {
    // when the indicator is not visible, set TRANSPARENT mode for indicator server
    ecore_wl_window_indicator_opacity_set(wlWindow, ECORE_WL_INDICATOR_TRANSPARENT); // it means hidden indicator
  }

  DoShowIndicator( mIndicatorOrientation );
}

void Window::RotateIndicator( Dali::Window::WindowOrientation orientation )
{
  DALI_LOG_TRACE_METHOD_FMT( gWindowLogFilter, "Orientation: %d\n", orientation );

  DoRotateIndicator( orientation );
}

void Window::SetIndicatorBgOpacity( Dali::Window::IndicatorBgOpacity opacityMode )
{
  mIndicatorOpacityMode = opacityMode;

  if( mIndicator != NULL )
  {
    mIndicator->SetOpacityMode( opacityMode );
  }
}

void Window::SetClass(std::string name, std::string klass)
{
  ECore::WindowRenderSurface* wlSurface( dynamic_cast< ECore::WindowRenderSurface * >( mSurface ) );

  if( wlSurface )
  {
    Ecore_Wl_Window* wlWindow = wlSurface->GetWlWindow();
    ecore_wl_window_title_set( wlWindow, name.c_str() );
    ecore_wl_window_class_name_set( wlWindow, klass.c_str() );
  }
  else
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window has no surface\n" );
  }
}

Window::Window()
: mSurface( NULL ),
  mIndicatorVisible( Dali::Window::VISIBLE ),
  mIndicatorIsShown( false ),
  mShowRotatedIndicatorOnClose( false ),
  mStarted( false ),
  mIsTransparent( false ),
  mWMRotationAppSet( false ),
  mEcoreEventHander( true ),
  mIsFocusAcceptable( true ),
  mIndicator( NULL ),
  mIndicatorOrientation( Dali::Window::PORTRAIT ),
  mNextIndicatorOrientation( Dali::Window::PORTRAIT ),
  mIndicatorOpacityMode( Dali::Window::OPAQUE ),
  mOverlay( NULL ),
  mAdaptor( NULL ),
  mEventHandler( NULL ),
  mPreferredOrientation( Dali::Window::PORTRAIT )
{
}

Window::~Window()
{
  delete mEventHandler;

  if( mIndicator )
  {
    mIndicator->Close();
    delete mIndicator;
  }

  if ( mAdaptor )
  {
    mAdaptor->RemoveObserver( *this );
    mAdaptor->SetDragAndDropDetector( NULL );
    mAdaptor = NULL;
  }

  delete mSurface;
}

void Window::Initialize(const PositionSize& windowPosition, const std::string& name, const std::string& className)
{
  // create an Wayland window by default
  Any surface;
  ECore::WindowRenderSurface* windowSurface = new ECore::WindowRenderSurface( windowPosition, surface, name, mIsTransparent );

  mSurface = windowSurface;
  SetClass( name, className );
  windowSurface->Map();

  mOrientation = Orientation::New(this);

  // create event handler for Wayland window
  mEventHandler = new EventHandler( this );
}

void Window::DoShowIndicator( Dali::Window::WindowOrientation lastOrientation )
{
  if( mIndicator == NULL )
  {
    if( mIndicatorVisible != Dali::Window::INVISIBLE )
    {
      mIndicator = new Indicator( mAdaptor, mIndicatorOrientation, this );
      mIndicator->SetOpacityMode( mIndicatorOpacityMode );
      Dali::Actor actor = mIndicator->GetActor();
      SetIndicatorActorRotation();
      mOverlay->Add(actor);
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
    mIndicator->Close(); // May synchronously call IndicatorClosed() callback
  }
  else
  {
    // Save orientation for when the indicator is next shown
    mShowRotatedIndicatorOnClose = false;
    mNextIndicatorOrientation = orientation;
  }
}

void Window::SetIndicatorProperties( bool isShow, Dali::Window::WindowOrientation lastOrientation )
{
  ECore::WindowRenderSurface* wlSurface( dynamic_cast< ECore::WindowRenderSurface * >( mSurface ) );

  if( wlSurface )
  {
    Ecore_Wl_Window* wlWindow = wlSurface->GetWlWindow();
    if ( isShow )
    {
      ecore_wl_window_indicator_state_set(wlWindow, ECORE_WL_INDICATOR_STATE_ON);
    }
    else
    {
      ecore_wl_window_indicator_state_set(wlWindow, ECORE_WL_INDICATOR_STATE_OFF);
    }
  }
}

void Window::IndicatorTypeChanged(Indicator::Type type)
{
#if defined(DALI_PROFILE_MOBILE)
  ECore::WindowRenderSurface* wlSurface( dynamic_cast< ECore::WindowRenderSurface * >( mSurface ) );

  if( wlSurface )
  {
    Ecore_Wl_Window* wlWindow = wlSurface->GetWlWindow();
    switch(type)
    {
      case Indicator::INDICATOR_TYPE_1:
        ecore_wl_indicator_visible_type_set(wlWindow, ECORE_WL_INDICATOR_VISIBLE_TYPE_SHOWN);
        break;

      case Indicator::INDICATOR_TYPE_2:
        ecore_wl_indicator_visible_type_set(wlWindow, ECORE_WL_INDICATOR_VISIBLE_TYPE_HIDDEN);
        break;

      case Indicator::INDICATOR_TYPE_UNKNOWN:
      default:
        break;
    }
  }
#endif //MOBILE
}

void Window::IndicatorClosed( IndicatorInterface* indicator )
{
  DALI_LOG_TRACE_METHOD( gWindowLogFilter );

  if( mShowRotatedIndicatorOnClose )
  {
    Dali::Window::WindowOrientation currentOrientation = mIndicatorOrientation;
    mIndicator->Open(mNextIndicatorOrientation);
    mIndicatorOrientation = mNextIndicatorOrientation;
    SetIndicatorActorRotation();
    DoShowIndicator(currentOrientation);
  }
}

void Window::IndicatorVisibilityChanged(bool isVisible)
{
  mIndicatorVisibilityChangedSignal.Emit(isVisible);
}

void Window::SetIndicatorActorRotation()
{
  DALI_LOG_TRACE_METHOD( gWindowLogFilter );
  DALI_ASSERT_DEBUG( mIndicator != NULL );

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

void Window::Raise()
{
  ecore_wl_window_raise( mEventHandler->mEcoreWindow );
}

void Window::Lower()
{
  ecore_wl_window_lower( mEventHandler->mEcoreWindow );
}

void Window::Activate()
{
  ecore_wl_window_activate( mEventHandler->mEcoreWindow );
}

Dali::DragAndDropDetector Window::GetDragAndDropDetector() const
{
  return mDragAndDropDetector;
}

Dali::Any Window::GetNativeHandle() const
{
  if(mEventHandler)
  {
    return mEventHandler->mEcoreWindow;
  }
  else
  {
    return Dali::Any();
  }
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

  delete mIndicator;
  mIndicator = NULL;
}

void Window::OnDestroy()
{
  mAdaptor = NULL;
}

void Window::AddAvailableOrientation(Dali::Window::WindowOrientation orientation)
{
  bool found = false;

  for( std::size_t i=0; i<mAvailableOrientations.size(); i++ )
  {
    if(mAvailableOrientations[i] == orientation)
    {
      found = true;
      break;
    }
  }

  if( ! found )
  {
    mAvailableOrientations.push_back(orientation);
    SetAvailableOrientations( mAvailableOrientations );
  }
}

void Window::RemoveAvailableOrientation(Dali::Window::WindowOrientation orientation)
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

void Window::SetAvailableOrientations(const std::vector<Dali::Window::WindowOrientation>& orientations)
{
  int rotations[4];
  for( std::size_t i = 0; i < mAvailableOrientations.size(); ++i )
  {
    rotations[i] = static_cast< int >( mAvailableOrientations[i] );
  }
  ecore_wl_window_rotation_available_rotations_set( mEventHandler->mEcoreWindow, rotations, mAvailableOrientations.size() );
}

const std::vector<Dali::Window::WindowOrientation>& Window::GetAvailableOrientations()
{
  return mAvailableOrientations;
}

void Window::SetPreferredOrientation(Dali::Window::WindowOrientation orientation)
{
  mPreferredOrientation = orientation;

  ecore_wl_window_rotation_preferred_rotation_set( mEventHandler->mEcoreWindow, orientation );
}

Dali::Window::WindowOrientation Window::GetPreferredOrientation()
{
  return mPreferredOrientation;
}

void Window::SetAcceptFocus( bool accept )
{
  mIsFocusAcceptable = accept;

  ecore_wl_window_focus_skip_set( mEventHandler->mEcoreWindow, !accept );
}

bool Window::IsFocusAcceptable()
{
  return mIsFocusAcceptable;
}

void Window::RotationDone( int orientation, int width, int height )
{
  ecore_wl_window_rotation_change_done_send( mEventHandler->mEcoreWindow );
}

} // Adaptor
} // Internal
} // Dali
