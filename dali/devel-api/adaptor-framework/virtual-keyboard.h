#ifndef __DALI_VIRTUAL_KEYBOARD_H__
#define __DALI_VIRTUAL_KEYBOARD_H__

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

// EXTERNAL INCLUDES
#include <dali/public-api/signals/dali-signal.h>
#include <dali/public-api/object/property-map.h>
#include <dali/public-api/math/rect.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/input-method.h>

namespace Dali
{

/**
 * @brief This namespace is provided for application developers to be able to show and hide the on-screen keyboard.
 *
 * Key events are sent to the actor in focus. Focus is set through the actor Public API.
 */
namespace VirtualKeyboard
{

// Types

typedef Signal< void () > VoidSignalType;
typedef Signal< void ( bool ) > StatusSignalType;
typedef Signal< void ( int ) > KeyboardResizedSignalType;
typedef Signal< void ( int ) > LanguageChangedSignalType;

// Enumerations

/**
 * @brief The direction of text.
 */
enum TextDirection
{
  LeftToRight,
  RightToLeft,
};

/**
 * @brief The meaning of the return key.
 */
enum ReturnKeyType
{
  DEFAULT,
  DONE,
  GO,
  JOIN,
  LOGIN,
  NEXT,
  SEARCH,
  SEND,
  SIGNIN
};

// Functions
/**
 * @brief Show the virtual keyboard.
 */
DALI_ADAPTOR_API void Show();

/**
 * @brief Hide the virtual keyboard.
 */
DALI_ADAPTOR_API void Hide();

/**
 * @brief Returns whether the virtual keyboard is visible or not.
 * @return true if visible, false otherwise.
 */
DALI_ADAPTOR_API bool IsVisible();

/**
 * @brief Set one or more of the Input Method Settings
 * @param[in] settingsMap Map of Settings to be applied.
 */
DALI_ADAPTOR_API void ApplySettings( const Property::Map& settingsMap );

/**
 * @brief Set the specific return key into the virtual keyboard.
 * @param[in] type the kind of return key types.
 */
DALI_ADAPTOR_API void SetReturnKeyType( const InputMethod::ButtonAction::Type type );

/**
 * @brief Retrieve the current return key type.
 * @return the type of retun key.
 */
DALI_ADAPTOR_API InputMethod::ButtonAction::Type GetReturnKeyType();

/**
 * @brief Enable/disable prediction (predictive text).
 *
 * By default prediction text is enabled.
 * @param[in] enable true or false to enable or disable
 * Prediction can not be changed while the keyboard is visible. It must be set in advance of showing keyboard.
 */
DALI_ADAPTOR_API void EnablePrediction(const bool enable);

/**
 * @brief Returns whether prediction is enabled in the virtual keyboard
 * @return true if predictive text enabled, false otherwise.
 */
DALI_ADAPTOR_API bool IsPredictionEnabled();

/**
 * @brief Provides size and position of keyboard.
 *
 * Position is relative to whether keyboard is visible or not.
 * If keyboard is not visible then position will be off the screen.
 * If keyboard is not being shown when this method is called the keyboard is partially setup (IMFContext) to get
 * the values then taken down.  So ideally GetSizeAndPosition() should be called after Show().
 * @return rect which is keyboard panel x, y, width, height
 */
DALI_ADAPTOR_API Dali::Rect<int> GetSizeAndPosition();

/**
 * @brief Rotates the keyboard orientation to the given angle.
 *
 * A value of 0 indicates the portrait orientation.
 * Other valid values are 90, 180, 270.
 * @param angle the angle is in degrees.
 */
DALI_ADAPTOR_API void RotateTo(int angle);

/**
 * @brief Returns text direction of the keyboard's current input language.
 * @return The direction of the text.
 */
DALI_ADAPTOR_API TextDirection GetTextDirection();

} // namespace VirtualKeyboard

} // namespace Dali

#endif // __DALI_VIRTUAL_KEYBOARD_H__
