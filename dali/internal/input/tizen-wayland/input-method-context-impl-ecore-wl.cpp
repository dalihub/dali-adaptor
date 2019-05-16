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

#include <dali/internal/input/tizen-wayland/input-method-context-impl-ecore-wl.h>

// EXTERNAL INCLUDES
#include <Ecore_Input.h>

#include <dali/public-api/events/key-event.h>
#include <dali/public-api/adaptor-framework/key.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor.h>
#include <dali/internal/system/common/locale-utils.h>
#include <dali/internal/system/common/singleton-service-impl.h>
#include <dali/public-api/adaptor-framework/input-method.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#define TOKEN_STRING(x) #x

Ecore_IMF_Input_Panel_Layout panelLayoutMap[] =
{
   ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL,
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBER,
   ECORE_IMF_INPUT_PANEL_LAYOUT_EMAIL,
   ECORE_IMF_INPUT_PANEL_LAYOUT_URL,
   ECORE_IMF_INPUT_PANEL_LAYOUT_PHONENUMBER,
   ECORE_IMF_INPUT_PANEL_LAYOUT_IP,
   ECORE_IMF_INPUT_PANEL_LAYOUT_MONTH,
   ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY,
   ECORE_IMF_INPUT_PANEL_LAYOUT_HEX,
   ECORE_IMF_INPUT_PANEL_LAYOUT_TERMINAL,
   ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD,
   ECORE_IMF_INPUT_PANEL_LAYOUT_DATETIME,
   ECORE_IMF_INPUT_PANEL_LAYOUT_EMOTICON,
   ECORE_IMF_INPUT_PANEL_LAYOUT_VOICE
};

Ecore_IMF_Autocapital_Type autoCapitalMap[] =
{
   ECORE_IMF_AUTOCAPITAL_TYPE_NONE,
   ECORE_IMF_AUTOCAPITAL_TYPE_WORD,
   ECORE_IMF_AUTOCAPITAL_TYPE_SENTENCE,
   ECORE_IMF_AUTOCAPITAL_TYPE_ALLCHARACTER,
};

Ecore_IMF_Input_Panel_Return_Key_Type returnKeyTypeMap[] =
{
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT,
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE,
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_GO,
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_JOIN,
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN,
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_NEXT,
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH,
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND,
   ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN
};

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

// Static function calls used by ecore 'c' style callback registration
void Commit( void *data, Ecore_IMF_Context *imfContext, void *event_info )
{
  if ( data )
  {
    InputMethodContextEcoreWl* inputMethodContext = reinterpret_cast< InputMethodContextEcoreWl* > ( data );
    inputMethodContext->CommitReceived( data, imfContext, event_info );
  }
}

void PreEdit( void *data, Ecore_IMF_Context *imfContext, void *event_info )
{
  if ( data )
  {
    InputMethodContextEcoreWl* inputMethodContext = reinterpret_cast< InputMethodContextEcoreWl* > ( data );
    inputMethodContext->PreEditChanged( data, imfContext, event_info );
  }
}

Eina_Bool ImfRetrieveSurrounding(void *data, Ecore_IMF_Context *imfContext, char** text, int* cursorPosition )
{
  if ( data )
  {
    InputMethodContextEcoreWl* inputMethodContext = reinterpret_cast< InputMethodContextEcoreWl* > ( data );
    return inputMethodContext->RetrieveSurrounding( data, imfContext, text, cursorPosition );
  }
  else
  {
    return false;
  }
}

void InputPanelStateChangeCallback( void* data, Ecore_IMF_Context* context, int value )
{
  if (!data)
  {
    return;
  }
  InputMethodContextEcoreWl* inputMethodContext = reinterpret_cast< InputMethodContextEcoreWl* > ( data );
  switch (value)
  {
    case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
    {
      inputMethodContext->StatusChangedSignal().Emit( true );
      break;
    }

    case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
    {
      inputMethodContext->StatusChangedSignal().Emit( false );
      break;
    }

    case ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
    default:
    {
      // Do nothing
      break;
    }
  }
}

void InputPanelLanguageChangeCallback( void* data, Ecore_IMF_Context* context, int value )
{
  if (!data)
  {
    return;
  }
  InputMethodContextEcoreWl* inputMethodContext = reinterpret_cast< InputMethodContextEcoreWl* > ( data );
  // Emit the signal that the language has changed
  inputMethodContext->LanguageChangedSignal().Emit(value);
}

void InputPanelGeometryChangedCallback ( void *data, Ecore_IMF_Context *context, int value )
{
  if (!data)
  {
    return;
  }
  InputMethodContextEcoreWl* inputMethodContext = reinterpret_cast< InputMethodContextEcoreWl* > ( data );
  // Emit signal that the keyboard is resized
  inputMethodContext->ResizedSignal().Emit(value);
}

void InputPanelKeyboardTypeChangedCallback( void *data, Ecore_IMF_Context *context, int value )
{
  if( !data )
  {
    return;
  }

  InputMethodContextEcoreWl* inputMethodContext = reinterpret_cast< InputMethodContextEcoreWl* > ( data );
  switch (value)
  {
    case ECORE_IMF_INPUT_PANEL_SW_KEYBOARD_MODE:
    {
      // Emit Signal that the keyboard type is changed to Software Keyboard
      inputMethodContext->KeyboardTypeChangedSignal().Emit( Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD );
      break;
    }
    case ECORE_IMF_INPUT_PANEL_HW_KEYBOARD_MODE:
    {
      // Emit Signal that the keyboard type is changed to Hardware Keyboard
      inputMethodContext->KeyboardTypeChangedSignal().Emit( Dali::InputMethodContext::KeyboardType::HARDWARE_KEYBOARD );
      break;
    }
  }
}

/**
 * Called when an IMF delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void ImfDeleteSurrounding( void *data, Ecore_IMF_Context *imfContext, void *event_info )
{
  if ( data )
  {
    InputMethodContextEcoreWl* inputMethodContext = reinterpret_cast< InputMethodContextEcoreWl* > ( data );
    inputMethodContext->DeleteSurrounding( data, imfContext, event_info );
  }
}

/**
 * Called when the input method sends a private command.
 */
void PrivateCommand( void *data, Ecore_IMF_Context *imfContext, void *event_info )
{
  if ( data )
  {
    InputMethodContextEcoreWl* inputMethodContext = reinterpret_cast< InputMethodContextEcoreWl* > ( data );
    inputMethodContext->SendPrivateCommand( data, imfContext, event_info );
  }
}

BaseHandle Create()
{
  return Dali::InputMethodContext::New();
}

Dali::TypeRegistration type( typeid(Dali::InputMethodContext), typeid(Dali::BaseHandle), Create );

} // unnamed namespace

InputMethodContextPtr InputMethodContextEcoreWl::New()
{
  InputMethodContextPtr inputMethodContext;

  // Create instance only if the adaptor is available
  if ( Dali::Adaptor::IsAvailable() )
  {
    Any nativeWindow = Dali::Adaptor::Get().GetNativeWindowHandle();

    // The window needs to use the InputMethodContext.
    // Only when the render surface is window, we can get the window.
    if( !nativeWindow.Empty() )
    {
      inputMethodContext = new InputMethodContextEcoreWl();
    }
    else
    {
      DALI_LOG_ERROR("Failed to get native window handle, can't create InputMethodContext instance.\n");
    }
  }
  return inputMethodContext;
}

void InputMethodContextEcoreWl::Finalize()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContext::Finalize\n" );

  DisconnectCallbacks();
  DeleteContext();
}

InputMethodContextEcoreWl::InputMethodContextEcoreWl()
: mIMFContext(),
  mIMFCursorPosition( 0 ),
  mSurroundingText(),
  mRestoreAfterFocusLost( false ),
  mIdleCallbackConnected( false )
{
  ecore_imf_init();
}

InputMethodContextEcoreWl::~InputMethodContextEcoreWl()
{
  Finalize();
  ecore_imf_shutdown();
}

void InputMethodContextEcoreWl::Initialize()
{
  CreateContext();
  ConnectCallbacks();
}

void InputMethodContextEcoreWl::CreateContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContext::CreateContext\n" );

  const char *contextId = ecore_imf_context_default_id_get();
  if( contextId )
  {
    mIMFContext = ecore_imf_context_add( contextId );

    if( mIMFContext )
    {
      // If we fail to get window id, we can't use the InputMethodContext correctly.
      // Thus you have to call "ecore_imf_context_client_window_set" somewhere.
      // In EvasPlugIn, this function is called in EvasPlugin::ConnectEcoreEvent().
      Dali::RenderSurfaceInterface& renderSurface = Dali::Adaptor::Get().GetSurface();
      WindowRenderSurface& windowRenderSurface = static_cast< WindowRenderSurface& >( renderSurface );

      int windowId = windowRenderSurface.GetNativeWindowId();
      if( windowId != 0 )
      {
        ecore_imf_context_client_window_set( mIMFContext, reinterpret_cast< void* >( windowId ) );
      }
    }
    else
    {
      DALI_LOG_WARNING("InputMethodContext Unable to get IMFContext\n");
    }
  }
  else
  {
    DALI_LOG_WARNING("InputMethodContext Unable to get IMFContext\n");
  }
}

void InputMethodContextEcoreWl::DeleteContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::DeleteContext\n" );

  if ( mIMFContext )
  {
    ecore_imf_context_del( mIMFContext );
    mIMFContext = NULL;
  }
}

// Callbacks for predicitive text support.
void InputMethodContextEcoreWl::ConnectCallbacks()
{
  if ( mIMFContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::ConnectCallbacks\n" );

    ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED,      PreEdit,    this );
    ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_COMMIT,               Commit,     this );
    ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING,   ImfDeleteSurrounding, this );
    ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, PrivateCommand, this );

    ecore_imf_context_input_panel_event_callback_add( mIMFContext, ECORE_IMF_INPUT_PANEL_STATE_EVENT,    InputPanelStateChangeCallback, this );
    ecore_imf_context_input_panel_event_callback_add( mIMFContext, ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT, InputPanelLanguageChangeCallback, this );
    ecore_imf_context_input_panel_event_callback_add( mIMFContext, ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT, InputPanelGeometryChangedCallback, this );
    ecore_imf_context_input_panel_event_callback_add( mIMFContext, ECORE_IMF_INPUT_PANEL_KEYBOARD_MODE_EVENT, InputPanelKeyboardTypeChangedCallback, this );

    ecore_imf_context_retrieve_surrounding_callback_set( mIMFContext, ImfRetrieveSurrounding, this);
  }
}

void InputMethodContextEcoreWl::DisconnectCallbacks()
{
  if ( mIMFContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::DisconnectCallbacks\n" );

    ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED,      PreEdit );
    ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_COMMIT,               Commit );
    ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING,   ImfDeleteSurrounding );
    ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, PrivateCommand );

    ecore_imf_context_input_panel_event_callback_del( mIMFContext, ECORE_IMF_INPUT_PANEL_STATE_EVENT,    InputPanelStateChangeCallback     );
    ecore_imf_context_input_panel_event_callback_del( mIMFContext, ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT, InputPanelLanguageChangeCallback  );
    ecore_imf_context_input_panel_event_callback_del( mIMFContext, ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT, InputPanelGeometryChangedCallback );
    ecore_imf_context_input_panel_event_callback_del( mIMFContext, ECORE_IMF_INPUT_PANEL_KEYBOARD_MODE_EVENT, InputPanelKeyboardTypeChangedCallback );

    // We do not need to unset the retrieve surrounding callback.
  }
}

void InputMethodContextEcoreWl::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;

  if ( mIMFContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::Activate\n" );

    ecore_imf_context_focus_in( mIMFContext );

    // emit keyboard activated signal
    Dali::InputMethodContext handle( this );
    mActivatedSignal.Emit( handle );
  }
}

void InputMethodContextEcoreWl::Deactivate()
{
  if( mIMFContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::Deactivate\n" );

    Reset();
    ecore_imf_context_focus_out( mIMFContext );
  }

  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void InputMethodContextEcoreWl::Reset()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::Reset\n" );

  if ( mIMFContext )
  {
    ecore_imf_context_reset( mIMFContext );
  }
}

ImfContext* InputMethodContextEcoreWl::GetContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetContext\n" );

  return mIMFContext;
}

bool InputMethodContextEcoreWl::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

void InputMethodContextEcoreWl::SetRestoreAfterFocusLost( bool toggle )
{
  mRestoreAfterFocusLost = toggle;
}

/**
 * Called when an InputMethodContext Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the InputMethodContext module thinks
 * the user wants to type.
 */
void InputMethodContextEcoreWl::PreEditChanged( void*, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::PreEditChanged\n" );
  auto context = reinterpret_cast<Ecore_IMF_Context*>(imfContext);

  char* preEditString( NULL );
  int cursorPosition( 0 );
  Eina_List* attrs = NULL;
  Eina_List* l = NULL;

  Ecore_IMF_Preedit_Attr* attr;

  // Retrieves attributes as well as the string the cursor position offset from start of pre-edit string.
  // the attributes (attrs) is used in languages that use the soft arrows keys to insert characters into a current pre-edit string.
  ecore_imf_context_preedit_string_with_attributes_get( context, &preEditString, &attrs, &cursorPosition );

  if ( attrs )
  {
    // iterate through the list of attributes getting the type, start and end position.
    for ( l = attrs, (attr =  static_cast<Ecore_IMF_Preedit_Attr*>( eina_list_data_get(l) ) ); l; l = eina_list_next(l), ( attr = static_cast<Ecore_IMF_Preedit_Attr*>( eina_list_data_get(l) ) ))
    {
#ifdef DALI_PROFILE_UBUNTU
      if ( attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB3 ) // (Ecore_IMF)
#else // DALI_PROFILE_UBUNTU
      if ( attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB4 ) // (Ecore_IMF)
#endif // DALI_PROFILE_UBUNTU
      {
        // check first byte so know how many bytes a character is represented by as keyboard returns cursor position in bytes. Which is different for some languages.

        size_t visualCharacterIndex = 0;
        size_t byteIndex = 0;

        // iterate through null terminated string checking each character's position against the given byte position ( attr->end_index ).
        const char leadByte = preEditString[byteIndex];
        while( leadByte != '\0' )
        {
          // attr->end_index is provided as a byte position not character and we need to know the character position.
          const size_t currentSequenceLength = Utf8SequenceLength( leadByte ); // returns number of bytes used to represent character.
          if ( byteIndex == attr->end_index )
          {
            cursorPosition = visualCharacterIndex;
            break;
            // end loop as found cursor position that matches byte position
          }
          else
          {
            byteIndex += currentSequenceLength; // jump to next character
            visualCharacterIndex++;  // increment character count so we know our position for when we get a match
          }

          DALI_ASSERT_DEBUG( visualCharacterIndex < strlen( preEditString ));
        }
      }
    }
  }

  if ( Dali::Adaptor::IsAvailable() )
  {
    Dali::InputMethodContext handle( this );
    Dali::InputMethodContext::EventData eventData( Dali::InputMethodContext::PRE_EDIT, preEditString, cursorPosition, 0 );
    Dali::InputMethodContext::CallbackData callbackData = mEventSignal.Emit( handle, eventData );

    if ( callbackData.update )
    {
      SetCursorPosition( callbackData.cursorPosition );
      SetSurroundingText( callbackData.currentText );

      NotifyCursorPosition();
    }

    if ( callbackData.preeditResetRequired )
    {
      Reset();
    }
  }
  free( preEditString );
}

void InputMethodContextEcoreWl::CommitReceived( void*, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::CommitReceived\n" );

  if ( Dali::Adaptor::IsAvailable() )
  {
    const std::string keyString( static_cast<char*>( event_info ) );

    Dali::InputMethodContext handle( this );
    Dali::InputMethodContext::EventData eventData( Dali::InputMethodContext::COMMIT, keyString, 0, 0 );
    Dali::InputMethodContext::CallbackData callbackData = mEventSignal.Emit( handle, eventData );

    if( callbackData.update )
    {
      SetCursorPosition( callbackData.cursorPosition );
      SetSurroundingText( callbackData.currentText );

      NotifyCursorPosition();
    }
  }
}

/**
 * Called when an InputMethodContext retrieve surround event is received.
 * Here the InputMethodContext module wishes to know the string we are working with and where within the string the cursor is
 * We need to signal the application to tell us this information.
 */
bool InputMethodContextEcoreWl::RetrieveSurrounding( void* data, ImfContext* imfContext, char** text, int* cursorPosition )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::RetrieveSurrounding\n" );

  Dali::InputMethodContext::EventData imfData( Dali::InputMethodContext::GET_SURROUNDING, std::string(), 0, 0 );
  Dali::InputMethodContext handle( this );
  Dali::InputMethodContext::CallbackData callbackData = mEventSignal.Emit( handle, imfData );

  if( callbackData.update )
  {
    if( text )
    {
      // The memory allocated by strdup() can be freed by ecore_imf_context_surrounding_get() internally.
      *text = strdup( callbackData.currentText.c_str() );
    }

    if( cursorPosition )
    {
      mIMFCursorPosition = static_cast<int>( callbackData.cursorPosition );
      *cursorPosition = mIMFCursorPosition;
    }
  }

  return EINA_TRUE;
}

/**
 * Called when an InputMethodContext delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void InputMethodContextEcoreWl::DeleteSurrounding( void* data, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::DeleteSurrounding\n" );

  if( Dali::Adaptor::IsAvailable() )
  {
    Ecore_IMF_Event_Delete_Surrounding* deleteSurroundingEvent = static_cast<Ecore_IMF_Event_Delete_Surrounding*>( event_info );

    Dali::InputMethodContext::EventData imfData( Dali::InputMethodContext::DELETE_SURROUNDING, std::string(), deleteSurroundingEvent->offset, deleteSurroundingEvent->n_chars );
    Dali::InputMethodContext handle( this );
    mEventSignal.Emit( handle, imfData );
  }
}

/**
 * Called when the input method sends a private command.
 */
void InputMethodContextEcoreWl::SendPrivateCommand( void* data, ImfContext* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::SendPrivateCommand\n" );

  if( Dali::Adaptor::IsAvailable() )
  {
    const char* privateCommandSendEvent = static_cast<const char*>( event_info );

    Dali::InputMethodContext::EventData imfData( Dali::InputMethodContext::PRIVATE_COMMAND, privateCommandSendEvent, 0, 0 );
    Dali::InputMethodContext handle( this );
    mEventSignal.Emit( handle, imfData );
  }
}

void InputMethodContextEcoreWl::NotifyCursorPosition()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::NotifyCursorPosition\n" );

  if( mIMFContext )
  {
    ecore_imf_context_cursor_position_set( mIMFContext, mIMFCursorPosition );
  }
}

void InputMethodContextEcoreWl::SetCursorPosition( unsigned int cursorPosition )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetCursorPosition\n" );

  mIMFCursorPosition = static_cast<int>( cursorPosition );
}

unsigned int InputMethodContextEcoreWl::GetCursorPosition() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetCursorPosition\n" );

  return static_cast<unsigned int>( mIMFCursorPosition );
}

void InputMethodContextEcoreWl::SetSurroundingText( const std::string& text )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetSurroundingText\n" );

  mSurroundingText = text;
}

const std::string& InputMethodContextEcoreWl::GetSurroundingText() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetSurroundingText\n" );

  return mSurroundingText;
}

void InputMethodContextEcoreWl::NotifyTextInputMultiLine( bool multiLine )
{
  Ecore_IMF_Input_Hints currentHint = ecore_imf_context_input_hint_get(mIMFContext);
  ecore_imf_context_input_hint_set( mIMFContext,
                                    static_cast< Ecore_IMF_Input_Hints >( multiLine ?
                                      (currentHint | ECORE_IMF_INPUT_HINT_MULTILINE) :
                                      (currentHint & ~ECORE_IMF_INPUT_HINT_MULTILINE)));
}

Dali::InputMethodContext::TextDirection InputMethodContextEcoreWl::GetTextDirection()
{
  Dali::InputMethodContext::TextDirection direction ( Dali::InputMethodContext::LeftToRight );

    if ( mIMFContext )
    {
      char* locale( NULL );
      ecore_imf_context_input_panel_language_locale_get( mIMFContext, &locale );

      if ( locale )
      {
        direction = static_cast< Dali::InputMethodContext::TextDirection >( Locale::GetDirection( std::string( locale ) ) );
        free( locale );
      }
    }

  return direction;
}

Rect<int> InputMethodContextEcoreWl::GetInputMethodArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_geometry_get( mIMFContext, &xPos, &yPos, &width, &height );
  }
  else
  {
    DALI_LOG_WARNING("VKB Unable to get IMFContext so GetSize unavailable\n");
  // return 0 as real size unknown.
  }

  return Rect<int>(xPos,yPos,width,height);
}

void InputMethodContextEcoreWl::ApplyOptions( const InputMethodOptions& options )
{
  using namespace Dali::InputMethod::Category;

  int index;

  if( mIMFContext == NULL )
  {
    DALI_LOG_WARNING("VKB Unable to excute ApplyOptions with Null ImfContext\n");
    return;
  }

  if( mOptions.CompareAndSet(PANEL_LAYOUT, options, index) )
  {
    ecore_imf_context_input_panel_layout_set( mIMFContext, panelLayoutMap[index] );

    // Sets the input hint which allows input methods to fine-tune their behavior.
    if( panelLayoutMap[index] == ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD )
    {
      ecore_imf_context_input_hint_set( mIMFContext, static_cast< Ecore_IMF_Input_Hints >( ecore_imf_context_input_hint_get( mIMFContext ) | ECORE_IMF_INPUT_HINT_SENSITIVE_DATA ) );
    }
    else
    {
      ecore_imf_context_input_hint_set( mIMFContext, static_cast< Ecore_IMF_Input_Hints >( ecore_imf_context_input_hint_get( mIMFContext ) & ~ECORE_IMF_INPUT_HINT_SENSITIVE_DATA ) );
    }
  }
  if( mOptions.CompareAndSet(BUTTON_ACTION, options, index) )
  {
    ecore_imf_context_input_panel_return_key_type_set( mIMFContext, returnKeyTypeMap[index] );
  }
  if( mOptions.CompareAndSet(AUTO_CAPITALIZE, options, index) )
  {
    ecore_imf_context_autocapital_type_set( mIMFContext, autoCapitalMap[index] );
  }
  if( mOptions.CompareAndSet(VARIATION, options, index) )
  {
    ecore_imf_context_input_panel_layout_variation_set( mIMFContext, index );
  }
}

void InputMethodContextEcoreWl::SetInputPanelData( const std::string& data )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetInputPanelData\n" );

  if( mIMFContext )
  {
    int length = data.length();
    ecore_imf_context_input_panel_imdata_set( mIMFContext, data.c_str(), length );
  }
}

void InputMethodContextEcoreWl::GetInputPanelData( std::string& data )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetInputPanelData\n" );

  if( mIMFContext )
  {
    int length = 4096; // The max length is 4096 bytes
    Dali::Vector< char > buffer;
    buffer.Resize( length );
    ecore_imf_context_input_panel_imdata_get( mIMFContext, &buffer[0], &length );
    data = std::string( buffer.Begin(), buffer.End() );
  }
}

Dali::InputMethodContext::State InputMethodContextEcoreWl::GetInputPanelState()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetInputPanelState\n" );

  if( mIMFContext )
  {
    int value;
    value = ecore_imf_context_input_panel_state_get( mIMFContext );

    switch (value)
    {
      case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
      {
        return Dali::InputMethodContext::SHOW;
        break;
      }

      case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
      {
        return Dali::InputMethodContext::HIDE;
        break;
      }

      case ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
      {
        return Dali::InputMethodContext::WILL_SHOW;
        break;
      }

      default:
      {
        return Dali::InputMethodContext::DEFAULT;
      }
    }
  }
  return Dali::InputMethodContext::DEFAULT;
}

void InputMethodContextEcoreWl::SetReturnKeyState( bool visible )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetReturnKeyState\n" );

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_return_key_disabled_set( mIMFContext, !visible );
  }
}

void InputMethodContextEcoreWl::AutoEnableInputPanel( bool enabled )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::AutoEnableInputPanel\n" );

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_enabled_set( mIMFContext, enabled );
  }
}

void InputMethodContextEcoreWl::ShowInputPanel()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::ShowInputPanel\n" );

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_show( mIMFContext );
  }
}

void InputMethodContextEcoreWl::HideInputPanel()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::HideInputPanel\n" );

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_hide( mIMFContext );
  }
}

Dali::InputMethodContext::KeyboardType InputMethodContextEcoreWl::GetKeyboardType()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetKeyboardType\n" );

  if( mIMFContext )
  {
    int value;
    value = ecore_imf_context_keyboard_mode_get( mIMFContext );

    switch (value)
    {
      case ECORE_IMF_INPUT_PANEL_SW_KEYBOARD_MODE:
      {
        return Dali::InputMethodContext::SOFTWARE_KEYBOARD;
        break;
      }
      case ECORE_IMF_INPUT_PANEL_HW_KEYBOARD_MODE:
      {
        return Dali::InputMethodContext::HARDWARE_KEYBOARD;
        break;
      }
    }
  }

  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

std::string InputMethodContextEcoreWl::GetInputPanelLocale()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetInputPanelLocale\n" );

  std::string locale = "";

  if( mIMFContext )
  {
    char* value = NULL;
    ecore_imf_context_input_panel_language_locale_get( mIMFContext, &value );

    if( value )
    {
      std::string valueCopy( value );
      locale = valueCopy;

      // The locale string retrieved must be freed with free().
      free( value );
    }
  }
  return locale;
}

bool InputMethodContextEcoreWl::FilterEventKey( const Dali::KeyEvent& keyEvent )
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

void InputMethodContextEcoreWl::AllowTextPrediction( bool prediction )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::AllowTextPrediction\n" );

  if( mIMFContext )
  {
    ecore_imf_context_prediction_allow_set( mIMFContext, prediction );
  }
}

bool InputMethodContextEcoreWl::IsTextPredictionAllowed() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::IsTextPredictionAllowed\n" );
  bool prediction = false;
  if( mIMFContext )
  {
    prediction = ecore_imf_context_prediction_allow_get( mIMFContext );
  }
  return prediction;
}

void InputMethodContextEcoreWl::SetInputPanelLanguage( Dali::InputMethodContext::InputPanelLanguage language )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetInputPanelLanguage\n" );
  if( mIMFContext )
  {
    switch (language)
    {
      case Dali::InputMethodContext::InputPanelLanguage::AUTOMATIC:
      {
        ecore_imf_context_input_panel_language_set( mIMFContext, ECORE_IMF_INPUT_PANEL_LANG_AUTOMATIC );
        break;
      }
      case Dali::InputMethodContext::InputPanelLanguage::ALPHABET:
      {
        ecore_imf_context_input_panel_language_set( mIMFContext, ECORE_IMF_INPUT_PANEL_LANG_ALPHABET );
        break;
      }
    }
  }
}

Dali::InputMethodContext::InputPanelLanguage InputMethodContextEcoreWl::GetInputPanelLanguage() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetInputPanelLanguage\n" );
  if( mIMFContext )
  {
    int value;
    value =  ecore_imf_context_input_panel_language_get( mIMFContext );

    switch (value)
    {
      case ECORE_IMF_INPUT_PANEL_LANG_AUTOMATIC:
      {
        return Dali::InputMethodContext::InputPanelLanguage::AUTOMATIC;
        break;
      }
      case ECORE_IMF_INPUT_PANEL_LANG_ALPHABET:
      {
        return Dali::InputMethodContext::InputPanelLanguage::ALPHABET;
        break;
      }
    }
  }
  return Dali::InputMethodContext::InputPanelLanguage::AUTOMATIC;
}

bool InputMethodContextEcoreWl::ProcessEventKeyDown( const KeyEvent& keyEvent )
{
  bool eventHandled( false );
  if ( mIMFContext )
  {
    Integration::KeyEvent integKeyEvent( keyEvent );
    std::string key = integKeyEvent.logicalKey;

    std::string compose = keyEvent.GetCompose();
    std::string deviceName = keyEvent.GetDeviceName();

    // We're consuming key down event so we have to pass to InputMethodContext so that it can parse it as well.
    Ecore_IMF_Event_Key_Down ecoreKeyDownEvent;
    ecoreKeyDownEvent.keyname = keyEvent.keyPressedName.c_str();
    ecoreKeyDownEvent.key = key.c_str();
    ecoreKeyDownEvent.string = keyEvent.keyPressed.c_str();
    ecoreKeyDownEvent.compose = compose.c_str();
    ecoreKeyDownEvent.timestamp = keyEvent.time;
    ecoreKeyDownEvent.modifiers = EcoreInputModifierToEcoreIMFModifier( keyEvent.keyModifier );
    ecoreKeyDownEvent.locks = EcoreInputModifierToEcoreIMFLock( keyEvent.keyModifier );
    ecoreKeyDownEvent.dev_name = deviceName.c_str();
    ecoreKeyDownEvent.dev_class = static_cast<Ecore_IMF_Device_Class> ( keyEvent.GetDeviceClass() );//ECORE_IMF_DEVICE_CLASS_KEYBOARD;
    ecoreKeyDownEvent.dev_subclass = static_cast<Ecore_IMF_Device_Subclass> ( keyEvent.GetDeviceSubclass() );//ECORE_IMF_DEVICE_SUBCLASS_NONE;

    // If the device is IME and the focused key is the direction keys, then we should send a key event to move a key cursor.
    if ((keyEvent.GetDeviceName() == "ime") && ((!strncmp(keyEvent.keyPressedName.c_str(), "Left", 4)) ||
                                   (!strncmp(keyEvent.keyPressedName.c_str(), "Right", 5)) ||
                                   (!strncmp(keyEvent.keyPressedName.c_str(), "Up", 2)) ||
                                   (!strncmp(keyEvent.keyPressedName.c_str(), "Down", 4))))
    {
      eventHandled = 0;
    }
    else
    {
      eventHandled = ecore_imf_context_filter_event(mIMFContext,
                                                    ECORE_IMF_EVENT_KEY_DOWN,
                                                    reinterpret_cast<Ecore_IMF_Event *>( &ecoreKeyDownEvent ) );
    }

    // If the event has not been handled by InputMethodContext then check if we should reset our input method context
    if (!eventHandled)
    {
      if (!strcmp(keyEvent.keyPressedName.c_str(), "Escape") ||
          !strcmp(keyEvent.keyPressedName.c_str(), "Return") ||
          !strcmp(keyEvent.keyPressedName.c_str(), "KP_Enter"))
      {
        ecore_imf_context_reset(mIMFContext);
      }
    }
  }
  return eventHandled;
}

bool InputMethodContextEcoreWl::ProcessEventKeyUp( const KeyEvent& keyEvent )
{
  bool eventHandled( false );
  if( mIMFContext )
  {
    Integration::KeyEvent integKeyEvent( keyEvent );
    std::string key = integKeyEvent.logicalKey;

    std::string compose = keyEvent.GetCompose();
    std::string deviceName = keyEvent.GetDeviceName();

    // We're consuming key up event so we have to pass to InputMethodContext so that it can parse it as well.
    Ecore_IMF_Event_Key_Up ecoreKeyUpEvent;
    ecoreKeyUpEvent.keyname = keyEvent.keyPressedName.c_str();
    ecoreKeyUpEvent.key = key.c_str();
    ecoreKeyUpEvent.string = keyEvent.keyPressed.c_str();
    ecoreKeyUpEvent.compose = compose.c_str();
    ecoreKeyUpEvent.timestamp = keyEvent.time;
    ecoreKeyUpEvent.modifiers =  EcoreInputModifierToEcoreIMFModifier( keyEvent.keyModifier );
    ecoreKeyUpEvent.locks = EcoreInputModifierToEcoreIMFLock( keyEvent.keyModifier );
    ecoreKeyUpEvent.dev_name = deviceName.c_str();
    ecoreKeyUpEvent.dev_class = static_cast<Ecore_IMF_Device_Class> ( keyEvent.GetDeviceClass() );//ECORE_IMF_DEVICE_CLASS_KEYBOARD;
    ecoreKeyUpEvent.dev_subclass = static_cast<Ecore_IMF_Device_Subclass> ( keyEvent.GetDeviceSubclass() );//ECORE_IMF_DEVICE_SUBCLASS_NONE;

    eventHandled = ecore_imf_context_filter_event(mIMFContext,
                                                  ECORE_IMF_EVENT_KEY_UP,
                                                  reinterpret_cast<Ecore_IMF_Event *>( &ecoreKeyUpEvent ) );
  }
  return eventHandled;
}

Ecore_IMF_Keyboard_Modifiers InputMethodContextEcoreWl::EcoreInputModifierToEcoreIMFModifier( unsigned int ecoreModifier )
{
  unsigned int modifier( ECORE_IMF_KEYBOARD_MODIFIER_NONE );  // If no other matches returns NONE.

  if ( ecoreModifier & ECORE_EVENT_MODIFIER_SHIFT )  // enums from ecore_input/Ecore_Input.h
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_SHIFT;  // enums from ecore_imf/ecore_imf.h
  }

  if ( ecoreModifier & ECORE_EVENT_MODIFIER_ALT )
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_ALT;
  }

  if ( ecoreModifier & ECORE_EVENT_MODIFIER_CTRL )
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_CTRL;
  }

  if ( ecoreModifier & ECORE_EVENT_MODIFIER_WIN )
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_WIN;
  }

  if ( ecoreModifier & ECORE_EVENT_MODIFIER_ALTGR )
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_ALTGR;
  }

  return static_cast<Ecore_IMF_Keyboard_Modifiers>( modifier );
}

Ecore_IMF_Keyboard_Locks InputMethodContextEcoreWl::EcoreInputModifierToEcoreIMFLock( unsigned int modifier )
{
    unsigned int lock( ECORE_IMF_KEYBOARD_LOCK_NONE ); // If no other matches, returns NONE.

    if( modifier & ECORE_EVENT_LOCK_NUM )
    {
      lock |= ECORE_IMF_KEYBOARD_LOCK_NUM; // Num lock is active.
    }

    if( modifier & ECORE_EVENT_LOCK_CAPS )
    {
      lock |= ECORE_IMF_KEYBOARD_LOCK_CAPS; // Caps lock is active.
    }

    if( modifier & ECORE_EVENT_LOCK_SCROLL )
    {
      lock |= ECORE_IMF_KEYBOARD_LOCK_SCROLL; // Scroll lock is active.
    }

    return static_cast<Ecore_IMF_Keyboard_Locks>( lock );
}

} // Adaptor

} // Internal

} // Dali
