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
// Ecore is littered with C style cast
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <tizen-extension-client-protocol.h>

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
Debug::Filter* gWindowLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_WINDOW");
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
    mWindowVisibilityStateHandler( NULL ),
    mWindowFocusInHandler( NULL ),
    mWindowFocusOutHandler( NULL ),
    mEcoreWindow( 0 ),
    mDisplay( NULL ),
    mEventQueue( NULL ),
    mTizenPolicy( NULL ),
    mTizenDisplayPolicy( NULL ),
    mNotificationLevel( -1 ),
    mNotificationChangeState( 0 ),
    mNotificationLevelChangeDone( true ),
    mScreenMode( 0 ),
    mScreenModeChangeState( 0 ),
    mScreenModeChangeDone( true ),
    mBrightness( 0 ),
    mBrightnessChangeState( 0 ),
    mBrightnessChangeDone( true )
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
      mWindowVisibilityStateHandler = ecore_event_handler_add( ECORE_WL_EVENT_WINDOW_VISIBILITY_CHANGE, EcoreEventWindowVisibilityChanged, this );
      mWindowFocusInHandler = ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_IN, EcoreEventWindowFocusIn, this );
      mWindowFocusOutHandler = ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_OUT, EcoreEventWindowFocusOut, this );
    }

    mDisplay = ecore_wl_display_get();

    if( mDisplay )
    {
      wl_display* displayWrapper = static_cast< wl_display* >( wl_proxy_create_wrapper( mDisplay ) );
      if( displayWrapper )
      {
        mEventQueue = wl_display_create_queue( mDisplay );
        if( mEventQueue )
        {
          wl_proxy_set_queue( reinterpret_cast< wl_proxy* >( displayWrapper ), mEventQueue );

          wl_registry* registry = wl_display_get_registry( displayWrapper );
          wl_registry_add_listener( registry, &mRegistryListener, this );
        }

        wl_proxy_wrapper_destroy( displayWrapper );
      }
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

    if ( mWindowVisibilityStateHandler )
    {
      ecore_event_handler_del( mWindowVisibilityStateHandler );
    }

    if( mWindowFocusInHandler )
    {
      ecore_event_handler_del( mWindowFocusInHandler );
    }

    if( mWindowFocusOutHandler )
    {
      ecore_event_handler_del( mWindowFocusOutHandler );
    }

    if( mEventQueue )
    {
      wl_event_queue_destroy( mEventQueue );
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

  /// Called when the window visibility is changed.
  static Eina_Bool EcoreEventWindowVisibilityChanged( void* data, int type, void* event )
  {
    Ecore_Wl_Event_Window_Visibility_Change* visibilityChangedEvent( static_cast< Ecore_Wl_Event_Window_Visibility_Change* >( event ) );
    EventHandler* handler( static_cast< EventHandler* >( data ) );
    Eina_Bool handled( ECORE_CALLBACK_PASS_ON );

    if ( handler && handler->mWindow )
    {
      WindowVisibilityObserver* observer( handler->mWindow->mAdaptor );
      if ( observer && ( visibilityChangedEvent->win == static_cast< unsigned int >( ecore_wl_window_id_get( handler->mEcoreWindow ) ) ) )
      {
        if( visibilityChangedEvent->fully_obscured == 1 )
        {
          observer->OnWindowHidden();
          DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window (%d) full obscured\n", handler->mEcoreWindow );
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

  static void RegistryGlobalCallback( void* data, struct wl_registry *registry, uint32_t name, const char* interface, uint32_t version )
  {
    Window::EventHandler* eventHandler = static_cast< Window::EventHandler* >( data );

    if( strcmp( interface, tizen_policy_interface.name ) == 0 )
    {
      eventHandler->mTizenPolicy = static_cast< tizen_policy* >( wl_registry_bind( registry, name, &tizen_policy_interface, version ) );
      if( !eventHandler->mTizenPolicy )
      {
        DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window::EventHandler::RegistryGlobalCallback: wl_registry_bind(tizen_policy_interface) is failed.\n" );
        return;
      }

      tizen_policy_add_listener( eventHandler->mTizenPolicy, &eventHandler->mTizenPolicyListener, data );

      DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window::EventHandler::RegistryGlobalCallback: tizen_policy_add_listener is called.\n" );
    }
    else if( strcmp( interface, tizen_display_policy_interface.name ) == 0 )
    {
      eventHandler->mTizenDisplayPolicy = static_cast< tizen_display_policy* >( wl_registry_bind( registry, name, &tizen_display_policy_interface, version ) );
      if( !eventHandler->mTizenDisplayPolicy )
      {
        DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window::EventHandler::RegistryGlobalCallback: wl_registry_bind(tizen_display_policy_interface) is failed.\n" );
        return;
      }

      tizen_display_policy_add_listener( eventHandler->mTizenDisplayPolicy, &eventHandler->mTizenDisplayPolicyListener, data );

      DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window::EventHandler::RegistryGlobalCallback: tizen_display_policy_add_listener is called.\n" );
    }
  }

  static void RegistryGlobalCallbackRemove( void* data, struct wl_registry* registry, uint32_t id )
  {
    Window::EventHandler* eventHandler = static_cast< Window::EventHandler* >( data );
    eventHandler->mTizenPolicy = NULL;
    eventHandler->mTizenDisplayPolicy = NULL;
  }

  static void TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state )
  {
    Window::EventHandler* eventHandler = static_cast< Window::EventHandler* >( data );

    eventHandler->mNotificationLevel = level;
    eventHandler->mNotificationChangeState = state;
    eventHandler->mNotificationLevelChangeDone = true;

    DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window::EventHandler::TizenPolicyNotificationChangeDone: level = %d, state = %d\n", level, state );
  }

  static void TizenPolicyScreenModeChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state )
  {
    Window::EventHandler* eventHandler = static_cast< Window::EventHandler* >( data );

    eventHandler->mScreenMode = mode;
    eventHandler->mScreenModeChangeState = state;
    eventHandler->mScreenModeChangeDone = true;

    DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window::EventHandler::TizenPolicyScreenModeChangeDone: mode = %d, state = %d\n", mode, state );
  }

  static void DisplayPolicyBrightnessChangeDone(void* data, struct tizen_display_policy *displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state )
  {
    Window::EventHandler* eventHandler = static_cast< Window::EventHandler* >( data );

    eventHandler->mBrightness = brightness;
    eventHandler->mBrightnessChangeState = state;
    eventHandler->mBrightnessChangeDone = true;

    DALI_LOG_INFO( gWindowLogFilter, Debug::General, "Window::EventHandler::DisplayPolicyBrightnessChangeDone: brightness = %d, state = %d\n", brightness, state );
  }

  const struct wl_registry_listener mRegistryListener =
  {
     RegistryGlobalCallback,
     RegistryGlobalCallbackRemove
  };

  const struct tizen_policy_listener mTizenPolicyListener =
  {
     NULL,
     NULL,
     TizenPolicyNotificationChangeDone,
     NULL,
     TizenPolicyScreenModeChangeDone,
     NULL,
     NULL,
     NULL,
     NULL
  };

  const struct tizen_display_policy_listener mTizenDisplayPolicyListener =
  {
    DisplayPolicyBrightnessChangeDone
  };

  // Data
  Window* mWindow;
  Ecore_Event_Handler* mWindowPropertyHandler;
  Ecore_Event_Handler* mWindowIconifyStateHandler;
  Ecore_Event_Handler* mWindowVisibilityStateHandler;
  Ecore_Event_Handler* mWindowFocusInHandler;
  Ecore_Event_Handler* mWindowFocusOutHandler;
  Ecore_Wl_Window* mEcoreWindow;

  wl_display* mDisplay;
  wl_event_queue* mEventQueue;
  tizen_policy* mTizenPolicy;
  tizen_display_policy* mTizenDisplayPolicy;

  int mNotificationLevel;
  uint32_t mNotificationChangeState;
  bool mNotificationLevelChangeDone;

  int mScreenMode;
  uint32_t mScreenModeChangeState;
  bool mScreenModeChangeDone;

  int mBrightness;
  uint32_t mBrightnessChangeState;
  bool mBrightnessChangeDone;
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

  if( wlSurface )
  {
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
  mVisible( true ),
  mOpaqueState( false ),
  mIndicator( NULL ),
  mIndicatorOrientation( Dali::Window::PORTRAIT ),
  mNextIndicatorOrientation( Dali::Window::PORTRAIT ),
  mIndicatorOpacityMode( Dali::Window::OPAQUE ),
  mOverlay( NULL ),
  mAdaptor( NULL ),
  mType( Dali::DevelWindow::NORMAL ),
  mEventHandler( NULL ),
  mPreferredOrientation( Dali::Window::PORTRAIT ),
  mSupportedAuxiliaryHints(),
  mAuxiliaryHints(),
  mIndicatorVisibilityChangedSignal(),
  mFocusChangedSignal(),
  mResizedSignal(),
  mDeleteRequestSignal()
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

  mSupportedAuxiliaryHints.clear();
  mAuxiliaryHints.clear();
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

  // get auxiliary hint
  Eina_List* hints = ecore_wl_window_aux_hints_supported_get( mEventHandler->mEcoreWindow );
  if( hints )
  {
    Eina_List* l = NULL;
    char* hint = NULL;

    for( l = hints, ( hint =  static_cast< char* >( eina_list_data_get(l) ) ); l; l = eina_list_next(l), ( hint = static_cast< char* >( eina_list_data_get(l) ) ) )
    {
      mSupportedAuxiliaryHints.push_back( hint );

      DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::Initialize: %s\n", hint );
    }
  }
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

void Window::Show()
{
  mVisible = true;
  ecore_wl_window_show( mEventHandler->mEcoreWindow );

  // Need an update request
  if( mAdaptor )
  {
    mAdaptor->RequestUpdateOnce();
  }
}

void Window::Hide()
{
  mVisible = false;
  ecore_wl_window_hide( mEventHandler->mEcoreWindow );
}

bool Window::IsVisible() const
{
  return mVisible;
}

void Window::RotationDone( int orientation, int width, int height )
{
  PositionSize positionSize( 0, 0, width, height );

  mAdaptor->SurfaceSizeChanged( positionSize );

  // Emit signal
  mResizedSignal.Emit( positionSize.width, positionSize.height );

  Dali::Window::WindowOrientation windowOrientation;
  switch( orientation )
  {
    case 0:
    {
      windowOrientation = Dali::Window::PORTRAIT;
      break;
    }
    case 90:
    {
      windowOrientation = Dali::Window::LANDSCAPE;
      break;
    }
    case 180:
    {
      windowOrientation = Dali::Window::PORTRAIT_INVERSE;
      break;
    }
    case 270:
    {
      windowOrientation = Dali::Window::LANDSCAPE_INVERSE;
      break;
    }
    default:
    {
      windowOrientation = Dali::Window::PORTRAIT;
      break;
    }
  }

  ECore::WindowRenderSurface* wlSurface( dynamic_cast< ECore::WindowRenderSurface * >( mSurface ) );
  wlSurface->RequestRotation( windowOrientation, positionSize.width, positionSize.height );
}

unsigned int Window::GetSupportedAuxiliaryHintCount()
{
  return mSupportedAuxiliaryHints.size();
}

std::string Window::GetSupportedAuxiliaryHint( unsigned int index )
{
  if( index >= GetSupportedAuxiliaryHintCount() )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetSupportedAuxiliaryHint: Invalid index! [%d]\n", index );
  }

  return mSupportedAuxiliaryHints[index];
}

unsigned int Window::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  bool supported = false;

  // Check if the hint is suppported
  for( std::vector< std::string >::iterator iter = mSupportedAuxiliaryHints.begin(); iter != mSupportedAuxiliaryHints.end(); ++iter )
  {
    if( *iter == hint )
    {
      supported = true;
      break;
    }
  }

  if( !supported )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Concise, "Window::AddAuxiliaryHint: Not supported auxiliary hint [%s]\n", hint.c_str() );
    return 0;
  }

  // Check if the hint is already added
  for( unsigned int i = 0; i < mAuxiliaryHints.size(); i++ )
  {
    if( mAuxiliaryHints[i].first == hint )
    {
      // Just change the value
      mAuxiliaryHints[i].second = value;

      DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::AddAuxiliaryHint: Change! hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), i + 1 );

      return i + 1;   // id is index + 1
    }
  }

  // Add the hint
  mAuxiliaryHints.push_back( std::pair< std::string, std::string >( hint, value ) );

  unsigned int id = mAuxiliaryHints.size();

  ecore_wl_window_aux_hint_add( mEventHandler->mEcoreWindow, static_cast< int >( id ), hint.c_str(), value.c_str() );

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::AddAuxiliaryHint: hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id );

  return id;
}

bool Window::RemoveAuxiliaryHint( unsigned int id )
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Concise, "Window::RemoveAuxiliaryHint: Invalid id [%d]\n", id );
    return false;
  }

  mAuxiliaryHints[id - 1].second = std::string();

  ecore_wl_window_aux_hint_del( mEventHandler->mEcoreWindow, static_cast< int >( id ) );

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::RemoveAuxiliaryHint: id = %d, hint = %s\n", id, mAuxiliaryHints[id - 1].first.c_str() );

  return true;
}

bool Window::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Concise, "Window::SetAuxiliaryHintValue: Invalid id [%d]\n", id );
    return false;
  }

  mAuxiliaryHints[id - 1].second = value;

  ecore_wl_window_aux_hint_change( mEventHandler->mEcoreWindow, static_cast< int >( id ), value.c_str() );

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str() );

  return true;
}

std::string Window::GetAuxiliaryHintValue( unsigned int id ) const
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Concise, "Window::GetAuxiliaryHintValue: Invalid id [%d]\n", id );
    return std::string();
  }

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str() );

  return mAuxiliaryHints[id - 1].second;
}

unsigned int Window::GetAuxiliaryHintId( const std::string& hint ) const
{
  for( unsigned int i = 0; i < mAuxiliaryHints.size(); i++ )
  {
    if( mAuxiliaryHints[i].first == hint )
    {
      DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetAuxiliaryHintId: hint = %s, id = %d\n", hint.c_str(), i + 1 );
      return i + 1;
    }
  }

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetAuxiliaryHintId: Invalid hint! [%s]\n", hint.c_str() );

  return 0;
}

void Window::SetInputRegion( const Rect< int >& inputRegion )
{
  ecore_wl_window_input_region_set( mEventHandler->mEcoreWindow, inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height );

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetInputRegion: x = %d, y = %d, w = %d, h = %d\n", inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height );
}

void Window::SetType( Dali::DevelWindow::Type type )
{
  Ecore_Wl_Window_Type windowType;

  if( type != mType )
  {
    switch( type )
    {
      case Dali::DevelWindow::NORMAL:
      {
        windowType = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
        break;
      }
      case Dali::DevelWindow::NOTIFICATION:
      {
        windowType = ECORE_WL_WINDOW_TYPE_NOTIFICATION;
        break;
      }
      case Dali::DevelWindow::UTILITY:
      {
        windowType = ECORE_WL_WINDOW_TYPE_UTILITY;
        break;
      }
      case Dali::DevelWindow::DIALOG:
      {
        windowType = ECORE_WL_WINDOW_TYPE_DIALOG;
        break;
      }
      default:
      {
        windowType = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
        break;
      }
    }

    ecore_wl_window_type_set( mEventHandler->mEcoreWindow, windowType );
  }

  mType = type;
}

Dali::DevelWindow::Type Window::GetType() const
{
  return mType;
}

bool Window::SetNotificationLevel( Dali::DevelWindow::NotificationLevel::Type level )
{
  if( mType != Dali::DevelWindow::NOTIFICATION )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetNotificationLevel: Not supported window type [%d]\n", mType );
    return false;
  }

  while( !mEventHandler->mTizenPolicy )
  {
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
  }

  int notificationLevel;

  switch( level )
  {
    case Dali::DevelWindow::NotificationLevel::NONE:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_NONE;
      break;
    }
    case Dali::DevelWindow::NotificationLevel::BASE:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_DEFAULT;
      break;
    }
    case Dali::DevelWindow::NotificationLevel::MEDIUM:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_MEDIUM;
      break;
    }
    case Dali::DevelWindow::NotificationLevel::HIGH:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_HIGH;
      break;
    }
    case Dali::DevelWindow::NotificationLevel::TOP:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_TOP;
      break;
    }
    default:
    {
      DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetNotificationLevel: invalid level [%d]\n", level );
      notificationLevel = TIZEN_POLICY_LEVEL_DEFAULT;
      break;
    }
  }

  mEventHandler->mNotificationLevelChangeDone = false;
  mEventHandler->mNotificationChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_policy_set_notification_level( mEventHandler->mTizenPolicy, ecore_wl_window_surface_get( mEventHandler->mEcoreWindow ), notificationLevel );

  int count = 0;

  while( !mEventHandler->mNotificationLevelChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
    count++;
  }

  if( !mEventHandler->mNotificationLevelChangeDone )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetNotificationLevel: Level change is failed [%d, %d]\n", level, mEventHandler->mNotificationChangeState );
    return false;
  }
  else if( mEventHandler->mNotificationChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetNotificationLevel: Permission denied! [%d]\n", level );
    return false;
  }

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetNotificationLevel: Level is changed [%d]\n", mEventHandler->mNotificationLevel );

  return true;
}

Dali::DevelWindow::NotificationLevel::Type Window::GetNotificationLevel()
{
  if( mType != Dali::DevelWindow::NOTIFICATION )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetNotificationLevel: Not supported window type [%d]\n", mType );
    return Dali::DevelWindow::NotificationLevel::NONE;
  }

  while( !mEventHandler->mTizenPolicy )
  {
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
  }

  int count = 0;

  while( !mEventHandler->mNotificationLevelChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
    count++;
  }

  if( !mEventHandler->mNotificationLevelChangeDone )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetNotificationLevel: Error! [%d]\n", mEventHandler->mNotificationChangeState );
    return Dali::DevelWindow::NotificationLevel::NONE;
  }

  Dali::DevelWindow::NotificationLevel::Type level;

  switch( mEventHandler->mNotificationLevel )
  {
    case TIZEN_POLICY_LEVEL_NONE:
    {
      level = Dali::DevelWindow::NotificationLevel::NONE;
      break;
    }
    case TIZEN_POLICY_LEVEL_DEFAULT:
    {
      level = Dali::DevelWindow::NotificationLevel::BASE;
      break;
    }
    case TIZEN_POLICY_LEVEL_MEDIUM:
    {
      level = Dali::DevelWindow::NotificationLevel::MEDIUM;
      break;
    }
    case TIZEN_POLICY_LEVEL_HIGH:
    {
      level = Dali::DevelWindow::NotificationLevel::HIGH;
      break;
    }
    case TIZEN_POLICY_LEVEL_TOP:
    {
      level = Dali::DevelWindow::NotificationLevel::TOP;
      break;
    }
    default:
    {
      DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetNotificationLevel: invalid level [%d]\n", mEventHandler->mNotificationLevel );
      level = Dali::DevelWindow::NotificationLevel::NONE;
      break;
    }
  }

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetNotificationLevel: level [%d]\n", mEventHandler->mNotificationLevel );

  return level;
}

void Window::SetOpaqueState( bool opaque )
{
  while( !mEventHandler->mTizenPolicy )
  {
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
  }

  tizen_policy_set_opaque_state( mEventHandler->mTizenPolicy, ecore_wl_window_surface_get( mEventHandler->mEcoreWindow ), ( opaque ? 1 : 0 ) );

  mOpaqueState = opaque;

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetOpaqueState: opaque = %d\n", opaque );
}

bool Window::IsOpaqueState()
{
  return mOpaqueState;
}

bool Window::SetScreenMode( Dali::DevelWindow::ScreenMode::Type screenMode )
{
  while( !mEventHandler->mTizenPolicy )
  {
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
  }

  mEventHandler->mScreenModeChangeDone = false;
  mEventHandler->mScreenModeChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  unsigned int mode = 0;

  switch( screenMode )
  {
    case Dali::DevelWindow::ScreenMode::DEFAULT:
    {
      mode = 0;
      break;
    }
    case Dali::DevelWindow::ScreenMode::ALWAYS_ON:
    {
      mode = 1;
      break;
    }
  }

  tizen_policy_set_window_screen_mode( mEventHandler->mTizenPolicy, ecore_wl_window_surface_get( mEventHandler->mEcoreWindow ), mode );

  int count = 0;

  while( !mEventHandler->mScreenModeChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
    count++;
  }

  if( !mEventHandler->mScreenModeChangeDone )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetScreenMode: Screen mode change is failed [%d, %d]\n", screenMode, mEventHandler->mScreenModeChangeState );
    return false;
  }
  else if( mEventHandler->mScreenModeChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetScreenMode: Permission denied! [%d]\n", screenMode );
    return false;
  }

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetScreenMode: Screen mode is changed [%d]\n", mEventHandler->mScreenMode );

  return true;
}

Dali::DevelWindow::ScreenMode::Type Window::GetScreenMode()
{
  while( !mEventHandler->mTizenPolicy )
  {
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
  }

  int count = 0;

  while( !mEventHandler->mScreenModeChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
    count++;
  }

  if( !mEventHandler->mScreenModeChangeDone )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetScreenMode: Error! [%d]\n", mEventHandler->mScreenModeChangeState );
    return Dali::DevelWindow::ScreenMode::DEFAULT;
  }

  Dali::DevelWindow::ScreenMode::Type screenMode = Dali::DevelWindow::ScreenMode::DEFAULT;

  switch( mEventHandler->mScreenMode )
  {
    case 0:
    {
      screenMode = Dali::DevelWindow::ScreenMode::DEFAULT;
      break;
    }
    case 1:
    {
      screenMode = Dali::DevelWindow::ScreenMode::ALWAYS_ON;
      break;
    }
  }

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetScreenMode: screen mode [%d]\n", mEventHandler->mScreenMode );

  return screenMode;
}

bool Window::SetBrightness( int brightness )
{
  if( brightness < 0 || brightness > 100 )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetBrightness: Invalid brightness value [%d]\n", brightness );
    return false;
  }

  while( !mEventHandler->mTizenDisplayPolicy )
  {
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
  }

  mEventHandler->mBrightnessChangeDone = false;
  mEventHandler->mBrightnessChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_display_policy_set_window_brightness( mEventHandler->mTizenDisplayPolicy, ecore_wl_window_surface_get( mEventHandler->mEcoreWindow ), brightness );

  int count = 0;

  while( !mEventHandler->mBrightnessChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
    count++;
  }

  if( !mEventHandler->mBrightnessChangeDone )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetBrightness: Brightness change is failed [%d, %d]\n", brightness, mEventHandler->mBrightnessChangeState );
    return false;
  }
  else if( mEventHandler->mBrightnessChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetBrightness: Permission denied! [%d]\n", brightness );
    return false;
  }

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::SetBrightness: Brightness is changed [%d]\n", mEventHandler->mBrightness );

  return true;
}

int Window::GetBrightness()
{
  while( !mEventHandler->mTizenDisplayPolicy )
  {
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
  }

  int count = 0;

  while( !mEventHandler->mBrightnessChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mEventHandler->mDisplay, mEventHandler->mEventQueue );
    count++;
  }

  if( !mEventHandler->mBrightnessChangeDone )
  {
    DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetBrightness: Error! [%d]\n", mEventHandler->mBrightnessChangeState );
    return 0;
  }

  DALI_LOG_INFO( gWindowLogFilter, Debug::Verbose, "Window::GetBrightness: Brightness [%d]\n", mEventHandler->mBrightness );

  return mEventHandler->mBrightness;
}

} // Adaptor

} // Internal

} // Dali

#pragma GCC diagnostic pop
