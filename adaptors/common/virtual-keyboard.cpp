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
#include <virtual-keyboard.h>

// INTERNAL INCLUDES
#include <virtual-keyboard-impl.h>

namespace Dali
{

namespace VirtualKeyboard
{

void Show()
{
  Internal::Adaptor::VirtualKeyboard::Show();
}

void Hide()
{
  Internal::Adaptor::VirtualKeyboard::Hide();
}

bool IsVisible()
{
  return Internal::Adaptor::VirtualKeyboard::IsVisible();
}

void SetReturnKeyType( ReturnKeyType type )
{
  Internal::Adaptor::VirtualKeyboard::SetReturnKeyType( type );
}

ReturnKeyType GetReturnKeyType()
{
  return Internal::Adaptor::VirtualKeyboard::GetReturnKeyType();
}

void EnablePrediction(const bool enable)
{
  Internal::Adaptor::VirtualKeyboard::EnablePrediction(enable);
}

bool IsPredictionEnabled()
{
  return Internal::Adaptor::VirtualKeyboard::IsPredictionEnabled();
}

Rect<int> GetSizeAndPosition()
{
  return Internal::Adaptor::VirtualKeyboard::GetSizeAndPosition();
}

void RotateTo(int angle)
{
  Internal::Adaptor::VirtualKeyboard::RotateTo(angle);
}

StatusSignalV2& StatusChangedSignal()
{
  return Internal::Adaptor::VirtualKeyboard::StatusChangedSignal();
}

VoidSignalV2& ResizedSignal()
{
  return Internal::Adaptor::VirtualKeyboard::ResizedSignal();
}

VoidSignalV2& LanguageChangedSignal()
{
  return Internal::Adaptor::VirtualKeyboard::LanguageChangedSignal();
}

TextDirection GetTextDirection()
{
  return Internal::Adaptor::VirtualKeyboard::GetTextDirection();
}

} // namespace VirtualKeyboard

} // namespace Dali
