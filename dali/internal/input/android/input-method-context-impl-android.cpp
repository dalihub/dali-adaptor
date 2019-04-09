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
#include <dali/internal/input/android/input-method-context-impl-android.h>

// EXTERNAL INCLUDES
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/adaptor-framework/key.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/locale-utils.h>
#include <dali/internal/system/common/singleton-service-impl.h>
#include <dali/internal/input/common/virtual-keyboard-impl.h>
#include <dali/internal/input/common/key-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_INPUT_METHOD_CONTEXT");
#endif
}

InputMethodContextPtr InputMethodContextAndroid::New()
{
  InputMethodContextPtr manager;

  if ( Adaptor::IsAvailable() )
  {
    // Create instance and register singleton only if the adaptor is available
    Adaptor& adaptorImpl( Adaptor::GetImplementation( Adaptor::Get() ) );
    manager = new InputMethodContextAndroid( nullptr );
  }

  return manager;
}

void InputMethodContextAndroid::Finalize()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::Finalize\n" );
  DisconnectCallbacks();
  DeleteContext();
}

InputMethodContextAndroid::InputMethodContextAndroid( struct android_app* app )
: mApp( app ),
  mRestoreAfterFocusLost( false ),
  mIdleCallbackConnected( false )
{
}

InputMethodContextAndroid::~InputMethodContextAndroid()
{
  Finalize();
}

void InputMethodContextAndroid::Initialize()
{
}

// Callbacks for predicitive text support.
void InputMethodContextAndroid::ConnectCallbacks()
{
}

void InputMethodContextAndroid::DisconnectCallbacks()
{
}

void InputMethodContextAndroid::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void InputMethodContextAndroid::Deactivate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void InputMethodContextAndroid::Reset()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::Reset\n" );
}

ImfContext* InputMethodContextAndroid::GetContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::GetContext\n" );

  return mApp;
}

bool InputMethodContextAndroid::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

void InputMethodContextAndroid::SetRestoreAfterFocusLost( bool toggle )
{
  mRestoreAfterFocusLost = toggle;
}

/**
 * Called when an InputMethodContext Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the InputMethodContext module thinks
 * the user wants to type.
 */
void InputMethodContextAndroid::PreEditChanged( void*, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::PreEditChanged\n" );
}

void InputMethodContextAndroid::CommitReceived( void*, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::CommitReceived\n" );

  if ( Dali::Adaptor::IsAvailable() )
  {
    const std::string keyString( static_cast<char*>( event_info ) );

    Dali::InputMethodContext handle( this );
    Dali::InputMethodContext::EventData eventData( Dali::InputMethodContext::COMMIT, keyString, 0, 0 );
    Dali::InputMethodContext::CallbackData callbackData = mEventSignal.Emit( handle, eventData );

    if( callbackData.update )
    {
      mIMFCursorPosition = static_cast<int>( callbackData.cursorPosition );

      NotifyCursorPosition();
    }
  }
}

/**
 * Called when an InputMethodContext retrieve surround event is received.
 * Here the InputMethodContext module wishes to know the string we are working with and where within the string the cursor is
 * We need to signal the application to tell us this information.
 */
bool InputMethodContextAndroid::RetrieveSurrounding( void* data, ImfContext* imfContext, char** text, int* cursorPosition )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::RetrieveSurrounding\n" );

  Dali::InputMethodContext::EventData imfData( Dali::InputMethodContext::GET_SURROUNDING, std::string(), 0, 0 );
  Dali::InputMethodContext handle( this );
  Dali::InputMethodContext::CallbackData callbackData = mEventSignal.Emit( handle, imfData );

  if( callbackData.update )
  {
    if( text )
    {
      *text = strdup( callbackData.currentText.c_str() );
    }

    if( cursorPosition )
    {
      mIMFCursorPosition = static_cast<int>( callbackData.cursorPosition );
      *cursorPosition = mIMFCursorPosition;
    }
  }

  return true;
}

/**
 * Called when an InputMethodContext delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void InputMethodContextAndroid::DeleteSurrounding( void* data, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::DeleteSurrounding\n" );
}

void InputMethodContextAndroid::NotifyCursorPosition()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::NotifyCursorPosition\n" );
}

void InputMethodContextAndroid::SetCursorPosition( unsigned int cursorPosition )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::SetCursorPosition\n" );

  mIMFCursorPosition = static_cast<int>( cursorPosition );
}

unsigned int InputMethodContextAndroid::GetCursorPosition() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::GetCursorPosition\n" );

  return static_cast<unsigned int>( mIMFCursorPosition );
}

void InputMethodContextAndroid::SetSurroundingText( const std::string& text )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::SetSurroundingText\n" );

  mSurroundingText = text;
}

const std::string& InputMethodContextAndroid::GetSurroundingText() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::GetSurroundingText\n" );

  return mSurroundingText;
}

void InputMethodContextAndroid::NotifyTextInputMultiLine( bool multiLine )
{
}

Dali::InputMethodContext::TextDirection InputMethodContextAndroid::GetTextDirection()
{
  Dali::InputMethodContext::TextDirection direction ( Dali::InputMethodContext::LeftToRight );
  return direction;
}

Rect<int> InputMethodContextAndroid::GetInputMethodArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;
  return Rect<int>(xPos,yPos,width,height);
}

void InputMethodContextAndroid::ApplyOptions( const InputMethodOptions& options )
{
}

void InputMethodContextAndroid::SetInputPanelData( const std::string& data )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::SetInputPanelData\n" );
}

void InputMethodContextAndroid::GetInputPanelData( std::string& data )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::GetInputPanelData\n" );
}

Dali::InputMethodContext::State InputMethodContextAndroid::GetInputPanelState()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::GetInputPanelState\n" );
  return Dali::InputMethodContext::DEFAULT;
}

void InputMethodContextAndroid::SetReturnKeyState( bool visible )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::SetReturnKeyState\n" );
}

void InputMethodContextAndroid::AutoEnableInputPanel( bool enabled )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::AutoEnableInputPanel\n" );
}

void InputMethodContextAndroid::ShowInputPanel()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::ShowInputPanel\n" );
}

void InputMethodContextAndroid::HideInputPanel()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::HideInputPanel\n" );
}

Dali::InputMethodContext::KeyboardType InputMethodContextAndroid::GetKeyboardType()
{
  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

std::string InputMethodContextAndroid::GetInputPanelLocale()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::GetInputPanelLocale\n" );

  std::string locale = "";
  return locale;
}

bool InputMethodContextAndroid::FilterEventKey( const Dali::KeyEvent& keyEvent )
{
  bool eventHandled( false );

  // If a device key then skip ecore_imf_context_filter_event.
  if ( ! KeyLookup::IsDeviceButton( keyEvent.keyPressedName.c_str() ))
  {
    //check whether it's key down or key up event
    if ( keyEvent.state == KeyEvent::Down )
    {
      eventHandled = ProcessEventKeyDown( keyEvent );
    }
    else if ( keyEvent.state == KeyEvent::Up )
    {
      eventHandled = ProcessEventKeyUp( keyEvent );
    }
  }

  return eventHandled;
}

void InputMethodContextAndroid::AllowTextPrediction( bool prediction )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::AllowTextPrediction\n" );
}

bool InputMethodContextAndroid::IsTextPredictionAllowed() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextAndroid::IsTextPredictionAllowed\n" );
  bool prediction = false;
  return prediction;
}

void InputMethodContextAndroid::CreateContext( struct android_app* app )
{

}

void InputMethodContextAndroid::DeleteContext()
{
}

bool InputMethodContextAndroid::ProcessEventKeyDown( const KeyEvent& keyEvent )
{
  bool eventHandled( false );
  return eventHandled;
}

bool InputMethodContextAndroid::ProcessEventKeyUp( const KeyEvent& keyEvent )
{
  bool eventHandled( false );
  return eventHandled;
}

} // Adaptor

} // Internal

} // Dali
