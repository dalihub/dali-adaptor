/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <imf-manager-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <input-method-devel.h>
#include <adaptor.h>
#include <locale-utils.h>
#include <window-render-surface.h>
#include <adaptor-impl.h>
#include <singleton-service-impl.h>

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
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_IMF_MANAGER");
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
    ImfManager* imfManager = reinterpret_cast< ImfManager* > ( data );
    imfManager->CommitReceived( data, imfContext, event_info );
  }
}

void PreEdit( void *data, Ecore_IMF_Context *imfContext, void *event_info )
{
  if ( data )
  {
    ImfManager* imfManager = reinterpret_cast< ImfManager* > ( data );
    imfManager->PreEditChanged( data, imfContext, event_info );
  }
}

Eina_Bool ImfRetrieveSurrounding(void *data, Ecore_IMF_Context *imfContext, char** text, int* cursorPosition )
{
  if ( data )
  {
    ImfManager* imfManager = reinterpret_cast< ImfManager* > ( data );
    return imfManager->RetrieveSurrounding( data, imfContext, text, cursorPosition );
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
  ImfManager* imfManager = reinterpret_cast< ImfManager* > ( data );
  switch (value)
  {
    case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
    {
      imfManager->StatusChangedSignal().Emit( true );
      break;
    }

    case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
    {
      imfManager->StatusChangedSignal().Emit( false );
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
  ImfManager* imfManager = reinterpret_cast< ImfManager* > ( data );
  // Emit the signal that the language has changed
  imfManager->LanguageChangedSignal().Emit();
}

void InputPanelGeometryChangedCallback ( void *data, Ecore_IMF_Context *context, int value )
{
  if (!data)
  {
    return;
  }
  ImfManager* imfManager = reinterpret_cast< ImfManager* > ( data );
  // Emit signal that the keyboard is resized
  imfManager->ResizedSignal().Emit();
}

/**
 * Called when an IMF delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void ImfDeleteSurrounding( void *data, Ecore_IMF_Context *imfContext, void *event_info )
{
  if ( data )
  {
    ImfManager* imfManager = reinterpret_cast< ImfManager* > ( data );
    imfManager->DeleteSurrounding( data, imfContext, event_info );
  }
}

/**
 * Called when the input method sends a private command.
 */
void PrivateCommand( void *data, Ecore_IMF_Context *imfContext, void *event_info )
{
  if ( data )
  {
    ImfManager* imfManager = reinterpret_cast< ImfManager* > ( data );
    imfManager->SendPrivateCommand( data, imfContext, event_info );
  }
}

BaseHandle Create()
{
  return ImfManager::Get();
}

TypeRegistration IMF_MANAGER_TYPE( typeid(Dali::ImfManager), typeid(Dali::BaseHandle), Create );

} // unnamed namespace

bool ImfManager::IsAvailable()
{
  bool available( false );

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
    available = service.GetSingleton( typeid( Dali::ImfManager ) );
  }

  return available;
}

Dali::ImfManager ImfManager::Get()
{
  Dali::ImfManager manager;

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::ImfManager ) );
    if( handle )
    {
      // If so, downcast the handle
      manager = Dali::ImfManager( dynamic_cast< ImfManager* >( handle.GetObjectPtr() ) );
    }
    else if ( Adaptor::IsAvailable() )
    {
      // Create instance and register singleton only if the adaptor is available

      Adaptor& adaptorImpl( Adaptor::GetImplementation( Adaptor::Get() ) );
      Any nativeWindow = adaptorImpl.GetNativeWindowHandle();

      // The Ecore_Wl_Window needs to use the ImfManager.
      // Only when the render surface is window, we can get the Ecore_Wl_Window.
      Ecore_Wl_Window *ecoreWwin( AnyCast< Ecore_Wl_Window* >( nativeWindow ) );
      if (ecoreWwin)
      {
        // If we fail to get Ecore_Wl_Window, we can't use the ImfManager correctly.
        // Thus you have to call "ecore_imf_context_client_window_set" somewhere.
        // In EvasPlugIn, this function is called in EvasPlugin::ConnectEcoreEvent().

        manager = Dali::ImfManager( new ImfManager( ecoreWwin ) );
        service.Register( typeid( manager ), manager );
      }
      else
      {
        DALI_LOG_ERROR("Failed to get native window handle\n");
      }
    }
  }

  return manager;
}

ImfManager::ImfManager( Ecore_Wl_Window *ecoreWlwin )
: mIMFContext(),
  mIMFCursorPosition( 0 ),
  mSurroundingText(),
  mRestoreAfterFocusLost( false ),
  mIdleCallbackConnected( false )
{
  ecore_imf_init();
  CreateContext( ecoreWlwin );

  ConnectCallbacks();
}

ImfManager::~ImfManager()
{
  DisconnectCallbacks();

  DeleteContext();
  ecore_imf_shutdown();
}


void ImfManager::CreateContext( Ecore_Wl_Window *ecoreWlwin )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::CreateContext\n" );

  const char *contextId = ecore_imf_context_default_id_get();
  if( contextId )
  {
    mIMFContext = ecore_imf_context_add( contextId );

    if( mIMFContext )
    {
      if( ecoreWlwin )
      {
          ecore_imf_context_client_window_set( mIMFContext,
            reinterpret_cast<void*>( ecore_wl_window_id_get(ecoreWlwin)) );
      }
    }
    else
    {
      DALI_LOG_WARNING("IMF Unable to get IMF Context\n");
    }
  }
  else
  {
    DALI_LOG_WARNING("IMF Unable to get IMF Context\n");
  }
}

void ImfManager::DeleteContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::DeleteContext\n" );

  if ( mIMFContext )
  {
    ecore_imf_context_del( mIMFContext );
    mIMFContext = NULL;
  }
}

// Callbacks for predicitive text support.
void ImfManager::ConnectCallbacks()
{
  if ( mIMFContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::ConnectCallbacks\n" );

    ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED,      PreEdit,    this );
    ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_COMMIT,               Commit,     this );
    ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING,   ImfDeleteSurrounding, this );
    ecore_imf_context_event_callback_add( mIMFContext, ECORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, PrivateCommand, this );

    ecore_imf_context_input_panel_event_callback_add( mIMFContext, ECORE_IMF_INPUT_PANEL_STATE_EVENT,    InputPanelStateChangeCallback, this );
    ecore_imf_context_input_panel_event_callback_add( mIMFContext, ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT, InputPanelLanguageChangeCallback, this );
    ecore_imf_context_input_panel_event_callback_add( mIMFContext, ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT, InputPanelGeometryChangedCallback, this );

    ecore_imf_context_retrieve_surrounding_callback_set( mIMFContext, ImfRetrieveSurrounding, this);
  }
}

void ImfManager::DisconnectCallbacks()
{
  if ( mIMFContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::DisconnectCallbacks\n" );

    ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED,      PreEdit );
    ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_COMMIT,               Commit );
    ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING,   ImfDeleteSurrounding );
    ecore_imf_context_event_callback_del( mIMFContext, ECORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, PrivateCommand );

    ecore_imf_context_input_panel_event_callback_del( mIMFContext, ECORE_IMF_INPUT_PANEL_STATE_EVENT,    InputPanelStateChangeCallback     );
    ecore_imf_context_input_panel_event_callback_del( mIMFContext, ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT, InputPanelLanguageChangeCallback  );
    ecore_imf_context_input_panel_event_callback_del( mIMFContext, ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT, InputPanelGeometryChangedCallback );

    // We do not need to unset the retrieve surrounding callback.
  }
}

void ImfManager::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;

  if ( mIMFContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::Activate\n" );

    ecore_imf_context_focus_in( mIMFContext );

    // emit keyboard activated signal
    Dali::ImfManager handle( this );
    mActivatedSignal.Emit( handle );
  }
}

void ImfManager::Deactivate()
{
  if( mIMFContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::Deactivate\n" );

    Reset();
    ecore_imf_context_focus_out( mIMFContext );
  }

  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void ImfManager::Reset()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::Reset\n" );

  if ( mIMFContext )
  {
    ecore_imf_context_reset( mIMFContext );
  }
}

Ecore_IMF_Context* ImfManager::GetContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::GetContext\n" );

  return mIMFContext;
}

bool ImfManager::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

void ImfManager::SetRestoreAfterFocusLost( bool toggle )
{
  mRestoreAfterFocusLost = toggle;
}

/**
 * Called when an IMF Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the IMF module thinks
 * the user wants to type.
 */
void ImfManager::PreEditChanged( void*, Ecore_IMF_Context* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::PreEditChanged\n" );

  char* preEditString( NULL );
  int cursorPosition( 0 );
  Eina_List* attrs = NULL;
  Eina_List* l = NULL;

  Ecore_IMF_Preedit_Attr* attr;

  // Retrieves attributes as well as the string the cursor position offset from start of pre-edit string.
  // the attributes (attrs) is used in languages that use the soft arrows keys to insert characters into a current pre-edit string.
  ecore_imf_context_preedit_string_with_attributes_get( imfContext, &preEditString, &attrs, &cursorPosition );

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
    Dali::ImfManager handle( this );
    Dali::ImfManager::ImfEventData imfEventData( Dali::ImfManager::PREEDIT, preEditString, cursorPosition, 0 );
    Dali::ImfManager::ImfCallbackData callbackData = mEventSignal.Emit( handle, imfEventData );

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

void ImfManager::CommitReceived( void*, Ecore_IMF_Context* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::CommitReceived\n" );

  if ( Dali::Adaptor::IsAvailable() )
  {
    const std::string keyString( static_cast<char*>( event_info ) );

    Dali::ImfManager handle( this );
    Dali::ImfManager::ImfEventData imfEventData( Dali::ImfManager::COMMIT, keyString, 0, 0 );
    Dali::ImfManager::ImfCallbackData callbackData = mEventSignal.Emit( handle, imfEventData );

    if( callbackData.update )
    {
      SetCursorPosition( callbackData.cursorPosition );
      SetSurroundingText( callbackData.currentText );

      NotifyCursorPosition();
    }
  }
}

/**
 * Called when an IMF retrieve surround event is received.
 * Here the IMF module wishes to know the string we are working with and where within the string the cursor is
 * We need to signal the application to tell us this information.
 */
Eina_Bool ImfManager::RetrieveSurrounding( void* data, Ecore_IMF_Context* imfContext, char** text, int* cursorPosition )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::RetrieveSurrounding\n" );

  Dali::ImfManager::ImfEventData imfData( Dali::ImfManager::GETSURROUNDING, std::string(), 0, 0 );
  Dali::ImfManager handle( this );
  mEventSignal.Emit( handle, imfData );

  if( text )
  {
    *text = strdup( mSurroundingText.c_str() );
  }

  if( cursorPosition )
  {
    *cursorPosition = mIMFCursorPosition;
  }

  return EINA_TRUE;
}

/**
 * Called when an IMF delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void ImfManager::DeleteSurrounding( void* data, Ecore_IMF_Context* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::DeleteSurrounding\n" );

  if( Dali::Adaptor::IsAvailable() )
  {
    Ecore_IMF_Event_Delete_Surrounding* deleteSurroundingEvent = static_cast<Ecore_IMF_Event_Delete_Surrounding*>( event_info );

    Dali::ImfManager::ImfEventData imfData( Dali::ImfManager::DELETESURROUNDING, std::string(), deleteSurroundingEvent->offset, deleteSurroundingEvent->n_chars );
    Dali::ImfManager handle( this );
    mEventSignal.Emit( handle, imfData );
  }
}

/**
 * Called when the input method sends a private command.
 */
void ImfManager::SendPrivateCommand( void* data, Ecore_IMF_Context* imfContext, void* event_info )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::SendPrivateCommand\n" );

  if( Dali::Adaptor::IsAvailable() )
  {
    const char* privateCommandSendEvent = static_cast<const char*>( event_info );

    Dali::ImfManager::ImfEventData imfData( Dali::ImfManager::PRIVATECOMMAND, privateCommandSendEvent, 0, 0 );
    Dali::ImfManager handle( this );
    mEventSignal.Emit( handle, imfData );
  }
}

void ImfManager::NotifyCursorPosition()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::NotifyCursorPosition\n" );

  if( mIMFContext )
  {
    ecore_imf_context_cursor_position_set( mIMFContext, mIMFCursorPosition );
  }
}

void ImfManager::SetCursorPosition( unsigned int cursorPosition )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::SetCursorPosition\n" );

  mIMFCursorPosition = static_cast<int>( cursorPosition );
}

unsigned int ImfManager::GetCursorPosition() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::GetCursorPosition\n" );

  return static_cast<unsigned int>( mIMFCursorPosition );
}

void ImfManager::SetSurroundingText( const std::string& text )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::SetSurroundingText\n" );

  mSurroundingText = text;
}

const std::string& ImfManager::GetSurroundingText() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::GetSurroundingText\n" );

  return mSurroundingText;
}

void ImfManager::NotifyTextInputMultiLine( bool multiLine )
{
  Ecore_IMF_Input_Hints currentHint = ecore_imf_context_input_hint_get(mIMFContext);
  ecore_imf_context_input_hint_set(mIMFContext, (Ecore_IMF_Input_Hints)(multiLine ?
    (currentHint | ECORE_IMF_INPUT_HINT_MULTILINE) :
    (currentHint & ~ECORE_IMF_INPUT_HINT_MULTILINE)));
}

Dali::ImfManager::TextDirection ImfManager::GetTextDirection()
{
  Dali::ImfManager::TextDirection direction ( Dali::ImfManager::LeftToRight );

  if ( ImfManager::IsAvailable() /* We do not want to create an instance of ImfManager */ )
  {
    if ( mIMFContext )
    {
      char* locale( NULL );
      ecore_imf_context_input_panel_language_locale_get( mIMFContext, &locale );

      if ( locale )
      {
        direction = Locale::GetTextDirection( std::string( locale ) );
        free( locale );
      }
    }
  }
  return direction;
}

Rect<int> ImfManager::GetInputMethodArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_geometry_get( mIMFContext, &xPos, &yPos, &width, &height );
  }
  else
  {
    DALI_LOG_WARNING("VKB Unable to get IMF Context so GetSize unavailable\n");
  // return 0 as real size unknown.
  }

  return Rect<int>(xPos,yPos,width,height);
}

void ImfManager::ApplyOptions( const InputMethodOptions& options )
{
  using namespace Dali::InputMethod::Category;

  int index;

  if (mIMFContext == NULL)
  {
    DALI_LOG_WARNING("VKB Unable to excute ApplyOptions with Null ImfContext\n");
    return;
  }

  if ( mOptions.CompareAndSet(PANEL_LAYOUT, options, index) )
  {
    ecore_imf_context_input_panel_layout_set( mIMFContext, panelLayoutMap[index] );
  }
  if ( mOptions.CompareAndSet(AUTO_CAPITALISE, options, index) )
  {
    ecore_imf_context_autocapital_type_set( mIMFContext, autoCapitalMap[index] );
  }
  if ( mOptions.CompareAndSet(ACTION_BUTTON_TITLE, options, index) )
  {
    ecore_imf_context_input_panel_return_key_type_set( mIMFContext, returnKeyTypeMap[index] );
  }
  if ( mOptions.CompareAndSet(VARIATION, options, index) )
  {
    ecore_imf_context_input_panel_layout_variation_set( mIMFContext, index );
  }
}

void ImfManager::SetInputPanelUserData( const std::string& data )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::SetInputPanelUserData\n" );

  if( mIMFContext )
  {
    int length = data.length();
    ecore_imf_context_input_panel_imdata_set( mIMFContext, &data, length );
  }
}

void ImfManager::GetInputPanelUserData( std::string& data )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::GetInputPanelUserData\n" );

  if( mIMFContext )
  {
    int* length = NULL;
    ecore_imf_context_input_panel_imdata_get( mIMFContext, &data, length );
  }
}

Dali::ImfManager::State ImfManager::GetInputPanelState()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::GetInputPanelState\n" );

  if( mIMFContext )
  {
    int value;
    value = ecore_imf_context_input_panel_state_get( mIMFContext );

    switch (value)
    {
      case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
      {
        return Dali::ImfManager::SHOW;
        break;
      }

      case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
      {
        return Dali::ImfManager::HIDE;
        break;
      }

      case ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
      {
        return Dali::ImfManager::WILL_SHOW;
        break;
      }

      default:
      {
        return Dali::ImfManager::DEFAULT;
      }
    }
  }
  return Dali::ImfManager::DEFAULT;
}

void ImfManager::SetReturnKeyState( bool visible )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::SetReturnKeyState\n" );

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_return_key_disabled_set( mIMFContext, !visible );
  }
}

void ImfManager::AutoEnableInputPanel( bool enabled )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::AutoEnableInputPanel\n" );

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_enabled_set( mIMFContext, enabled );
  }
}

void ImfManager::ShowInputPanel()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::ShowInputPanel\n" );

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_show( mIMFContext );
  }
}

void ImfManager::HideInputPanel()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::HideInputPanel\n" );

  if( mIMFContext )
  {
    ecore_imf_context_input_panel_hide( mIMFContext );
  }
}

} // Adaptor

} // Internal

} // Dali
