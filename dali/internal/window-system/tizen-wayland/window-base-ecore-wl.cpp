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

// Ecore is littered with C style cast
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

// CLASS HEADER
#include <dali/internal/window-system/tizen-wayland/window-base-ecore-wl.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/tizen-wayland/window-render-surface-ecore-wl.h>
#include <dali/internal/input/common/key-impl.h>

// EXTERNAL_HEADERS
#include <dali/public-api/object/any.h>
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_WINDOW_BASE" );
#endif

const uint32_t MAX_TIZEN_CLIENT_VERSION = 7;

/// Called when the window iconify state is changed.
static Eina_Bool EcoreEventWindowIconifyStateChanged( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    return windowBase->OnIconifyStateChanged( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the window gains focus
static Eina_Bool EcoreEventWindowFocusIn( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    return windowBase->OnFocusIn( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the window loses focus
static Eina_Bool EcoreEventWindowFocusOut( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    return windowBase->OnFocusOut( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the output is transformed
static Eina_Bool EcoreEventOutputTransform( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    return windowBase->OnOutputTransform( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the output transform should be ignored
static Eina_Bool EcoreEventIgnoreOutputTransform( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    return windowBase->OnIgnoreOutputTransform( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

static void RegistryGlobalCallback( void* data, struct wl_registry *registry, uint32_t name, const char* interface, uint32_t version )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    windowBase->RegistryGlobalCallback( data, registry, name, interface, version );
  }
}

static void RegistryGlobalCallbackRemove( void* data, struct wl_registry* registry, uint32_t id )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    windowBase->RegistryGlobalCallbackRemove( data, registry, id );
  }
}

static void TizenPolicyConformant( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t isConformant )
{
}

static void TizenPolicyConformantArea( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t conformantPart, uint32_t state, int32_t x, int32_t y, int32_t w, int32_t h )
{
}

static void TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    windowBase->TizenPolicyNotificationChangeDone( data, tizenPolicy, surface, level, state );
  }
}

static void TizenPolicyTransientForDone( void* data, struct tizen_policy* tizenPolicy, uint32_t childId )
{
}

static void TizenPolicyScreenModeChangeDone( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    windowBase->TizenPolicyScreenModeChangeDone( data, tizenPolicy, surface, mode, state );
  }
}

static void TizenPolicyIconifyStateChanged( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t iconified, uint32_t force )
{
}

static void TizenPolicySupportedAuxiliaryHints( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, struct wl_array* hints, uint32_t numNints )
{
}

static void TizenPolicyAllowedAuxiliaryHint( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int id )
{
}

static void TizenPolicyAuxiliaryMessage( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, const char* key, const char* val, struct wl_array* options )
{
}

static void TizenPolicyConformantRegion( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t conformantPart, uint32_t state, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t serial )
{
}

static void DisplayPolicyBrightnessChangeDone( void* data, struct tizen_display_policy *displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    windowBase->DisplayPolicyBrightnessChangeDone( data, displayPolicy, surface, brightness, state );
  }
}

const struct wl_registry_listener registryListener =
{
   RegistryGlobalCallback,
   RegistryGlobalCallbackRemove
};

const struct tizen_policy_listener tizenPolicyListener =
{
   TizenPolicyConformant,
   TizenPolicyConformantArea,
   TizenPolicyNotificationChangeDone,
   TizenPolicyTransientForDone,
   TizenPolicyScreenModeChangeDone,
   TizenPolicyIconifyStateChanged,
   TizenPolicySupportedAuxiliaryHints,
   TizenPolicyAllowedAuxiliaryHint,
   TizenPolicyAuxiliaryMessage,
   TizenPolicyConformantRegion
};

const struct tizen_display_policy_listener tizenDisplayPolicyListener =
{
  DisplayPolicyBrightnessChangeDone
};

} // unnamed namespace

WindowBaseEcoreWl::WindowBaseEcoreWl( Window* window, WindowRenderSurface* windowRenderSurface )
: mEcoreEventHandler(),
  mWindow( window ),
  mWindowSurface( NULL ),
  mEcoreWindow( NULL ),
  mDisplay( NULL ),
  mEventQueue( NULL ),
  mTizenPolicy( NULL ),
  mTizenDisplayPolicy( NULL ),
  mSupportedAuxiliaryHints(),
  mAuxiliaryHints(),
  mNotificationLevel( -1 ),
  mNotificationChangeState( 0 ),
  mNotificationLevelChangeDone( true ),
  mScreenOffMode( 0 ),
  mScreenOffModeChangeState( 0 ),
  mScreenOffModeChangeDone( true ),
  mBrightness( 0 ),
  mBrightnessChangeState( 0 ),
  mBrightnessChangeDone( true )
{
  mWindowSurface = dynamic_cast< WindowRenderSurfaceEcoreWl* >( windowRenderSurface );
}

WindowBaseEcoreWl::~WindowBaseEcoreWl()
{
  for( Dali::Vector< Ecore_Event_Handler* >::Iterator iter = mEcoreEventHandler.Begin(), endIter = mEcoreEventHandler.End(); iter != endIter; ++iter )
  {
    ecore_event_handler_del( *iter );
  }
  mEcoreEventHandler.Clear();

  if( mEventQueue )
  {
    wl_event_queue_destroy( mEventQueue );
  }

  mSupportedAuxiliaryHints.clear();
  mAuxiliaryHints.clear();
}

void WindowBaseEcoreWl::Initialize()
{
  if( !mWindowSurface )
  {
    DALI_ASSERT_ALWAYS( "Invalid window surface" );
  }

  mEcoreWindow = mWindowSurface->GetWlWindow();
  DALI_ASSERT_ALWAYS( mEcoreWindow != 0 && "There is no EcoreWl window" );

  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_WINDOW_ICONIFY_STATE_CHANGE, EcoreEventWindowIconifyStateChanged, this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_IN, EcoreEventWindowFocusIn, this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_OUT, EcoreEventWindowFocusOut, this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_OUTPUT_TRANSFORM, EcoreEventOutputTransform, this) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_IGNORE_OUTPUT_TRANSFORM, EcoreEventIgnoreOutputTransform, this) );

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
        wl_registry_add_listener( registry, &registryListener, this );
      }

      wl_proxy_wrapper_destroy( displayWrapper );
    }
  }

  // get auxiliary hint
  Eina_List* hints = ecore_wl_window_aux_hints_supported_get( mEcoreWindow );
  if( hints )
  {
    Eina_List* l = NULL;
    char* hint = NULL;

    for( l = hints, ( hint =  static_cast< char* >( eina_list_data_get(l) ) ); l; l = eina_list_next(l), ( hint = static_cast< char* >( eina_list_data_get(l) ) ) )
    {
      mSupportedAuxiliaryHints.push_back( hint );

      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::Initialize: %s\n", hint );
    }
  }
}

Eina_Bool WindowBaseEcoreWl::OnIconifyStateChanged( void* data, int type, void* event )
{
  Ecore_Wl_Event_Window_Iconify_State_Change* iconifyChangedEvent( static_cast< Ecore_Wl_Event_Window_Iconify_State_Change* >( event ) );
  Eina_Bool handled( ECORE_CALLBACK_PASS_ON );

  if( iconifyChangedEvent->win == static_cast< unsigned int>( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    if( iconifyChangedEvent->iconified == EINA_TRUE )
    {
      mWindow->OnIconifyChanged( true );
    }
    else
    {
      mWindow->OnIconifyChanged( false );
    }
    handled = ECORE_CALLBACK_DONE;
  }

  return handled;
}

Eina_Bool WindowBaseEcoreWl::OnFocusIn( void* data, int type, void* event )
{
  Ecore_Wl_Event_Focus_In* focusInEvent( static_cast< Ecore_Wl_Event_Focus_In* >( event ) );

  if( focusInEvent->win == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusIn\n" );

    mWindow->OnFocusChanged( true );
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl::OnFocusOut( void* data, int type, void* event )
{
  Ecore_Wl_Event_Focus_Out* focusOutEvent( static_cast< Ecore_Wl_Event_Focus_Out* >( event ) );

  if( focusOutEvent->win == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusOut\n" );

    mWindow->OnFocusChanged( false );
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl::OnOutputTransform( void* data, int type, void* event )
{
  Ecore_Wl_Event_Output_Transform* transformEvent( static_cast< Ecore_Wl_Event_Output_Transform* >( event ) );

  if( transformEvent->output == ecore_wl_window_output_find( mEcoreWindow ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%p) EcoreEventOutputTransform\n", mEcoreWindow );

    mWindowSurface->OutputTransformed();

    mWindow->OnOutputTransformed();
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl::OnIgnoreOutputTransform( void* data, int type, void* event )
{
  Ecore_Wl_Event_Ignore_Output_Transform* ignoreTransformEvent( static_cast< Ecore_Wl_Event_Ignore_Output_Transform* >( event ) );

  if( ignoreTransformEvent->win == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%p) EcoreEventIgnoreOutputTransform\n", mEcoreWindow );

    mWindowSurface->OutputTransformed();

    mWindow->OnOutputTransformed();
  }

  return ECORE_CALLBACK_PASS_ON;
}

void WindowBaseEcoreWl::RegistryGlobalCallback( void* data, struct wl_registry *registry, uint32_t name, const char* interface, uint32_t version )
{
  if( strcmp( interface, tizen_policy_interface.name ) == 0 )
  {
    uint32_t clientVersion = std::min( version, MAX_TIZEN_CLIENT_VERSION );

    mTizenPolicy = static_cast< tizen_policy* >( wl_registry_bind( registry, name, &tizen_policy_interface, clientVersion ) );
    if( !mTizenPolicy )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::RegistryGlobalCallback: wl_registry_bind(tizen_policy_interface) is failed.\n" );
      return;
    }

    tizen_policy_add_listener( mTizenPolicy, &tizenPolicyListener, data );

    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::RegistryGlobalCallback: tizen_policy_add_listener is called.\n" );
  }
  else if( strcmp( interface, tizen_display_policy_interface.name ) == 0 )
  {
    mTizenDisplayPolicy = static_cast< tizen_display_policy* >( wl_registry_bind( registry, name, &tizen_display_policy_interface, version ) );
    if( !mTizenDisplayPolicy )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::RegistryGlobalCallback: wl_registry_bind(tizen_display_policy_interface) is failed.\n" );
      return;
    }

    tizen_display_policy_add_listener( mTizenDisplayPolicy, &tizenDisplayPolicyListener, data );

    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::RegistryGlobalCallback: tizen_display_policy_add_listener is called.\n" );
  }
}

void WindowBaseEcoreWl::RegistryGlobalCallbackRemove( void* data, struct wl_registry* registry, uint32_t id )
{
  mTizenPolicy = NULL;
  mTizenDisplayPolicy = NULL;
}

void WindowBaseEcoreWl::TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state )
{
  mNotificationLevel = level;
  mNotificationChangeState = state;
  mNotificationLevelChangeDone = true;

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::TizenPolicyNotificationChangeDone: level = %d, state = %d\n", level, state );
}

void WindowBaseEcoreWl::TizenPolicyScreenModeChangeDone( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state )
{
  mScreenOffMode = mode;
  mScreenOffModeChangeState = state;
  mScreenOffModeChangeDone = true;

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::TizenPolicyScreenModeChangeDone: mode = %d, state = %d\n", mode, state );
}

void WindowBaseEcoreWl::DisplayPolicyBrightnessChangeDone( void* data, struct tizen_display_policy *displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state )
{
  mBrightness = brightness;
  mBrightnessChangeState = state;
  mBrightnessChangeDone = true;

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::DisplayPolicyBrightnessChangeDone: brightness = %d, state = %d\n", brightness, state );
}

void WindowBaseEcoreWl::ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode, Dali::Window::IndicatorBgOpacity opacityMode )
{
  DALI_LOG_TRACE_METHOD_FMT( gWindowBaseLogFilter, "visible : %d\n", visibleMode );

  if( visibleMode == Dali::Window::VISIBLE )
  {
    // when the indicator is visible, set proper mode for indicator server according to bg mode
    if( opacityMode == Dali::Window::OPAQUE )
    {
      ecore_wl_window_indicator_opacity_set( mEcoreWindow, ECORE_WL_INDICATOR_OPAQUE );
    }
    else if( opacityMode == Dali::Window::TRANSLUCENT )
    {
      ecore_wl_window_indicator_opacity_set( mEcoreWindow, ECORE_WL_INDICATOR_TRANSLUCENT );
    }
    else if( opacityMode == Dali::Window::TRANSPARENT )
    {
      ecore_wl_window_indicator_opacity_set( mEcoreWindow, ECORE_WL_INDICATOR_OPAQUE );
    }
  }
  else
  {
    // when the indicator is not visible, set TRANSPARENT mode for indicator server
    ecore_wl_window_indicator_opacity_set( mEcoreWindow, ECORE_WL_INDICATOR_TRANSPARENT); // it means hidden indicator
  }
}

void WindowBaseEcoreWl::SetIndicatorProperties( bool isShow, Dali::Window::WindowOrientation lastOrientation )
{
  if( isShow )
  {
    ecore_wl_window_indicator_state_set( mEcoreWindow, ECORE_WL_INDICATOR_STATE_ON );
  }
  else
  {
    ecore_wl_window_indicator_state_set( mEcoreWindow, ECORE_WL_INDICATOR_STATE_OFF );
  }
}

void WindowBaseEcoreWl::IndicatorTypeChanged( IndicatorInterface::Type type )
{
#if defined(DALI_PROFILE_MOBILE)
  switch( type )
  {
    case IndicatorInterface::INDICATOR_TYPE_1:
    {
      ecore_wl_indicator_visible_type_set( mEcoreWindow, ECORE_WL_INDICATOR_VISIBLE_TYPE_SHOWN );
      break;
    }
    case IndicatorInterface::INDICATOR_TYPE_2:
    {
      ecore_wl_indicator_visible_type_set( mEcoreWindow, ECORE_WL_INDICATOR_VISIBLE_TYPE_HIDDEN );
      break;
    }
    case IndicatorInterface::INDICATOR_TYPE_UNKNOWN:
    default:
    {
      break;
    }
  }
#endif //MOBILE
}

void WindowBaseEcoreWl::SetClass( std::string name, std::string className )
{
  ecore_wl_window_title_set( mEcoreWindow, name.c_str() );
  ecore_wl_window_class_name_set( mEcoreWindow, className.c_str() );
}

void WindowBaseEcoreWl::Raise()
{
  // Use ecore_wl_window_activate to prevent the window shown without rendering
  ecore_wl_window_activate( mEcoreWindow );
}

void WindowBaseEcoreWl::Lower()
{
  ecore_wl_window_lower( mEcoreWindow );
}

void WindowBaseEcoreWl::Activate()
{
  ecore_wl_window_activate( mEcoreWindow );
}

void WindowBaseEcoreWl::SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations )
{
  int rotations[4] = { 0 };
  for( std::size_t i = 0; i < orientations.size(); ++i )
  {
    rotations[i] = static_cast< int >( orientations[i] );
  }
  ecore_wl_window_rotation_available_rotations_set( mEcoreWindow, rotations, orientations.size() );
}

void WindowBaseEcoreWl::SetPreferredOrientation( Dali::Window::WindowOrientation orientation )
{
  ecore_wl_window_rotation_preferred_rotation_set( mEcoreWindow, orientation );
}

void WindowBaseEcoreWl::SetAcceptFocus( bool accept )
{
  ecore_wl_window_focus_skip_set( mEcoreWindow, !accept );
}

void WindowBaseEcoreWl::Show()
{
  ecore_wl_window_show( mEcoreWindow );
}

void WindowBaseEcoreWl::Hide()
{
  ecore_wl_window_hide( mEcoreWindow );
}

unsigned int WindowBaseEcoreWl::GetSupportedAuxiliaryHintCount() const
{
  return mSupportedAuxiliaryHints.size();
}

std::string WindowBaseEcoreWl::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  if( index >= GetSupportedAuxiliaryHintCount() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetSupportedAuxiliaryHint: Invalid index! [%d]\n", index );
  }

  return mSupportedAuxiliaryHints[index];
}

unsigned int WindowBaseEcoreWl::AddAuxiliaryHint( const std::string& hint, const std::string& value )
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
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::AddAuxiliaryHint: Not supported auxiliary hint [%s]\n", hint.c_str() );
    return 0;
  }

  // Check if the hint is already added
  for( unsigned int i = 0; i < mAuxiliaryHints.size(); i++ )
  {
    if( mAuxiliaryHints[i].first == hint )
    {
      // Just change the value
      mAuxiliaryHints[i].second = value;

      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::AddAuxiliaryHint: Change! hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), i + 1 );

      return i + 1;   // id is index + 1
    }
  }

  // Add the hint
  mAuxiliaryHints.push_back( std::pair< std::string, std::string >( hint, value ) );

  unsigned int id = mAuxiliaryHints.size();

  ecore_wl_window_aux_hint_add( mEcoreWindow, static_cast< int >( id ), hint.c_str(), value.c_str() );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::AddAuxiliaryHint: hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id );

  return id;
}

bool WindowBaseEcoreWl::RemoveAuxiliaryHint( unsigned int id )
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::RemoveAuxiliaryHint: Invalid id [%d]\n", id );
    return false;
  }

  mAuxiliaryHints[id - 1].second = std::string();

  ecore_wl_window_aux_hint_del( mEcoreWindow, static_cast< int >( id ) );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::RemoveAuxiliaryHint: id = %d, hint = %s\n", id, mAuxiliaryHints[id - 1].first.c_str() );

  return true;
}

bool WindowBaseEcoreWl::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::SetAuxiliaryHintValue: Invalid id [%d]\n", id );
    return false;
  }

  mAuxiliaryHints[id - 1].second = value;

  ecore_wl_window_aux_hint_change( mEcoreWindow, static_cast< int >( id ), value.c_str() );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str() );

  return true;
}

std::string WindowBaseEcoreWl::GetAuxiliaryHintValue( unsigned int id ) const
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::GetAuxiliaryHintValue: Invalid id [%d]\n", id );
    return std::string();
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str() );

  return mAuxiliaryHints[id - 1].second;
}

unsigned int WindowBaseEcoreWl::GetAuxiliaryHintId( const std::string& hint ) const
{
  for( unsigned int i = 0; i < mAuxiliaryHints.size(); i++ )
  {
    if( mAuxiliaryHints[i].first == hint )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetAuxiliaryHintId: hint = %s, id = %d\n", hint.c_str(), i + 1 );
      return i + 1;
    }
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetAuxiliaryHintId: Invalid hint! [%s]\n", hint.c_str() );

  return 0;
}

void WindowBaseEcoreWl::SetInputRegion( const Rect< int >& inputRegion )
{
  ecore_wl_window_input_region_set( mEcoreWindow, inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height );
}

void WindowBaseEcoreWl::SetType( Dali::Window::Type type )
{
  Ecore_Wl_Window_Type windowType;

  switch( type )
  {
    case Dali::Window::NORMAL:
    {
      windowType = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
      break;
    }
    case Dali::Window::NOTIFICATION:
    {
      windowType = ECORE_WL_WINDOW_TYPE_NOTIFICATION;
      break;
    }
    case Dali::Window::UTILITY:
    {
      windowType = ECORE_WL_WINDOW_TYPE_UTILITY;
      break;
    }
    case Dali::Window::DIALOG:
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

  ecore_wl_window_type_set( mEcoreWindow, windowType );
}

bool WindowBaseEcoreWl::SetNotificationLevel( Dali::Window::NotificationLevel::Type level )
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int notificationLevel;

  switch( level )
  {
    case Dali::Window::NotificationLevel::NONE:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_NONE;
      break;
    }
    case Dali::Window::NotificationLevel::BASE:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_DEFAULT;
      break;
    }
    case Dali::Window::NotificationLevel::MEDIUM:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_MEDIUM;
      break;
    }
    case Dali::Window::NotificationLevel::HIGH:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_HIGH;
      break;
    }
    case Dali::Window::NotificationLevel::TOP:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_TOP;
      break;
    }
    default:
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetNotificationLevel: invalid level [%d]\n", level );
      notificationLevel = TIZEN_POLICY_LEVEL_DEFAULT;
      break;
    }
  }

  mNotificationLevelChangeDone = false;
  mNotificationChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_policy_set_notification_level( mTizenPolicy, ecore_wl_window_surface_get( mEcoreWindow ), notificationLevel );

  int count = 0;

  while( !mNotificationLevelChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mNotificationLevelChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetNotificationLevel: Level change is failed [%d, %d]\n", level, mNotificationChangeState );
    return false;
  }
  else if( mNotificationChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetNotificationLevel: Permission denied! [%d]\n", level );
    return false;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetNotificationLevel: Level is changed [%d]\n", mNotificationLevel );

  return true;
}

Dali::Window::NotificationLevel::Type WindowBaseEcoreWl::GetNotificationLevel() const
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int count = 0;

  while( !mNotificationLevelChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mNotificationLevelChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetNotificationLevel: Error! [%d]\n", mNotificationChangeState );
    return Dali::Window::NotificationLevel::NONE;
  }

  Dali::Window::NotificationLevel::Type level;

  switch( mNotificationLevel )
  {
    case TIZEN_POLICY_LEVEL_NONE:
    {
      level = Dali::Window::NotificationLevel::NONE;
      break;
    }
    case TIZEN_POLICY_LEVEL_DEFAULT:
    {
      level = Dali::Window::NotificationLevel::BASE;
      break;
    }
    case TIZEN_POLICY_LEVEL_MEDIUM:
    {
      level = Dali::Window::NotificationLevel::MEDIUM;
      break;
    }
    case TIZEN_POLICY_LEVEL_HIGH:
    {
      level = Dali::Window::NotificationLevel::HIGH;
      break;
    }
    case TIZEN_POLICY_LEVEL_TOP:
    {
      level = Dali::Window::NotificationLevel::TOP;
      break;
    }
    default:
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetNotificationLevel: invalid level [%d]\n", mNotificationLevel );
      level = Dali::Window::NotificationLevel::NONE;
      break;
    }
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetNotificationLevel: level [%d]\n", mNotificationLevel );

  return level;
}

void WindowBaseEcoreWl::SetOpaqueState( bool opaque )
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  tizen_policy_set_opaque_state( mTizenPolicy, ecore_wl_window_surface_get( mEcoreWindow ), ( opaque ? 1 : 0 ) );
}

bool WindowBaseEcoreWl::SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode)
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  mScreenOffModeChangeDone = false;
  mScreenOffModeChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  unsigned int mode = 0;

  switch( screenOffMode )
  {
    case Dali::Window::ScreenOffMode::TIMEOUT:
    {
      mode = 0;
      break;
    }
    case Dali::Window::ScreenOffMode::NEVER:
    {
      mode = 1;
      break;
    }
  }

  tizen_policy_set_window_screen_mode( mTizenPolicy, ecore_wl_window_surface_get( mEcoreWindow ), mode );

  int count = 0;

  while( !mScreenOffModeChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mScreenOffModeChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetScreenOffMode: Screen mode change is failed [%d, %d]\n", screenOffMode, mScreenOffModeChangeState );
    return false;
  }
  else if( mScreenOffModeChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetScreenOffMode: Permission denied! [%d]\n", screenOffMode );
    return false;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetScreenOffMode: Screen mode is changed [%d]\n", mScreenOffMode );

  return true;
}

Dali::Window::ScreenOffMode::Type WindowBaseEcoreWl::GetScreenOffMode() const
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int count = 0;

  while( !mScreenOffModeChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mScreenOffModeChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetScreenOffMode: Error! [%d]\n", mScreenOffModeChangeState );
    return Dali::Window::ScreenOffMode::TIMEOUT;
  }

  Dali::Window::ScreenOffMode::Type screenMode = Dali::Window::ScreenOffMode::TIMEOUT;

  switch( mScreenOffMode )
  {
    case 0:
    {
      screenMode = Dali::Window::ScreenOffMode::TIMEOUT;
      break;
    }
    case 1:
    {
      screenMode = Dali::Window::ScreenOffMode::NEVER;
      break;
    }
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetScreenOffMode: screen mode [%d]\n", mScreenOffMode );

  return screenMode;
}

bool WindowBaseEcoreWl::SetBrightness( int brightness )
{
  while( !mTizenDisplayPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  mBrightnessChangeDone = false;
  mBrightnessChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_display_policy_set_window_brightness( mTizenDisplayPolicy, ecore_wl_window_surface_get( mEcoreWindow ), brightness );

  int count = 0;

  while( !mBrightnessChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mBrightnessChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetBrightness: Brightness change is failed [%d, %d]\n", brightness, mBrightnessChangeState );
    return false;
  }
  else if( mBrightnessChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetBrightness: Permission denied! [%d]\n", brightness );
    return false;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetBrightness: Brightness is changed [%d]\n", mBrightness );

  return true;
}

int WindowBaseEcoreWl::GetBrightness() const
{
  while( !mTizenDisplayPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int count = 0;

  while( !mBrightnessChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mBrightnessChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetBrightness: Error! [%d]\n", mBrightnessChangeState );
    return 0;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetBrightness: Brightness [%d]\n", mBrightness );

  return mBrightness;
}

bool WindowBaseEcoreWl::GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode )
{
  Ecore_Wl_Window_Keygrab_Mode mode;

  switch( grabMode )
  {
    case KeyGrab::TOPMOST:
    {
      mode = ECORE_WL_WINDOW_KEYGRAB_TOPMOST;
      break;
    }
    case KeyGrab::SHARED:
    {
      mode = ECORE_WL_WINDOW_KEYGRAB_SHARED;
      break;
    }
    case KeyGrab::OVERRIDE_EXCLUSIVE:
    {
      mode = ECORE_WL_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
      break;
    }
    case KeyGrab::EXCLUSIVE:
    {
      mode = ECORE_WL_WINDOW_KEYGRAB_EXCLUSIVE;
      break;
    }
    default:
    {
      return false;
    }
  }

  return ecore_wl_window_keygrab_set( mEcoreWindow, KeyLookup::GetKeyName( key ), 0, 0, 0, mode );
}

bool WindowBaseEcoreWl::UngrabKey( Dali::KEY key )
{
  return ecore_wl_window_keygrab_unset( mEcoreWindow, KeyLookup::GetKeyName( key ), 0, 0 );
}

bool WindowBaseEcoreWl::GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result )
{
  int keyCount = key.Count();
  int keyGrabModeCount = grabMode.Count();

  if( keyCount != keyGrabModeCount || keyCount == 0 )
  {
    return false;
  }

  eina_init();

  Eina_List* keyList = NULL;
  Ecore_Wl_Window_Keygrab_Info* info = new Ecore_Wl_Window_Keygrab_Info[keyCount];

  for( int index = 0; index < keyCount; ++index )
  {
    info[index].key = const_cast< char* >( KeyLookup::GetKeyName( key[index] ) );

    switch( grabMode[index] )
    {
      case KeyGrab::TOPMOST:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_TOPMOST;
        break;
      }
      case KeyGrab::SHARED:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_SHARED;
        break;
      }
      case KeyGrab::OVERRIDE_EXCLUSIVE:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
        break;
      }
      case KeyGrab::EXCLUSIVE:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_EXCLUSIVE;
        break;
      }
      default:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_UNKNOWN;
        break;
      }
    }

    keyList = eina_list_append( keyList, &info );
  }

  Eina_List* grabList = ecore_wl_window_keygrab_list_set( mEcoreWindow, keyList );

  result.Resize( keyCount, true );

  Eina_List* l = NULL;
  Eina_List* m = NULL;
  void* listData = NULL;
  void* data = NULL;
  if( grabList != NULL )
  {
    EINA_LIST_FOREACH( grabList, m, data )
    {
      int index = 0;
      EINA_LIST_FOREACH( keyList, l, listData )
      {
        if( static_cast< Ecore_Wl_Window_Keygrab_Info* >( listData )->key == NULL )
        {
          DALI_LOG_ERROR( "input key list has null data!" );
          break;
        }

        if( strcmp( static_cast< char* >( data ), static_cast< Ecore_Wl_Window_Keygrab_Info* >( listData )->key ) == 0 )
        {
          result[index] = false;
        }
        ++index;
      }
    }
  }

  delete [] info;

  eina_list_free( keyList );
  eina_list_free( grabList );
  eina_shutdown();

  return true;
}

bool WindowBaseEcoreWl::UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result )
{
  int keyCount = key.Count();
  if( keyCount == 0 )
  {
    return false;
  }

  eina_init();

  Eina_List* keyList = NULL;
  Ecore_Wl_Window_Keygrab_Info* info = new Ecore_Wl_Window_Keygrab_Info[keyCount];

  for( int index = 0; index < keyCount; ++index )
  {
    info[index].key = const_cast< char* >( KeyLookup::GetKeyName( key[index] ) );
    keyList = eina_list_append( keyList, &info );
  }

  Eina_List* ungrabList = ecore_wl_window_keygrab_list_unset( mEcoreWindow, keyList );

  result.Resize( keyCount, true );

  Eina_List* l = NULL;
  Eina_List* m = NULL;
  void *listData = NULL;
  void *data = NULL;

  if( ungrabList != NULL )
  {
    EINA_LIST_FOREACH( ungrabList, m, data )
    {
      int index = 0;
      EINA_LIST_FOREACH( keyList, l, listData )
      {
        if( strcmp( static_cast< char* >( data ), static_cast< Ecore_Wl_Window_Keygrab_Info* >( listData )->key ) == 0 )
        {
          result[index] = false;
        }
        ++index;
      }
    }
  }

  delete [] info;

  eina_list_free( keyList );
  eina_list_free( ungrabList );
  eina_shutdown();

  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
