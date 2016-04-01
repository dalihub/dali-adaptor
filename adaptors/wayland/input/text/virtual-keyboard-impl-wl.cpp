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

// HEADER
#include <virtual-keyboard.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include "text-input-manager.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
#define TOKEN_STRING(x) #x

#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VIRTUAL_KEYBOARD");
#endif
}
// Current DALi Virtual Keyboard devel API has no knowledge of Seats
// so we use the most recently 'active' seat. If Tizen ever supports two input panels
// at the same time, we need to modify DALi to be aware of it.
namespace VirtualKeyboard
{

// Functions listed in order from virtual-keyboard.h

void Show()
{
  TextInputManager::Get().ShowInputPanel();
}

void Hide()
{
  TextInputManager::Get().HideInputPanel();
}

bool IsVisible()
{
  return TextInputManager::Get().IsInputPanelVisible();
}


void SetReturnKeyType( const InputMethod::ActionButton type )
{
  TextInputManager::Get().SetReturnKeyType( type );
}

void ApplySettings( const Property::Map& settingsMap )
{
  using namespace InputMethod; // Allows exclusion of namespace in TOKEN_STRING.

  for( unsigned int i = 0, count = settingsMap.Count(); i < count; ++i )
  {
    std::string key = settingsMap.GetKey( i );
    Property::Value item = settingsMap.GetValue(i);

    if( key == TOKEN_STRING( ACTION_BUTTON ) )
    {
      if( item.GetType() == Property::INTEGER )
      {
        int value = item.Get< int >();
        SetReturnKeyType( static_cast<InputMethod::ActionButton>(value) );
      }
    }
    else
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "Provided Settings Key not supported\n" );
    }
  }
}


InputMethod::ActionButton GetReturnKeyType()
{
  TextInputManager::SeatInfo& info = TextInputManager::Get().GetLastActiveSeat();
  return info.mReturnKeyType;
}

void EnablePrediction(const bool enable)
{
  // currently not part of Tizen text_input protocol
}

bool IsPredictionEnabled()
{
  return false;
}

Dali::Rect< int > GetSizeAndPosition()
{
  TextInputManager::SeatInfo& info = TextInputManager::Get().GetLastActiveSeat();
  return info.mInputPanelDimensions;
}

void RotateTo(int angle)
{
  // not part of the wayland protocol
}

Dali::VirtualKeyboard::TextDirection GetTextDirection()
{
  TextInputManager::SeatInfo& info = TextInputManager::Get().GetLastActiveSeat();
  return info.mTextDirection;
}

Dali::VirtualKeyboard::StatusSignalType& StatusChangedSignal()
{
  return TextInputManager::Get().StatusChangedSignal();
}

Dali::VirtualKeyboard::VoidSignalType& ResizedSignal()
{
  return TextInputManager::Get().ResizedSignal();
}

Dali::VirtualKeyboard::VoidSignalType& LanguageChangedSignal()
{
  return TextInputManager::Get().LanguageChangedSignal();
}

} // namespace VirtualKeyboard

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
