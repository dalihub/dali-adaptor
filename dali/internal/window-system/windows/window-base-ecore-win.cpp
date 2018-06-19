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
#include <dali/internal/window-system/windows/window-base-ecore-win.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/windows/window-render-surface-ecore-win.h>

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

static bool EcoreEventWindowPropertyChanged( void* data, int type, void* event )
{
  WindowBaseEcoreWin* windowBase = static_cast< WindowBaseEcoreWin* >( data );
  if( windowBase )
  {
    return windowBase->OnWindowPropertyChanged( data, type, event );
  }

  return true;
}

/// Called when the window receives a delete request
static bool EcoreEventWindowDeleteRequest( void* data, int type, void* event )
{
  WindowBaseEcoreWin* windowBase = static_cast< WindowBaseEcoreWin* >( data );
  if( windowBase )
  {
    windowBase->OnDeleteRequest();
  }
  return true;
}

} // unnamed namespace

WindowBaseEcoreWin::WindowBaseEcoreWin( Window* window, WindowRenderSurface* windowRenderSurface )
: mEcoreEventHandler(),
  mWindow( window ),
  mWindowSurface( NULL ),
  mEcoreWindow( 0 ),
  mRotationAppSet( false )
{
  mWindowSurface = dynamic_cast< WindowRenderSurfaceEcoreWin* >( windowRenderSurface );
}

WindowBaseEcoreWin::~WindowBaseEcoreWin()
{
  for( Dali::Vector< Ecore_Event_Handler* >::Iterator iter = mEcoreEventHandler.Begin(), endIter = mEcoreEventHandler.End(); iter != endIter; ++iter )
  {
    ecore_event_handler_del( **iter );
  }
  mEcoreEventHandler.Clear();
}

void WindowBaseEcoreWin::Initialize()
{
  if( !mWindowSurface )
  {
    DALI_ASSERT_ALWAYS( "Invalid window surface" );
  }

  mEcoreWindow = mWindowSurface->GetWinWindow();
  DALI_ASSERT_ALWAYS( mEcoreWindow != 0 && "There is no EcoreWin window" );

  //ecore_x_input_multi_select( mEcoreWindow );

  // This ensures that we catch the window close (or delete) request
  //ecore_x_icccm_protocol_set( mEcoreWindow, ECORE_X_WM_PROTOCOL_DELETE_REQUEST, EINA_TRUE );

  //mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_WINDOW_PROPERTY, EcoreEventWindowPropertyChanged, this ) );
  //mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_WINDOW_DELETE_REQUEST, EcoreEventWindowDeleteRequest, this ) );
}

bool WindowBaseEcoreWin::OnWindowPropertyChanged( void* data, int type, void* event )
{
  //Ecore_X_Event_Window_Property* propertyChangedEvent = static_cast< Ecore_X_Event_Window_Property* >( event );
  //bool handled( ECORE_CALLBACK_PASS_ON );

  //if( propertyChangedEvent->win == mEcoreWindow )
  //{
  //  Ecore_X_Window_State_Hint state( ecore_x_icccm_state_get( propertyChangedEvent->win ) );

  //  switch( state )
  //  {
  //    case ECORE_X_WINDOW_STATE_HINT_WITHDRAWN:
  //    {
  //      // Window was hidden.
  //      mWindow->OnIconifyChanged( true );
  //      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%d) Withdrawn\n", mWindow );
  //      handled = ECORE_CALLBACK_DONE;
  //      break;
  //    }
  //    case ECORE_X_WINDOW_STATE_HINT_ICONIC:
  //    {
  //      // Window was iconified (minimised).
  //      mWindow->OnIconifyChanged( true );
  //      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%d) Iconfied\n", mWindow );
  //      handled = ECORE_CALLBACK_DONE;
  //      break;
  //    }
  //    case ECORE_X_WINDOW_STATE_HINT_NORMAL:
  //    {
  //      // Window was shown.
  //      mWindow->OnIconifyChanged( false );
  //      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%d) Shown\n", mWindow );
  //      handled = ECORE_CALLBACK_DONE;
  //      break;
  //    }
  //    default:
  //    {
  //      // Ignore
  //      break;
  //    }
  //  }
  //}

  //return handled;
  return true;
}

void WindowBaseEcoreWin::OnDeleteRequest()
{
  mWindow->OnDeleteRequest();
}

void WindowBaseEcoreWin::ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode, Dali::Window::IndicatorBgOpacity opacityMode )
{
//  DALI_LOG_TRACE_METHOD_FMT( gWindowBaseLogFilter, "visible : %d\n", visibleMode );
//
//  if( visibleMode == Dali::Window::VISIBLE )
//  {
//    // when the indicator is visible, set proper mode for indicator server according to bg mode
//    if( opacityMode == Dali::Window::OPAQUE )
//    {
//      ecore_x_e_illume_indicator_opacity_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_OPAQUE );
//    }
//    else if( opacityMode == Dali::Window::TRANSLUCENT )
//    {
//      ecore_x_e_illume_indicator_opacity_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_TRANSLUCENT );
//    }
//#if defined (DALI_PROFILE_MOBILE)
//    else if( opacityMode == Dali::Window::TRANSPARENT )
//    {
//      ecore_x_e_illume_indicator_opacity_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_OPAQUE );
//    }
//#endif
//  }
//  else
//  {
//    // when the indicator is not visible, set TRANSPARENT mode for indicator server
//    ecore_x_e_illume_indicator_opacity_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_TRANSPARENT ); // it means hidden indicator
//  }
}

void WindowBaseEcoreWin::SetIndicatorProperties( bool isShow, Dali::Window::WindowOrientation lastOrientation )
{
  /*int show_state = static_cast< int >( isShow );
  ecore_x_window_prop_property_set( mEcoreWindow, ECORE_X_ATOM_E_ILLUME_INDICATOR_STATE,
                                    ECORE_X_ATOM_CARDINAL, 32, &show_state, 1 );

  if( isShow )
  {
    ecore_x_e_illume_indicator_state_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_STATE_ON );
  }
  else
  {
    ecore_x_e_illume_indicator_state_set( mEcoreWindow, ECORE_X_ILLUME_INDICATOR_STATE_OFF );
  }*/
}

void WindowBaseEcoreWin::IndicatorTypeChanged( IndicatorInterface::Type type )
{
}

void WindowBaseEcoreWin::SetClass( std::string name, std::string className )
{
  //ecore_x_icccm_name_class_set( mEcoreWindow, name.c_str(), className.c_str() );
}

void WindowBaseEcoreWin::Raise()
{
  //ecore_x_window_raise( mEcoreWindow );
}

void WindowBaseEcoreWin::Lower()
{
  //ecore_x_window_lower( mEcoreWindow );
}

void WindowBaseEcoreWin::Activate()
{
  //ecore_x_netwm_client_active_request( ecore_x_window_root_get( mEcoreWindow ), mEcoreWindow, 1 /* request type, 1:application, 2:pager */, 0 );
}

void WindowBaseEcoreWin::SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations )
{
}

void WindowBaseEcoreWin::SetPreferredOrientation( Dali::Window::WindowOrientation orientation )
{
}

void WindowBaseEcoreWin::SetAcceptFocus( bool accept )
{
}

void WindowBaseEcoreWin::Show()
{
  //ecore_x_window_show( mEcoreWindow );
}

void WindowBaseEcoreWin::Hide()
{
  //ecore_x_window_hide( mEcoreWindow );
}

unsigned int WindowBaseEcoreWin::GetSupportedAuxiliaryHintCount() const
{
  return 0;
}

std::string WindowBaseEcoreWin::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  return std::string();
}

unsigned int WindowBaseEcoreWin::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return 0;
}

bool WindowBaseEcoreWin::RemoveAuxiliaryHint( unsigned int id )
{
  return false;
}

bool WindowBaseEcoreWin::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return false;
}

std::string WindowBaseEcoreWin::GetAuxiliaryHintValue( unsigned int id ) const
{
  return std::string();
}

unsigned int WindowBaseEcoreWin::GetAuxiliaryHintId( const std::string& hint ) const
{
  return 0;
}

void WindowBaseEcoreWin::SetInputRegion( const Rect< int >& inputRegion )
{
}

void WindowBaseEcoreWin::SetType( Dali::Window::Type type )
{
}

bool WindowBaseEcoreWin::SetNotificationLevel( Dali::Window::NotificationLevel::Type level )
{
  return false;
}

Dali::Window::NotificationLevel::Type WindowBaseEcoreWin::GetNotificationLevel() const
{
  return Dali::Window::NotificationLevel::NONE;
}

void WindowBaseEcoreWin::SetOpaqueState( bool opaque )
{
}

bool WindowBaseEcoreWin::SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode)
{
  return false;
}

Dali::Window::ScreenOffMode::Type WindowBaseEcoreWin::GetScreenOffMode() const
{
  return Dali::Window::ScreenOffMode::TIMEOUT;
}

bool WindowBaseEcoreWin::SetBrightness( int brightness )
{
  return false;
}

int WindowBaseEcoreWin::GetBrightness() const
{
  return 0;
}

bool WindowBaseEcoreWin::GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode )
{
  return false;
}

bool WindowBaseEcoreWin::UngrabKey( Dali::KEY key )
{
  return false;
}

bool WindowBaseEcoreWin::GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result )
{
  return false;
}

bool WindowBaseEcoreWin::UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result )
{
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
