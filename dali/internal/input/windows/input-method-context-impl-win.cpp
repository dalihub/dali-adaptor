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
#include <dali/internal/input/windows/input-method-context-impl-win.h>

// EXTERNAL INCLUDES
//#include <Ecore_IMF.h>
//#include <Ecore_Input.h>
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
// Ecore is littered with C style cast
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
//#include <dali/internal/input/tizen-wayland/ecore-virtual-keyboard.h>

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

// Currently this code is internal to dali/dali/internal/event/text/utf8.h but should be made Public and used from there instead.
size_t Utf8SequenceLength(const unsigned char leadByte)
{
  size_t length = 0;

  if ((leadByte & 0x80) == 0 )          //ASCII character (lead bit zero)
  {
    length = 1;
  }
  else if (( leadByte & 0xe0 ) == 0xc0 ) //110x xxxx
  {
    length = 2;
  }
  else if (( leadByte & 0xf0 ) == 0xe0 ) //1110 xxxx
  {
    length = 3;
  }
  else if (( leadByte & 0xf8 ) == 0xf0 ) //1111 0xxx
  {
    length = 4;
  }

  return length;
}
//
//// Static function calls used by ecore 'c' style callback registration
//void Commit( void *data, Ecore_IMF_Context *imfContext, void *event_info )
//{
//  if ( data )
//  {
//    InputMethodContextWin* inputMethodContext = reinterpret_cast< InputMethodContextWin* > ( data );
//    inputMethodContext->CommitReceived( data, imfContext, event_info );
//  }
//}
//
//void PreEdit( void *data, Ecore_IMF_Context *imfContext, void *event_info )
//{
//  if ( data )
//  {
//    InputMethodContextWin* inputMethodContext = reinterpret_cast< InputMethodContextWin* > ( data );
//    inputMethodContext->PreEditChanged( data, imfContext, event_info );
//  }
//}
//
//Eina_Bool ImfRetrieveSurrounding(void *data, Ecore_IMF_Context *imfContext, char** text, int* cursorPosition )
//{
//  if ( data )
//  {
//    InputMethodContextWin* inputMethodContext = reinterpret_cast< InputMethodContextWin* > ( data );
//    return inputMethodContext->RetrieveSurrounding( data, imfContext, text, cursorPosition );
//  }
//  else
//  {
//    return false;
//  }
//}
//
///**
// * Called when an InputMethodContext delete surrounding event is received.
// * Here we tell the application that it should delete a certain range.
// */
//void ImfDeleteSurrounding( void *data, Ecore_IMF_Context *imfContext, void *event_info )
//{
//  if ( data )
//  {
//    InputMethodContextWin* inputMethodContext = reinterpret_cast< InputMethodContextWin* > ( data );
//    inputMethodContext->DeleteSurrounding( data, imfContext, event_info );
//  }
//}

} // unnamed namespace

InputMethodContextPtr InputMethodContextWin::New()
{
  InputMethodContextPtr manager;

  if ( Adaptor::IsAvailable() )
  {
    // Create instance and register singleton only if the adaptor is available
    Adaptor& adaptorImpl( Adaptor::GetImplementation( Adaptor::Get() ) );
    Any nativeWindow = adaptorImpl.GetNativeWindowHandle();

    // The Win_Window_Handle needs to use the InputMethodContext.
    // Only when the render surface is window, we can get the Win_Window_Handle.
    Win_Window_Handle winWindow( AnyCast<Win_Window_Handle>(nativeWindow) );
    if ( winWindow )
    {
      // If we fail to get Win_Window_Handle, we can't use the InputMethodContext correctly.
      // Thus you have to call "ecore_imf_context_client_window_set" somewhere.
      // In EvasPlugIn, this function is called in EvasPlugin::ConnectEcoreEvent().

      manager = new InputMethodContextWin( winWindow );
    }
    else
    {
      DALI_LOG_ERROR("Failed to get native window handle\n");
    }
  }

  return manager;
}

void InputMethodContextWin::Finalize()
{
  //DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::Finalize\n" );
  //VirtualKeyboard::DisconnectCallbacks( mIMFContext );
  //DisconnectCallbacks();
  //DeleteContext();
}

InputMethodContextWin::InputMethodContextWin( Win_Window_Handle winWindow )
: mWin32Window( winWindow ),
  mIMFCursorPosition( 0 ),
  mSurroundingText(),
  mRestoreAfterFocusLost( false ),
  mIdleCallbackConnected( false )
{
  //ecore_imf_init();
}

InputMethodContextWin::~InputMethodContextWin()
{
  Finalize();
  //ecore_imf_shutdown();
}

void InputMethodContextWin::Initialize()
{
  CreateContext( mWin32Window );
  ConnectCallbacks();
  //VirtualKeyboard::ConnectCallbacks( mIMFContext );
}

void InputMethodContextWin::CreateContext( Win_Window_Handle ecoreXwin )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::CreateContext\n" );

  //const char *contextId = ecore_imf_context_default_id_get();
  //if( contextId )
  //{
  //  mIMFContext = ecore_imf_context_add( contextId );

  //  if( mIMFContext )
  //  {
  //    if( ecoreXwin )
  //    {
  //      ecore_imf_context_client_window_set( mIMFContext, reinterpret_cast<void*>( ecoreXwin ) );
  //    }
  //  }
  //  else
  //  {
  //    DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContext Unable to get IMFContext\n");
  //  }
  //}
  //else
  //{
  //  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContext Unable to get IMFContext\n");
  //}
}

void InputMethodContextWin::DeleteContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::DeleteContext\n" );

  //if ( mIMFContext )
  //{
  //  ecore_imf_context_del( mIMFContext );
  //  mIMFContext = NULL;
  //}
}

// Callbacks for predicitive text support.
void InputMethodContextWin::ConnectCallbacks()
{
  //if ( mIMFContext )
  //{
  //  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::ConnectCallbacks\n" );

  //  ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED,    PreEdit,    this );
  //  ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_COMMIT,             Commit,     this );
  //  ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, ImfDeleteSurrounding, this );

  //  ecore_imf_context_retrieve_surrounding_callback_set( mIMFContext, ImfRetrieveSurrounding, this);
  //}
}

void InputMethodContextWin::DisconnectCallbacks()
{
  //if ( mIMFContext )
  //{
  //  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::DisconnectCallbacks\n" );

  //  ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED,    PreEdit );
  //  ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_COMMIT,             Commit );
  //  ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, ImfDeleteSurrounding );

  //  // We do not need to unset the retrieve surrounding callback.
  //}
}

void InputMethodContextWin::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;

  //if ( mIMFContext )
  //{
  //  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::Activate\n" );

  //  ecore_imf_context_focus_in( mIMFContext );

  //  // emit keyboard activated signal
  //  Dali::InputMethodContext handle( this );
  //  mActivatedSignal.Emit( handle );
  //}
}

void InputMethodContextWin::Deactivate()
{
  //if( mIMFContext )
  //{
  //  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::Deactivate\n" );

  //  Reset();
  //  ecore_imf_context_focus_out( mIMFContext );
  //}

  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void InputMethodContextWin::Reset()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::Reset\n" );

  //if ( mIMFContext )
  //{
  //  ecore_imf_context_reset( mIMFContext );
  //}
}

ImfContext* InputMethodContextWin::GetContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::GetContext\n" );

  //return mIMFContext;
  return NULL;
}

bool InputMethodContextWin::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

void InputMethodContextWin::SetRestoreAfterFocusLost( bool toggle )
{
  mRestoreAfterFocusLost = toggle;
}

/**
 * Called when an InputMethodContext Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the InputMethodContext module thinks
 * the user wants to type.
 */
void InputMethodContextWin::PreEditChanged( void*, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::PreEditChanged\n" );
//  auto context = reinterpret_cast<Ecore_IMF_Context*>(imfContext);
//
//  char* preEditString( NULL );
//  int cursorPosition( 0 );
//  Eina_List* attrs = NULL;
//  Eina_List* l = NULL;
//
//  Ecore_IMF_Preedit_Attr* attr;
//
//  // Retrieves attributes as well as the string the cursor position offset from start of pre-edit string.
//  // the attributes (attrs) is used in languages that use the soft arrows keys to insert characters into a current pre-edit string.
//  ecore_imf_context_preedit_string_with_attributes_get( context, &preEditString, &attrs, &cursorPosition );
//
//  if ( attrs )
//  {
//    // iterate through the list of attributes getting the type, start and end position.
//    for ( l = attrs, (attr =  static_cast<Ecore_IMF_Preedit_Attr*>( eina_list_data_get(l) ) ); l; l = eina_list_next(l), ( attr = static_cast<Ecore_IMF_Preedit_Attr*>( eina_list_data_get(l) ) ))
//    {
//#ifdef DALI_PROFILE_UBUNTU
//      if ( attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB3 ) // (Ecore_IMF)
//#else // DALI_PROFILE_UBUNTU
//      if ( attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB4 ) // (Ecore_IMF)
//#endif // DALI_PROFILE_UBUNTU
//      {
//        // check first byte so know how many bytes a character is represented by as keyboard returns cursor position in bytes. Which is different for some languages.
//
//        size_t visualCharacterIndex = 0;
//        size_t byteIndex = 0;
//
//        // iterate through null terminated string checking each character's position against the given byte position ( attr->end_index ).
//        const char leadByte = preEditString[byteIndex];
//        while( leadByte != '\0' )
//        {
//          // attr->end_index is provided as a byte position not character and we need to know the character position.
//          const size_t currentSequenceLength = Utf8SequenceLength( leadByte ); // returns number of bytes used to represent character.
//          if ( byteIndex == attr->end_index )
//          {
//            cursorPosition = visualCharacterIndex;
//            break;
//            // end loop as found cursor position that matches byte position
//          }
//          else
//          {
//            byteIndex += currentSequenceLength; // jump to next character
//            visualCharacterIndex++;  // increment character count so we know our position for when we get a match
//          }
//
//          DALI_ASSERT_DEBUG( visualCharacterIndex < strlen( preEditString ));
//        }
//      }
//    }
//  }

  //if ( Dali::Adaptor::IsAvailable() )
  //{
  //  Dali::InputMethodContext handle( this );
  //  Dali::InputMethodContext::EventData eventData( Dali::InputMethodContext::PRE_EDIT, preEditString, cursorPosition, 0 );
  //  Dali::InputMethodContext::CallbackData callbackData = mEventSignal.Emit( handle, eventData );

  //  if( callbackData.update )
  //  {
  //    mIMFCursorPosition = static_cast<int>( callbackData.cursorPosition );

  //    NotifyCursorPosition();
  //  }

  //  if( callbackData.preeditResetRequired )
  //  {
  //    Reset();
  //  }
  //}
  //free( preEditString );
}

void InputMethodContextWin::CommitReceived( void*, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::CommitReceived\n" );

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
bool InputMethodContextWin::RetrieveSurrounding( void* data, ImfContext* imfContext, char** text, int* cursorPosition )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::RetrieveSurrounding\n" );

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

  //return EINA_TRUE;
  return true;
}

/**
 * Called when an InputMethodContext delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void InputMethodContextWin::DeleteSurrounding( void* data, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::DeleteSurrounding\n" );

  //if( Dali::Adaptor::IsAvailable() )
  //{
  //  Ecore_IMF_Event_Delete_Surrounding* deleteSurroundingEvent = static_cast<Ecore_IMF_Event_Delete_Surrounding*>( event_info );

  //  Dali::InputMethodContext::EventData imfData( Dali::InputMethodContext::DELETE_SURROUNDING, std::string(), deleteSurroundingEvent->offset, deleteSurroundingEvent->n_chars );
  //  Dali::InputMethodContext handle( this );
  //  Dali::InputMethodContext::CallbackData callbackData = mEventSignal.Emit( handle, imfData );

  //  if( callbackData.update )
  //  {
  //    mIMFCursorPosition = static_cast<int>( callbackData.cursorPosition );

  //    NotifyCursorPosition();
  //  }
  //}
}

void InputMethodContextWin::NotifyCursorPosition()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::NotifyCursorPosition\n" );

  //if( mIMFContext )
  //{
  //  ecore_imf_context_cursor_position_set( mIMFContext, mIMFCursorPosition );
  //}
}

void InputMethodContextWin::SetCursorPosition( unsigned int cursorPosition )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::SetCursorPosition\n" );

  mIMFCursorPosition = static_cast<int>( cursorPosition );
}

unsigned int InputMethodContextWin::GetCursorPosition() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::GetCursorPosition\n" );

  return static_cast<unsigned int>( mIMFCursorPosition );
}

void InputMethodContextWin::SetSurroundingText( const std::string& text )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::SetSurroundingText\n" );

  mSurroundingText = text;
}

const std::string& InputMethodContextWin::GetSurroundingText() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::GetSurroundingText\n" );

  return mSurroundingText;
}

void InputMethodContextWin::NotifyTextInputMultiLine( bool multiLine )
{
}

Dali::InputMethodContext::TextDirection InputMethodContextWin::GetTextDirection()
{
  Dali::InputMethodContext::TextDirection direction ( Dali::InputMethodContext::LeftToRight );

    //if ( mIMFContext )
    //{
    //  char* locale( NULL );
    //  ecore_imf_context_input_panel_language_locale_get( mIMFContext, &locale );

    //  if ( locale )
    //  {
    //    direction = static_cast< Dali::InputMethodContext::TextDirection >( Locale::GetDirection( std::string( locale ) ) );
    //    free( locale );
    //  }
    //}

  return direction;
}

Rect<int> InputMethodContextWin::GetInputMethodArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;

  //if( mIMFContext )
  //{
  //  ecore_imf_context_input_panel_geometry_get( mIMFContext, &xPos, &yPos, &width, &height );
  //}
  //else
  //{
  //  DALI_LOG_WARNING("VKB Unable to get InputMethodContext Context so GetSize unavailable\n");
  //}

  return Rect<int>(xPos,yPos,width,height);
}

void InputMethodContextWin::ApplyOptions( const InputMethodOptions& options )
{
  using namespace Dali::InputMethod::Category;

  int index;

  //if (mIMFContext == NULL)
  //{
  //  DALI_LOG_WARNING("VKB Unable to excute ApplyOptions with Null ImfContext\n");
  //  return;
  //}

  if ( mOptions.CompareAndSet(PANEL_LAYOUT, options, index) )
  {
  }
  if ( mOptions.CompareAndSet(BUTTON_ACTION, options, index) )
  {
  }
  if ( mOptions.CompareAndSet(AUTO_CAPITALIZE, options, index) )
  {
  }
  if ( mOptions.CompareAndSet(VARIATION, options, index) )
  {
  }
}

void InputMethodContextWin::SetInputPanelData( const std::string& data )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelData\n" );

  //if( mIMFContext )
  //{
  //  int length = data.length();
  //  ecore_imf_context_input_panel_imdata_set( mIMFContext, data.c_str(), length );
  //}
}

void InputMethodContextWin::GetInputPanelData( std::string& data )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelData\n" );

  //if( mIMFContext )
  //{
  //  int length = 4096; // The max length is 4096 bytes
  //  Dali::Vector< char > buffer;
  //  buffer.Resize( length );
  //  ecore_imf_context_input_panel_imdata_get( mIMFContext, &buffer[0], &length );
  //  data = std::string( buffer.Begin(), buffer.End() );
  //}
}

Dali::InputMethodContext::State InputMethodContextWin::GetInputPanelState()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelState\n" );

  //if( mIMFContext )
  //{
  //  int value;
  //  value = ecore_imf_context_input_panel_state_get( mIMFContext );

  //  switch (value)
  //  {
  //    case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
  //    {
  //      return Dali::InputMethodContext::SHOW;
  //      break;
  //    }

  //    case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
  //    {
  //      return Dali::InputMethodContext::HIDE;
  //      break;
  //    }

  //    case ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
  //    {
  //      return Dali::InputMethodContext::WILL_SHOW;
  //      break;
  //    }

  //    default:
  //    {
  //      return Dali::InputMethodContext::DEFAULT;
  //    }
  //  }
  //}
  return Dali::InputMethodContext::DEFAULT;
}

void InputMethodContextWin::SetReturnKeyState( bool visible )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::SetReturnKeyState\n" );

  //if( mIMFContext )
  //{
  //  ecore_imf_context_input_panel_return_key_disabled_set( mIMFContext, !visible );
  //}
}

void InputMethodContextWin::AutoEnableInputPanel( bool enabled )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::AutoEnableInputPanel\n" );

  //if( mIMFContext )
  //{
  //  ecore_imf_context_input_panel_enabled_set( mIMFContext, enabled );
  //}
}

void InputMethodContextWin::ShowInputPanel()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::ShowInputPanel\n" );

  //if( mIMFContext )
  //{
  //  ecore_imf_context_input_panel_show( mIMFContext );
  //}
}

void InputMethodContextWin::HideInputPanel()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::HideInputPanel\n" );

  //if( mIMFContext )
  //{
  //  ecore_imf_context_input_panel_hide( mIMFContext );
  //}
}

Dali::InputMethodContext::KeyboardType InputMethodContextWin::GetKeyboardType()
{
  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

std::string InputMethodContextWin::GetInputPanelLocale()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelLocale\n" );

  std::string locale = "";

  //if( mIMFContext )
  //{
  //  char* value = NULL;
  //  ecore_imf_context_input_panel_language_locale_get( mIMFContext, &value );

  //  if( value )
  //  {
  //    std::string valueCopy( value );
  //    locale = valueCopy;

  //    // The locale string retrieved must be freed with free().
  //    free( value );
  //  }
  //}
  return locale;
}

bool InputMethodContextWin::FilterEventKey( const Dali::KeyEvent& keyEvent )
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

bool InputMethodContextWin::ProcessEventKeyDown( const KeyEvent& keyEvent )
{
  bool eventHandled( false );
//  if ( mIMFContext )
//  {
//    // We're consuming key down event so we have to pass to InputMethodContext so that it can parse it as well.
//    Ecore_IMF_Event_Key_Down ecoreKeyDownEvent;
//    ecoreKeyDownEvent.keyname = keyEvent.keyPressedName.c_str();
//    ecoreKeyDownEvent.key = keyEvent.keyPressedName.c_str();
//    ecoreKeyDownEvent.string = keyEvent.keyPressed.c_str();
//    ecoreKeyDownEvent.compose = keyEvent.GetCompose().c_str();
//    ecoreKeyDownEvent.timestamp = keyEvent.time;
//    ecoreKeyDownEvent.modifiers = EcoreInputModifierToEcoreIMFModifier( keyEvent.keyModifier );
//    ecoreKeyDownEvent.locks = EcoreInputModifierToEcoreIMFLock( keyEvent.keyModifier );
//#ifdef ECORE_IMF_1_13
//    ecoreKeyDownEvent.dev_name  = "";
//    ecoreKeyDownEvent.dev_class = ECORE_IMF_DEVICE_CLASS_KEYBOARD;
//    ecoreKeyDownEvent.dev_subclass = ECORE_IMF_DEVICE_SUBCLASS_NONE;
//#endif // ECORE_IMF_1_13
//
//    // If the device is IME and the focused key is the direction keys, then we should send a key event to move a key cursor.
//    if ((keyEvent.GetDeviceName() == "ime") && ((!strncmp(keyEvent.keyPressedName.c_str(), "Left", 4)) ||
//                                   (!strncmp(keyEvent.keyPressedName.c_str(), "Right", 5)) ||
//                                   (!strncmp(keyEvent.keyPressedName.c_str(), "Up", 2)) ||
//                                   (!strncmp(keyEvent.keyPressedName.c_str(), "Down", 4))))
//    {
//      eventHandled = 0;
//    }
//    else
//    {
//      eventHandled = ecore_imf_context_filter_event(mIMFContext,
//                                                    ECORE_IMF_EVENT_KEY_DOWN,
//                                                    (Ecore_IMF_Event *) &ecoreKeyDownEvent);
//    }
//
//    // If the event has not been handled by InputMethodContext then check if we should reset our IMFcontext
//    if (!eventHandled)
//    {
//      if (!strcmp(keyEvent.keyPressedName.c_str(), "Escape") ||
//          !strcmp(keyEvent.keyPressedName.c_str(), "Return") ||
//          !strcmp(keyEvent.keyPressedName.c_str(), "KP_Enter"))
//      {
//        ecore_imf_context_reset(mIMFContext);
//      }
//    }
//  }
  return eventHandled;
}

bool InputMethodContextWin::ProcessEventKeyUp( const KeyEvent& keyEvent )
{
  bool eventHandled( false );
//  if( mIMFContext )
//  {
//    // We're consuming key up event so we have to pass to InputMethodContext so that it can parse it as well.
//    Ecore_IMF_Event_Key_Up ecoreKeyUpEvent;
//    ecoreKeyUpEvent.keyname = keyEvent.keyPressedName.c_str();
//    ecoreKeyUpEvent.key = keyEvent.keyPressedName.c_str();
//    ecoreKeyUpEvent.string = keyEvent.keyPressed.c_str();
//    ecoreKeyUpEvent.compose = keyEvent.GetCompose().c_str();
//    ecoreKeyUpEvent.timestamp = keyEvent.time;
//    ecoreKeyUpEvent.modifiers = EcoreInputModifierToEcoreIMFModifier( keyEvent.keyModifier );
//    ecoreKeyUpEvent.locks = EcoreInputModifierToEcoreIMFLock( keyEvent.keyModifier );
//#ifdef ECORE_IMF_1_13
//    ecoreKeyUpEvent.dev_name  = "";
//#endif // ECORE_IMF_1_13
//
//    eventHandled = ecore_imf_context_filter_event(mIMFContext,
//                                                  ECORE_IMF_EVENT_KEY_UP,
//                                                  (Ecore_IMF_Event *) &ecoreKeyUpEvent);
//  }
  return eventHandled;
}

//Ecore_IMF_Keyboard_Modifiers InputMethodContextWin::EcoreInputModifierToEcoreIMFModifier( unsigned int ecoreModifier )
//{
//  unsigned int modifier( ECORE_IMF_KEYBOARD_MODIFIER_NONE );  // If no other matches returns NONE.
//
//  if ( ecoreModifier & ECORE_EVENT_MODIFIER_SHIFT )  // enums from ecore_input/Ecore_Input.h
//  {
//    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_SHIFT;  // enums from ecore_imf/ecore_imf.h
//  }
//
//  if ( ecoreModifier & ECORE_EVENT_MODIFIER_ALT )
//  {
//    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_ALT;
//  }
//
//  if ( ecoreModifier & ECORE_EVENT_MODIFIER_CTRL )
//  {
//    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_CTRL;
//  }
//
//  if ( ecoreModifier & ECORE_EVENT_MODIFIER_WIN )
//  {
//    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_WIN;
//  }
//
//  if ( ecoreModifier & ECORE_EVENT_MODIFIER_ALTGR )
//  {
//    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_ALTGR;
//  }
//
//  return static_cast<Ecore_IMF_Keyboard_Modifiers>( modifier );
//}
//
//Ecore_IMF_Keyboard_Locks InputMethodContextWin::EcoreInputModifierToEcoreIMFLock( unsigned int modifier )
//{
//    unsigned int lock( ECORE_IMF_KEYBOARD_LOCK_NONE ); // If no other matches, returns NONE.
//
//    if( modifier & ECORE_EVENT_LOCK_NUM )
//    {
//      lock |= ECORE_IMF_KEYBOARD_LOCK_NUM; // Num lock is active.
//    }
//
//    if( modifier & ECORE_EVENT_LOCK_CAPS )
//    {
//      lock |= ECORE_IMF_KEYBOARD_LOCK_CAPS; // Caps lock is active.
//    }
//
//    if( modifier & ECORE_EVENT_LOCK_SCROLL )
//    {
//      lock |= ECORE_IMF_KEYBOARD_LOCK_SCROLL; // Scroll lock is active.
//    }
//
//    return static_cast<Ecore_IMF_Keyboard_Locks>( lock );
//}

} // Adaptor

} // Internal

} // Dali

#pragma GCC diagnostic pop
