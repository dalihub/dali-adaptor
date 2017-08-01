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

Dali::Vector<bool> GrabKeyList( Window window, const Dali::Vector<Dali::KEY>& daliKeyVector, const Dali::Vector<KeyGrabMode>& grabModeVector)
{
  Dali::Vector<bool> resultVector;
  Eina_List *keyList = NULL, *grabList = NULL, *l = NULL, *m = NULL;
  void *listData = NULL, *data = NULL;
  Dali::Vector<Dali::KEY>::SizeType keyCount = daliKeyVector.Count();
  Dali::Vector<KeyGrabMode>::SizeType keyGrabModeCount = grabModeVector.Count();


  if(keyCount != keyGrabModeCount || keyCount == 0)
    return resultVector;

  eina_init();

  for( Dali::Vector<float>::SizeType index = 0; index < keyCount; ++index )
  {
    Ecore_Wl_Window_Keygrab_Info *info = (Ecore_Wl_Window_Keygrab_Info*)malloc(sizeof(Ecore_Wl_Window_Keygrab_Info));
    if( info )
    {
      info->key = (char*)Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKeyVector[index] );

      switch(grabModeVector[index])
      {
        case TOPMOST:
          info->mode = ECORE_WL_WINDOW_KEYGRAB_TOPMOST;
          break;
        case SHARED:
          info->mode = ECORE_WL_WINDOW_KEYGRAB_SHARED;
          break;
        case OVERRIDE_EXCLUSIVE:
          info->mode = ECORE_WL_WINDOW_KEYGRAB_EXCLUSIVE;
          break;
        case EXCLUSIVE:
          info->mode = ECORE_WL_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
          break;
        default:
          info->mode = ECORE_WL_WINDOW_KEYGRAB_UNKNOWN;
          break;
      }

      keyList = eina_list_append(keyList, info);
    }
  }

  grabList = ecore_wl_window_keygrab_list_set(AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ), keyList);

  for( Dali::Vector<float>::SizeType index = 0; index < keyCount; ++index )
  {
    resultVector.PushBack(true);
  }

  if( grabList != NULL)
  {
    EINA_LIST_FOREACH(grabList, m, data)
    {
      Dali::Vector<float>::SizeType index = 0;
      EINA_LIST_FOREACH(keyList, l, listData)
      {
        if(strcmp((char*)data, ((Ecore_Wl_Window_Keygrab_Info*)listData)->key) == 0)
          resultVector[index] = false;

        ++index;
      }
    }
  }

  eina_list_free(keyList);
  eina_list_free(grabList);
  eina_shutdown();

  return resultVector;
}

Dali::Vector<bool> UngrabKeyList( Window window, const Dali::Vector<Dali::KEY>& daliKeyVector )
{
  Dali::Vector<bool> resultVector;
  Eina_List *keyList = NULL, *ungrabList = NULL, *l = NULL, *m = NULL;
  void *listData = NULL, *data = NULL;
  Dali::Vector<Dali::KEY>::SizeType keyCount = daliKeyVector.Count();


  if(keyCount == 0)
    return resultVector;

  eina_init();

  for( Dali::Vector<float>::SizeType index = 0; index < keyCount; ++index )
  {
    Ecore_Wl_Window_Keygrab_Info *info = (Ecore_Wl_Window_Keygrab_Info*)malloc(sizeof(Ecore_Wl_Window_Keygrab_Info));
    if( info )
    {
      info->key = (char*)Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKeyVector[index] );
      keyList = eina_list_append(keyList, info);
    }
  }

  ungrabList = ecore_wl_window_keygrab_list_unset(AnyCast<Ecore_Wl_Window*>( window.GetNativeHandle() ), keyList);

  for( Dali::Vector<float>::SizeType index = 0; index < keyCount; ++index )
  {
    resultVector.PushBack(true);
  }

  if( ungrabList != NULL)
  {
    EINA_LIST_FOREACH(ungrabList, m, data)
    {
      Dali::Vector<float>::SizeType index = 0;
      EINA_LIST_FOREACH(keyList, l, listData)
      {
        if(strcmp((char*)data, ((Ecore_Wl_Window_Keygrab_Info*)listData)->key) == 0)
          resultVector[index] = false;

        ++index;
      }
    }
  }

  eina_list_free(keyList);
  eina_list_free(ungrabList);
  eina_shutdown();

  return resultVector;
}

} // namespace KeyGrab

} // namespace Dali


