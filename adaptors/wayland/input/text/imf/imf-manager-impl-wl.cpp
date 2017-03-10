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
#include <imf-manager-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <singleton-service-impl.h>
#include <adaptor-impl.h>
#include <string.h>

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

unsigned int ConvertByteToVisualPosition( const char* utf8String, unsigned int cursorBytePosition )
{

  // check first byte so know how many bytes a character is represented by as keyboard returns cursor position in bytes. Which is different for some languages.
  size_t visualCharacterIndex = 0;
  size_t byteIndex = 0;

  // iterate through null terminated string checking each character's position against the given byte position ( attr->end_index ).
  const char leadByte = utf8String[byteIndex];
  while( leadByte != '\0' )
  {
    // attr->end_index is provided as a byte position not character and we need to know the character position.
    const size_t currentSequenceLength = Utf8SequenceLength( leadByte ); // returns number of bytes used to represent character.
    if ( byteIndex == cursorBytePosition )
    {
      return visualCharacterIndex;
      // end loop as found cursor position that matches byte position
    }
    else
    {
      byteIndex += currentSequenceLength; // jump to next character
      visualCharacterIndex++;  // increment character count so we know our position for when we get a match
    }
    DALI_ASSERT_DEBUG( visualCharacterIndex < strlen( utf8String ));
  }

  return visualCharacterIndex;
}

BaseHandle Create()
{
  return ImfManager::Get();
}

TypeRegistration IMF_MANAGER_TYPE( typeid(Dali::ImfManager), typeid(Dali::BaseHandle), Create );


}
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
  if (! service )
  {
    return manager;
  }

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
    manager = Dali::ImfManager( new ImfManager() );
    service.Register( typeid( manager ), manager );
  }
  else
  {
    DALI_LOG_ERROR("Failed to get native window handle\n");
  }
  return manager;
}
ImfManager::ImfManager()
: mTextInputManager( TextInputManager::Get() ),
  mPreEditCursorPosition( 0 ),
  mEditCursorPosition( 0 ),
  mRestoreAfterFocusLost( false )
{
  ConnectCallbacks();
}
ImfManager::~ImfManager()
{
  DisconnectCallbacks();
}

void ImfManager::ConnectCallbacks()
{
  mTextInputManager.PreEditStringSignal().Connect( this, &ImfManager::PreEditStringChange );
  mTextInputManager.PreEditCursorSignal().Connect( this, &ImfManager::PreEditCursorChange );
  mTextInputManager.CommitStringSignal().Connect( this, &ImfManager::CommitString );
  mTextInputManager.DeleteSurroundingTextSignal().Connect( this, &ImfManager::DeleteSurroundingText );

}
void ImfManager::DisconnectCallbacks()
{
  mTextInputManager.PreEditStringSignal().Disconnect( this, &ImfManager::PreEditStringChange );
  mTextInputManager.PreEditCursorSignal().Disconnect( this, &ImfManager::PreEditCursorChange );
  mTextInputManager.CommitStringSignal().Disconnect( this, &ImfManager::CommitString );
  mTextInputManager.DeleteSurroundingTextSignal().Disconnect( this, &ImfManager::DeleteSurroundingText );
}

void ImfManager::PreEditStringChange( unsigned int serial, const std::string text, const std::string commit  )
{

  int visualCursorPosition = 0;
  if( text.length() > 0 )
  {
    visualCursorPosition = ConvertByteToVisualPosition( text.c_str(), mPreEditCursorPosition );
  }
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::PreEditChanged to %s, pre-edit cursor %d \n",text.c_str(), mPreEditCursorPosition );


  // get the latest visual cursor pre-edit position

  Dali::ImfManager handle( this );
  Dali::ImfManager::ImfEventData imfEventData( Dali::ImfManager::PREEDIT, text, visualCursorPosition, visualCursorPosition );
  Dali::ImfManager::ImfCallbackData callbackData = mEventSignal.Emit( handle, imfEventData );

  if( callbackData.update )
  {
    mEditCursorPosition = callbackData.cursorPosition;
    mSurroundingText = callbackData.currentText;
  }

  if( callbackData.preeditResetRequired )
  {
    mPreEditCursorPosition = 0;
  }
}
void ImfManager::PreEditCursorChange( int cursor )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::PreEditCursorChange %d\n", cursor );

  mPreEditCursorPosition = cursor;

}

void ImfManager::CommitString( unsigned int serial, const std::string commit )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::CommitString\n", commit.c_str() );

  Dali::ImfManager handle( this );
  Dali::ImfManager::ImfEventData imfEventData( Dali::ImfManager::COMMIT, commit, 0, 0 );
  Dali::ImfManager::ImfCallbackData callbackData = mEventSignal.Emit( handle, imfEventData );

  if( callbackData.update )
  {
    SetCursorPosition( callbackData.cursorPosition );
    SetSurroundingText( callbackData.currentText );
    mEditCursorPosition = callbackData.cursorPosition;
    mPreEditCursorPosition = mEditCursorPosition;
    NotifyCursorPosition();
  }

}
void ImfManager::NotifyCursorPosition()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::NotifyCursorPosition \n" );

  // Set surrounding text also sets the cursor/ anchor position
  SetSurroundingText( mSurroundingText );
}

void ImfManager::DeleteSurroundingText( int index, unsigned int length )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "ImfManager::DeleteSurroundingText %d %d \n", index, length );

  Dali::ImfManager::ImfEventData imfData( Dali::ImfManager::DELETESURROUNDING, std::string(),index, length );
  Dali::ImfManager handle( this );
  mEventSignal.Emit( handle, imfData );
}

void ImfManager::Activate()
{
  Dali::ImfManager handle( this );
  mActivatedSignal.Emit( handle );

}

void ImfManager::Deactivate()
{
  // do nothing for now
}

void ImfManager::Reset()
{
  mSurroundingText = "";
  mPreEditCursorPosition = 0;
  mEditCursorPosition  = 0;
  mTextInputManager.Reset();
}

void* ImfManager::GetContext()
{
  return this;
}

bool ImfManager::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

void ImfManager::SetRestoreAfterFocusLost( bool toggle )
{
  mRestoreAfterFocusLost = toggle;
}

unsigned int ImfManager::GetCursorPosition() const
{
  return mEditCursorPosition;
}

void ImfManager::SetCursorPosition( unsigned int cursorPosition )
{
  mEditCursorPosition = cursorPosition;
}

void ImfManager::SetSurroundingText(  const std::string& text )
{
  mSurroundingText = text;
  mTextInputManager.SetSurroundingText( text, mEditCursorPosition, mEditCursorPosition /*anchor*/ );
}

const std::string& ImfManager::GetSurroundingText() const
{
  return mSurroundingText;
}

void ImfManager::NotifyTextInputMultiLine( bool multiLine )
{
}

Dali::Rect< int > ImfManager::GetInputMethodArea()
{
  TextInputManager::SeatInfo& info = mTextInputManager.GetLastActiveSeat();
  return info.mInputPanelDimensions;
}

void ImfManager::ApplyOptions(const InputMethodOptions& options)
{
}

Dali::ImfManager::TextDirection ImfManager::GetTextDirection()
{
  TextInputManager::SeatInfo& info = mTextInputManager.GetLastActiveSeat();
  return (Dali::ImfManager::TextDirection)info.mTextDirection;
}

} // Adaptor

} // Internal

} // Dali
