#ifndef DALI_INTERNAL_VIRTUAL_KEYBOARD_H
#define DALI_INTERNAL_VIRTUAL_KEYBOARD_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/virtual-keyboard.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Implementation of the virtual keyboard namespace
 */
namespace VirtualKeyboard
{

/**
 * @copydoc Dali::VirtualKeyboard::Show()
 */
void Show();

/**
 * @copydoc Dali::VirtualKeyboard::Hide()
 */
void Hide();

/**
 * @copydoc Dali::VirtualKeyboard::IsVisible()
 */
bool IsVisible();

/**
 * @copydoc Dali::VirtualKeyboard::ApplySettings()
 */
void ApplySettings( const Property::Map& settingsMap );

/**
 * @copydoc Dali::VirtualKeyboard::SetReturnKeyType()
 */
void SetReturnKeyType( const Dali::InputMethod::ButtonAction::Type type );

/**
 * @copydoc Dali::VirtualKeyboard::GetReturnKeyType()
 */
Dali::InputMethod::ButtonAction::Type GetReturnKeyType();

/**
 * @copydoc Dali::VirtualKeyboard::EnablePrediction()
 */
void EnablePrediction(const bool enable);

/**
 * @copydoc Dali::VirtualKeyboard::IsPredictionEnabled()
 */
bool IsPredictionEnabled();

/**
 * @copydoc Dali::VirtualKeyboard::GetSizeAndPosition()
 */
Rect<int> GetSizeAndPosition();

/**
 * @copydoc Dali::VirtualKeyboard::RotateKeyboard()
 */
void RotateTo(int angle);

/**
 * @copydoc Dali::VirtualKeyboard::GetTextDirection
 */
Dali::VirtualKeyboard::TextDirection GetTextDirection();

} // namespace VirtualKeyboard

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_VIRTUAL_KEYBOARD_H
