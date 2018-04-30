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
#include <dali/public-api/adaptor-framework/key-grab.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-impl.h>

namespace Dali
{

namespace KeyGrab
{

bool GrabKeyTopmost( Window window, Dali::KEY daliKey )
{
  Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation( window );
  return windowImpl.GrabKey( daliKey, TOPMOST );
}

bool UngrabKeyTopmost( Window window, Dali::KEY daliKey )
{
  Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation( window );
  return windowImpl.UngrabKey( daliKey );
}

bool GrabKey( Window window, Dali::KEY daliKey, KeyGrabMode grabMode )
{
  Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation( window );
  return windowImpl.GrabKey( daliKey, grabMode );
}


bool UngrabKey( Window window, Dali::KEY daliKey )
{
  Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation( window );
  return windowImpl.UngrabKey( daliKey );
}


bool GrabKeyList( Window window, const Dali::Vector<Dali::KEY>& daliKeyVector, const Dali::Vector<KeyGrabMode>& grabModeVector, Dali::Vector<bool>& returnVector)
{
  Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation( window );
  return windowImpl.GrabKeyList( daliKeyVector, grabModeVector, returnVector );
}

bool UngrabKeyList( Window window, const Dali::Vector<Dali::KEY>& daliKeyVector, Dali::Vector<bool>& returnVector)
{
  Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation( window );
  return windowImpl.UngrabKeyList( daliKeyVector, returnVector );
}

} // namespace KeyGrab

} // namespace Dali

