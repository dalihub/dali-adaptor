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
#include <key-grab.h>

// EXTERNAL INCLUDES
#include <Ecore_Wayland.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <window.h>
#include <key-impl.h>
#include <iostream>

// keycode-related code
#include "ecore_wl_private.h"

using namespace std;

// keycode-related code
namespace
{

const int KEYCODE_BUFFER_SIZE = 5;

typedef struct _keycode_map{
    xkb_keysym_t keysym;
    xkb_keycode_t *keycodes;
    int nkeycodes;
}keycode_map;

static void find_keycode( struct xkb_keymap *keymap, xkb_keycode_t key, void *data )
{
   keycode_map *found_keycodes = (keycode_map *)data;
   xkb_keysym_t keysym = found_keycodes->keysym;
   int nsyms = 0;
   const xkb_keysym_t *syms_out = NULL;
  
   nsyms = xkb_keymap_key_get_syms_by_level(keymap, key, 0, 0, &syms_out);
   if( nsyms && syms_out )
   {
     if( *syms_out == keysym )
     {
       found_keycodes->nkeycodes++;
       found_keycodes->keycodes = static_cast<xkb_keycode_t*>(realloc( found_keycodes->keycodes, sizeof(int)*found_keycodes->nkeycodes ));
       found_keycodes->keycodes[found_keycodes->nkeycodes-1] = key;
     }
   }
}

int xkb_keycode_from_keysym( struct xkb_keymap *keymap, xkb_keysym_t keysym, xkb_keycode_t **keycodes )
{
  keycode_map found_keycodes = {0,};
  found_keycodes.keysym = keysym;
  xkb_keymap_key_for_each( keymap, find_keycode, &found_keycodes );

  *keycodes = found_keycodes.keycodes;
  return found_keycodes.nkeycodes;
}

bool keycode_from_keyname( const char* keyname, char* keycode_buffer )
{
  xkb_keysym_t keysym = xkb_keysym_from_name( keyname, XKB_KEYSYM_NO_FLAGS );

  int nkeycodes = 0; // num of keycodes mapping with keysym
  xkb_keycode_t *keycodes = NULL; // keycodes list
  nkeycodes = xkb_keycode_from_keysym( ecore_wl_input_get()->xkb.keymap, keysym, &keycodes );
  if( nkeycodes > 0)
  {
    snprintf( keycode_buffer, KEYCODE_BUFFER_SIZE, "%d", keycodes[0] );
    return true;
  }
  else
  {
    return false;
  }
  free(keycodes);
}

} // unnamed namespace

namespace Dali
{

namespace KeyGrab
{

bool GrabKeyTopmost( Window window, Dali::KEY daliKey )
{
  return GrabKey( window, daliKey, TOPMOST);
}

bool UngrabKeyTopmost( Window window, Dali::KEY daliKey )
{
  return UngrabKey( window, daliKey );
}

bool GrabKey( Window window, Dali::KEY daliKey, KeyGrabMode grabMode )
{
  Ecore_Wl_Window_Keygrab_Mode wlGrabMode;
  if( grabMode == TOPMOST )
  {
    wlGrabMode = ECORE_WL_WINDOW_KEYGRAB_TOPMOST;
  }
  else if( grabMode == SHARED )
  {
    wlGrabMode = ECORE_WL_WINDOW_KEYGRAB_SHARED;
  }
  else if( grabMode == OVERRIDE_EXCLUSIVE )
  {
    wlGrabMode = ECORE_WL_WINDOW_KEYGRAB_EXCLUSIVE;
  }
  else if( grabMode == EXCLUSIVE )
  {
    wlGrabMode = ECORE_WL_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
  }
  else
  {
    return false;
  }

  // keycode-related code
  char keycode[KEYCODE_BUFFER_SIZE];
  if( !keycode_from_keyname( Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ), keycode ) )
  {
    DALI_LOG_WARNING( "Unable to get keycode from keyname %s.\n", Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ) );
    return false;
  }
  return ecore_wl_window_keygrab_set( AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ),
                                      keycode,
                                      0, 0, 0, wlGrabMode );

  // Currently the 2nd parameter of ecore_wl_window_keygrab_set means keycode, but its meaning will be changed to keyname later.
  // Once changed, we can remove all "keycode-related code" and just uncomment below line and
  // also can remove following files:
  // ecore_wl_private.h, tizen-extension-client-protocol.h, tizen-policy-client-protocol.h

  //return ecore_wl_window_keygrab_set( AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ),
                                      //Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ),
                                      //0, 0, 0, wlGrabMode );
}

bool UngrabKey( Window window, Dali::KEY daliKey )
{
  // keycode-related code
  char keycode[KEYCODE_BUFFER_SIZE];
  if( !keycode_from_keyname( Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ), keycode ) )
  {
    DALI_LOG_WARNING( "Unable to get keycode from keyname %s.\n", Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ) );
    return false;
  }
  return ecore_wl_window_keygrab_unset( AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ),
                                      keycode,
                                      0, 0 );

  // Currently the 2nd parameter of ecore_wl_window_keygrab_set means keycode, but its meaning will be changed to keyname later.
  // Once changed, we can remove all "keycode-related code" and just uncomment below line and
  // also can remove following files:
  // ecore_wl_private.h, tizen-extension-client-protocol.h, tizen-policy-client-protocol.h

  //return ecore_wl_window_keygrab_unset( AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ),
                                      //Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ),
                                      //0, 0 );
}

} // namespace KeyGrab

} // namespace Dali


