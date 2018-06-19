/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <dali/internal/clipboard/common/clipboard-impl.h>

// EXTERNAL INCLUDES
#include <Ecore_X.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/window-system/ubuntu-x11/window-interface-ecore-x.h>
#include <dali/internal/system/common/singleton-service-impl.h>
#include <dali/internal/clipboard/common/clipboard-event-notifier-impl.h>

namespace //unnamed namespace
{
const char* const CBHM_WINDOW = "CBHM_XWIN";
const char* const CBHM_MSG = "CBHM_MSG";
const char* const CBHM_ITEM = "CBHM_ITEM";
const char* const CBHM_cCOUNT = "CBHM_cCOUNT";
const char* const CBHM_ERROR = "CBHM_ERROR";
const char* const SET_ITEM = "SET_ITEM";
const char* const SHOW = "show0";
const char* const HIDE = "cbhm_hide";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Clipboard
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

struct Clipboard::Impl
{
  Impl( Ecore_X_Window ecoreXwin )
  {
    mApplicationWindow = ecoreXwin;
  }

  Ecore_X_Window mApplicationWindow;
};

Clipboard::Clipboard(Impl* impl)
: mImpl( impl )
{
}

Clipboard::~Clipboard()
{
  delete mImpl;
}

Dali::Clipboard Clipboard::Get()
{
  Dali::Clipboard clipboard;

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::Clipboard ) );
    if(handle)
    {
      // If so, downcast the handle
      clipboard = Dali::Clipboard( dynamic_cast< Clipboard* >( handle.GetObjectPtr() ) );
    }
    else
    {
      Adaptor& adaptorImpl( Adaptor::GetImplementation( Adaptor::Get() ) );
      Any nativewindow = adaptorImpl.GetNativeWindowHandle();

      // The Ecore_X_Window needs to use the Clipboard.
      // Only when the render surface is window, we can get the Ecore_X_Window.
      Ecore_X_Window ecoreXwin( AnyCast<Ecore_X_Window>(nativewindow) );
      if (ecoreXwin)
      {
        // If we fail to get Ecore_X_Window, we can't use the Clipboard correctly.
        // Thus you have to call "ecore_imf_context_client_window_set" somewhere.
        // In EvasPlugIn, this function is called in EvasPlugin::ConnectEcoreEvent().
        Clipboard::Impl* impl( new Clipboard::Impl( ecoreXwin ) );
        clipboard = Dali::Clipboard( new Clipboard( impl ) );
        service.Register( typeid( clipboard ), clipboard );
      }
    }
  }

  return clipboard;
}
bool Clipboard::SetItem(const std::string &itemData )
{
  Ecore_X_Window cbhmWin = ECore::WindowInterface::GetWindow();
  Ecore_X_Atom atomCbhmItem = ecore_x_atom_get( CBHM_ITEM );
  Ecore_X_Atom dataType = ECORE_X_ATOM_STRING;

  // Set item (property) to send
  ecore_x_window_prop_property_set( cbhmWin, atomCbhmItem, dataType, 8, const_cast<char*>( itemData.c_str() ), itemData.length() + 1 );
  ecore_x_sync();

  // Trigger sending of item (property)
  Ecore_X_Atom atomCbhmMsg = ecore_x_atom_get( CBHM_MSG );
  ECore::WindowInterface::SendXEvent(ecore_x_display_get(), cbhmWin, False, NoEventMask, atomCbhmMsg, 8, SET_ITEM );
  return true;
}

/*
 * Request clipboard service to retrieve an item
 */
void Clipboard::RequestItem()
{
  int index = 0;
  char sendBuf[20];
  snprintf( sendBuf, 20,  "%s%d", CBHM_ITEM, index );
  Ecore_X_Atom xAtomCbhmItem = ecore_x_atom_get( sendBuf );
  Ecore_X_Atom xAtomItemType = 0;

  std::string clipboardString( ECore::WindowInterface::GetWindowProperty(xAtomCbhmItem, &xAtomItemType, index ) );

  // Only return the text string if the Atom type is text (Do not return a text string/URL for images).
  if( !clipboardString.empty() &&
      ( xAtomItemType == ECORE_X_ATOM_TEXT || xAtomItemType == ECORE_X_ATOM_COMPOUND_TEXT || xAtomItemType == ECORE_X_ATOM_STRING || xAtomItemType == ECORE_X_ATOM_UTF8_STRING ) )
  {
    Ecore_X_Atom xAtomCbhmError = ecore_x_atom_get( CBHM_ERROR );
    if ( xAtomItemType != xAtomCbhmError )
    {
      // Call ClipboardEventNotifier to notify event observe of retrieved string
      Dali::ClipboardEventNotifier clipboardEventNotifier(ClipboardEventNotifier::Get());
      if ( clipboardEventNotifier )
      {
        ClipboardEventNotifier& notifierImpl( ClipboardEventNotifier::GetImplementation( clipboardEventNotifier ) );

        notifierImpl.SetContent( clipboardString );
        notifierImpl.EmitContentSelectedSignal();
      }
    }
  }
}

/*
 * Get number of items in clipboard
 */
unsigned int Clipboard::NumberOfItems()
{
  Ecore_X_Atom xAtomCbhmCountGet = ecore_x_atom_get( CBHM_cCOUNT );

  std::string ret( ECore::WindowInterface::GetWindowProperty( xAtomCbhmCountGet, NULL, 0 ) );
  int count = 0;

  if ( !ret.empty() )
  {
    count = atoi( ret.c_str() );
  }

  return count;
}

/**
 * Show clipboard window
 * Function to send message to show the Clipboard (cbhm) as no direct API available
 * Reference elementary/src/modules/ctxpopup_copypasteUI/cbhm_helper.c
 */
void Clipboard::ShowClipboard()
{
  // Claim the ownership of the SECONDARY selection.
  ecore_x_selection_secondary_set(mImpl->mApplicationWindow, "", 1);
  Ecore_X_Window cbhmWin = ECore::WindowInterface::GetWindow();

  // Launch the clipboard window
  Ecore_X_Atom atomCbhmMsg = ecore_x_atom_get( CBHM_MSG );
  ECore::WindowInterface::SendXEvent( ecore_x_display_get(), cbhmWin, False, NoEventMask, atomCbhmMsg, 8, SHOW );
}

void Clipboard::HideClipboard(bool skipFirstHide)
{
  Ecore_X_Window cbhmWin = ECore::WindowInterface::GetWindow();
  // Launch the clipboard window
  Ecore_X_Atom atomCbhmMsg = ecore_x_atom_get( CBHM_MSG );
  ECore::WindowInterface::SendXEvent( ecore_x_display_get(), cbhmWin, False, NoEventMask, atomCbhmMsg, 8, HIDE );

  // release the ownership of SECONDARY selection
  ecore_x_selection_secondary_clear();
}

bool Clipboard::IsVisible() const
{
  return false;
}

char* Clipboard::ExcuteBuffered( bool type, void *event )
{
  if( !type )
  {
    // Receive
    Ecore_X_Event_Selection_Notify* selectionNotifyEvent = static_cast< Ecore_X_Event_Selection_Notify* >( event );

    Ecore_X_Selection_Data* selectionData = static_cast< Ecore_X_Selection_Data* >( selectionNotifyEvent->data );
    if( selectionData->data )
    {
      if( selectionNotifyEvent->selection == ECORE_X_SELECTION_SECONDARY )
      {
        // Claim the ownership of the SECONDARY selection.
        ecore_x_selection_secondary_set( mImpl->mApplicationWindow, "", 1 );

        return ( reinterpret_cast< char* >( selectionData->data ) );
      }
    }
  }
  return NULL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
