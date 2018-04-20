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
#include <dali/internal/window-system/ubuntu-x11/window-base-ecore-x.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/ubuntu-x11/window-render-surface-ecore-x.h>

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

static Eina_Bool EcoreEventWindowPropertyChanged( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    return windowBase->OnWindowPropertyChanged( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the window receives a delete request
static Eina_Bool EcoreEventWindowDeleteRequest( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnDeleteRequest();
  }
  return ECORE_CALLBACK_DONE;
}

} // unnamed namespace

WindowBaseEcoreX::WindowBaseEcoreX( Window* window, WindowRenderSurface* windowRenderSurface )
: mEcoreEventHandler(),
  mWindow( window ),
  mWindowSurface( NULL ),
  mEcoreWindow( 0 ),
  mRotationAppSet( false )
{
  mWindowSurface = dynamic_cast< WindowRenderSurfaceEcoreX* >( windowRenderSurface );
}

WindowBaseEcoreX::~WindowBaseEcoreX()
{
  for( Dali::Vector< Ecore_Event_Handler* >::Iterator iter = mEcoreEventHandler.Begin(), endIter = mEcoreEventHandler.End(); iter != endIter; ++iter )
  {
    ecore_event_handler_del( *iter );
  }
  mEcoreEventHandler.Clear();
}

void WindowBaseEcoreX::Initialize()
{
  if( !mWindowSurface )
  {
    DALI_ASSERT_ALWAYS( "Invalid window surface" );
  }

  mEcoreWindow = mWindowSurface->GetXWindow();
  DALI_ASSERT_ALWAYS( mEcoreWindow != 0 && "There is no EcoreX window" );

  ecore_x_input_multi_select( mEcoreWindow );

  // This ensures that we catch the window close (or delete) request
  ecore_x_icccm_protocol_set( mEcoreWindow, ECORE_X_WM_PROTOCOL_DELETE_REQUEST, EINA_TRUE );

  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_WINDOW_PROPERTY, EcoreEventWindowPropertyChanged, this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_WINDOW_DELETE_REQUEST, EcoreEventWindowDeleteRequest, this ) );
}

Eina_Bool WindowBaseEcoreX::OnWindowPropertyChanged( void* data, int type, void* event )
{
  Ecore_X_Event_Window_Property* propertyChangedEvent = static_cast< Ecore_X_Event_Window_Property* >( event );
  Eina_Bool handled( ECORE_CALLBACK_PASS_ON );

  if( propertyChangedEvent->win == mEcoreWindow )
  {
    Ecore_X_Window_State_Hint state( ecore_x_icccm_state_get( propertyChangedEvent->win ) );

    switch( state )
    {
      case ECORE_X_WINDOW_STATE_HINT_WITHDRAWN:
      {
        // Window was hidden.
        mWindow->OnIconifyChanged( true );
        DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%d) Withdrawn\n", mWindow );
        handled = ECORE_CALLBACK_DONE;
        break;
      }
      case ECORE_X_WINDOW_STATE_HINT_ICONIC:
      {
        // Window was iconified (minimised).
        mWindow->OnIconifyChanged( true );
        DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%d) Iconfied\n", mWindow );
        handled = ECORE_CALLBACK_DONE;
        break;
      }
      case ECORE_X_WINDOW_STATE_HINT_NORMAL:
      {
        // Window was shown.
        mWindow->OnIconifyChanged( false );
        DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%d) Shown\n", mWindow );
        handled = ECORE_CALLBACK_DONE;
        break;
      }
      default:
      {
        // Ignore
        break;
      }
    }
  }

  return handled;
}

void WindowBaseEcoreX::OnDeleteRequest()
{
  mWindow->OnDeleteRequest();
}

void WindowBaseEcoreX::ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode, Dali::Window::IndicatorBgOpacity opacityMode )
{
  DALI_LOG_TRACE_METHOD_FMT( gWindowBaseLogFilter, "visible : %d\n", visibleMode );

  if( visibleMode == Dali::Window::VISIBLE )
  {
    // when the indicator is visible, set proper mode for indicator server according to bg mode
    if( opacityMode == Dali::Window::OPAQUE )
    {
      ecore_x_e_illume_indicator_opacity_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_OPAQUE );
    }
    else if( opacityMode == Dali::Window::TRANSLUCENT )
    {
      ecore_x_e_illume_indicator_opacity_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_TRANSLUCENT );
    }
#if defined (DALI_PROFILE_MOBILE)
    else if( opacityMode == Dali::Window::TRANSPARENT )
    {
      ecore_x_e_illume_indicator_opacity_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_OPAQUE );
    }
#endif
  }
  else
  {
    // when the indicator is not visible, set TRANSPARENT mode for indicator server
    ecore_x_e_illume_indicator_opacity_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_TRANSPARENT ); // it means hidden indicator
  }
}

void WindowBaseEcoreX::SetIndicatorProperties( bool isShow, Dali::Window::WindowOrientation lastOrientation )
{
  int show_state = static_cast< int >( isShow );
  ecore_x_window_prop_property_set( mEcoreWindow, ECORE_X_ATOM_E_ILLUME_INDICATOR_STATE,
                                    ECORE_X_ATOM_CARDINAL, 32, &show_state, 1 );

  if( isShow )
  {
    ecore_x_e_illume_indicator_state_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_STATE_ON );
  }
  else
  {
    ecore_x_e_illume_indicator_state_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_STATE_OFF );
  }
}

void WindowBaseEcoreX::IndicatorTypeChanged( IndicatorInterface::Type type )
{
}

void WindowBaseEcoreX::SetClass( std::string name, std::string className )
{
  ecore_x_icccm_name_class_set( mEcoreWindow, name.c_str(), className.c_str() );
}

void WindowBaseEcoreX::Raise()
{
  ecore_x_window_raise( mEcoreWindow );
}

void WindowBaseEcoreX::Lower()
{
  ecore_x_window_lower( mEcoreWindow );
}

void WindowBaseEcoreX::Activate()
{
  ecore_x_netwm_client_active_request( ecore_x_window_root_get( mEcoreWindow ), mEcoreWindow, 1 /* request type, 1:application, 2:pager */, 0 );
}

void WindowBaseEcoreX::SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations )
{
}

void WindowBaseEcoreX::SetPreferredOrientation( Dali::Window::WindowOrientation orientation )
{
}

void WindowBaseEcoreX::SetAcceptFocus( bool accept )
{
}

void WindowBaseEcoreX::Show()
{
  ecore_x_window_show( mEcoreWindow );
}

void WindowBaseEcoreX::Hide()
{
  ecore_x_window_hide( mEcoreWindow );
}

unsigned int WindowBaseEcoreX::GetSupportedAuxiliaryHintCount() const
{
  return 0;
}

std::string WindowBaseEcoreX::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  return std::string();
}

unsigned int WindowBaseEcoreX::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return 0;
}

bool WindowBaseEcoreX::RemoveAuxiliaryHint( unsigned int id )
{
  return false;
}

bool WindowBaseEcoreX::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return false;
}

std::string WindowBaseEcoreX::GetAuxiliaryHintValue( unsigned int id ) const
{
  return std::string();
}

unsigned int WindowBaseEcoreX::GetAuxiliaryHintId( const std::string& hint ) const
{
  return 0;
}

void WindowBaseEcoreX::SetInputRegion( const Rect< int >& inputRegion )
{
}

void WindowBaseEcoreX::SetType( Dali::Window::Type type )
{
}

bool WindowBaseEcoreX::SetNotificationLevel( Dali::Window::NotificationLevel::Type level )
{
  return false;
}

Dali::Window::NotificationLevel::Type WindowBaseEcoreX::GetNotificationLevel() const
{
  return Dali::Window::NotificationLevel::NONE;
}

void WindowBaseEcoreX::SetOpaqueState( bool opaque )
{
}

bool WindowBaseEcoreX::SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode)
{
  return false;
}

Dali::Window::ScreenOffMode::Type WindowBaseEcoreX::GetScreenOffMode() const
{
  return Dali::Window::ScreenOffMode::TIMEOUT;
}

bool WindowBaseEcoreX::SetBrightness( int brightness )
{
  return false;
}

int WindowBaseEcoreX::GetBrightness() const
{
  return 0;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
