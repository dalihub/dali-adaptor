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
#include <key-grab.h>

// EXTERNAL INCLUDES
#include <Ecore_Wayland.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <window.h>
#include <key-impl.h>

#include <iostream>
#include <string.h>

using namespace std;

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
    wlGrabMode = ECORE_WL_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
  }
  else if( grabMode == EXCLUSIVE )
  {
    wlGrabMode = ECORE_WL_WINDOW_KEYGRAB_EXCLUSIVE;
  }
  else
  {
    return false;
  }

  return ecore_wl_window_keygrab_set( AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ),
                                      Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ),
                                      0, 0, 0, wlGrabMode );
}


bool UngrabKey( Window window, Dali::KEY daliKey )
{
  return ecore_wl_window_keygrab_unset( AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ),
                                      Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ),
                                      0, 0 );
}


bool GrabKeyList( Window window, const Dali::Vector<Dali::KEY>& daliKeyVector, const Dali::Vector<KeyGrabMode>& grabModeVector, Dali::Vector<bool>& returnVector)
{
  const Dali::Vector<bool>::SizeType returnCount = returnVector.Count();
  const Dali::Vector<Dali::KEY>::SizeType keyCount = daliKeyVector.Count();
  const Dali::Vector<KeyGrabMode>::SizeType keyGrabModeCount = grabModeVector.Count();

  if( keyCount != keyGrabModeCount || keyCount != returnCount || keyCount == 0 )
  {
    return false;
  }

  eina_init();

  Eina_List* keyList = NULL;
  {
    for( Dali::Vector<float>::SizeType index = 0; index < keyCount; ++index )
    {
      Ecore_Wl_Window_Keygrab_Info info;
      info.key = const_cast<char*>(Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKeyVector[index] ));

      switch( grabModeVector[index] )
      {
        case TOPMOST:
        {
          info.mode = ECORE_WL_WINDOW_KEYGRAB_TOPMOST;
          break;
        }
        case SHARED:
        {
          info.mode = ECORE_WL_WINDOW_KEYGRAB_SHARED;
          break;
        }
        case OVERRIDE_EXCLUSIVE:
        {
          info.mode = ECORE_WL_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
          break;
        }
        case EXCLUSIVE:
        {
          info.mode = ECORE_WL_WINDOW_KEYGRAB_EXCLUSIVE;
          break;
        }
        default:
        {
          info.mode = ECORE_WL_WINDOW_KEYGRAB_UNKNOWN;
          break;
        }
      }

      keyList = eina_list_append( keyList, &info );
    }
  }

  Eina_List* grabList = ecore_wl_window_keygrab_list_set( AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ), keyList );

  returnVector.Resize( keyCount, true );

  Eina_List* l = NULL;
  Eina_List* m = NULL;
  void *listData = NULL;
  void *data = NULL;
  if( grabList != NULL )
  {
    EINA_LIST_FOREACH( grabList, m, data )
    {
      Dali::Vector<float>::SizeType index = 0;
      EINA_LIST_FOREACH( keyList, l, listData )
      {
        if((static_cast<Ecore_Wl_Window_Keygrab_Info*>(listData))->key == NULL)
        {
          DALI_LOG_ERROR("input key list has null data!");
          break;
        }

        if( strcmp( static_cast<char*>(data), static_cast<Ecore_Wl_Window_Keygrab_Info*>(listData)->key ) == 0 )
        {
          returnVector[index] = false;
        }
        ++index;
      }
    }
  }

  eina_list_free( keyList );
  eina_list_free( grabList );
  eina_shutdown();

  return true;
}

bool UngrabKeyList( Window window, const Dali::Vector<Dali::KEY>& daliKeyVector, Dali::Vector<bool>& returnVector)
{
  const Dali::Vector<bool>::SizeType returnCount = returnVector.Count();
  const Dali::Vector<Dali::KEY>::SizeType keyCount = daliKeyVector.Count();

  if( keyCount != returnCount ||keyCount == 0 )
  {
    return false;
  }

  eina_init();

  Eina_List* keyList = NULL;
  {
    for( Dali::Vector<float>::SizeType index = 0; index < keyCount; ++index )
    {
      Ecore_Wl_Window_Keygrab_Info info;
      info.key = const_cast<char*>(Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKeyVector[index] ));
      keyList = eina_list_append( keyList, &info );
    }
  }

  Eina_List* ungrabList = ecore_wl_window_keygrab_list_unset( AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ), keyList );

  returnVector.Resize( keyCount, true );

  Eina_List* l = NULL;
  Eina_List* m = NULL;
  void *listData = NULL;
  void *data = NULL;

  if( ungrabList != NULL )
  {
    EINA_LIST_FOREACH( ungrabList, m, data )
    {
      Dali::Vector<float>::SizeType index = 0;
      EINA_LIST_FOREACH( keyList, l, listData )
      {
        if( strcmp( static_cast<char*>(data), static_cast<Ecore_Wl_Window_Keygrab_Info*>(listData)->key ) == 0 )
        {
          returnVector[index] = false;
        }
        ++index;
      }
    }
  }

  eina_list_free( keyList );
  eina_list_free( ungrabList );
  eina_shutdown();

  return true;
}

} // namespace KeyGrab

} // namespace Dali

