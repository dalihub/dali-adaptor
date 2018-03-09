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
#include <dali/internal/input/common/virtual-keyboard-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace VirtualKeyboard
{
void Show()
{
}

void Hide()
{

}

bool IsVisible()
{
  return false;
}

void ApplySettings( const Property::Map& settingsMap )
{

}

void EnablePrediction( const bool enable )
{

}

bool IsPredictionEnabled()
{
  return false;
}

Rect<int> GetSizeAndPosition()
{
  Rect<int> ret;
  return ret;
}

Dali::VirtualKeyboard::StatusSignalType& StatusChangedSignal()
{
  Dali::VirtualKeyboard::StatusSignalType ret;
  return ret;
}

Dali::VirtualKeyboard::KeyboardResizedSignalType& ResizedSignal()
{
  Dali::VirtualKeyboard::KeyboardResizedSignalType ret;
  return ret;
}

Dali::VirtualKeyboard::LanguageChangedSignalType& LanguageChangedSignal()
{
  Dali::VirtualKeyboard::LanguageChangedSignalType ret;
  return ret;
}

Dali::VirtualKeyboard::TextDirection GetTextDirection()
{
  return Dali::VirtualKeyboard::LeftToRight;
}

Dali::InputMethod::ActionButton gActionButtonFunction = Dali::InputMethod::ACTION_DEFAULT;


void RotateTo(int angle)
{
}

void SetReturnKeyType( const InputMethod::ButtonAction::Type type )
{
}

Dali::InputMethod::ButtonAction::Type GetReturnKeyType()
{
  return Dali::InputMethod::ButtonAction::DEFAULT;
}

} // namespace VirtualKeyboard

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
