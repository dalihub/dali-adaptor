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
#include <Ecore.h>
#include <Ecore_X.h>
#include <utilX.h>

// INTERNAL INCLUDES
#include <window.h>
#include <key-impl.h>
#include <ecore-x-types.h>

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
  int xGrabMode;
  if( grabMode == TOPMOST )
  {
    xGrabMode = TOP_POSITION_GRAB;
  }
  else if( grabMode == SHARED )
  {
    xGrabMode = SHARED_GRAB;
  }
  else if( grabMode == OVERRIDE_EXCLUSIVE )
  {
    xGrabMode = OR_EXCLUSIVE_GRAB;
  }
  else if( grabMode == EXCLUSIVE )
  {
    xGrabMode = EXCLUSIVE_GRAB;
  }
  else
  {
    return false;
  }

  int ret = utilx_grab_key ( static_cast<Display*>( ecore_x_display_get() ),
                             static_cast<XWindow>( AnyCast<Ecore_X_Window>( window.GetNativeHandle() ) ),
                             Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ), xGrabMode );
  return ret==0;
}

bool UngrabKey( Window window, Dali::KEY daliKey )
{
  int ret = utilx_ungrab_key ( static_cast<Display*>( ecore_x_display_get() ),
                               static_cast<XWindow>( AnyCast<Ecore_X_Window>( window.GetNativeHandle() ) ),
                               Dali::Internal::Adaptor::KeyLookup::GetKeyName( daliKey ) );
  return ret==0;
}

} // namespace KeyGrab

} // namespace Dali


