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
#include <dali/devel-api/adaptor-framework/virtual-keyboard.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/virtual-keyboard-impl.h>

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

void ApplySettings( const Property::Map& settingsMap )
{
  Internal::Adaptor::VirtualKeyboard::ApplySettings( settingsMap );
}

void SetReturnKeyType( const InputMethod::ButtonAction::Type type )
{
  Internal::Adaptor::VirtualKeyboard::SetReturnKeyType( type );
}

InputMethod::ButtonAction::Type GetReturnKeyType()
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

TextDirection GetTextDirection()
{
  return Internal::Adaptor::VirtualKeyboard::GetTextDirection();
}

} // namespace VirtualKeyboard

} // namespace Dali
