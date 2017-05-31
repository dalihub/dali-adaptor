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

} // namespace KeyGrab

} // namespace Dali


