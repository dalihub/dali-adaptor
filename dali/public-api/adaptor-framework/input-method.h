#ifndef __DALI_INPUT_MEHTOD_H__
#define __DALI_INPUT_MEHTOD_H__

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief Enumeration for input method.
 * @SINCE_1_0.0
 */
namespace InputMethod
{

/**
 * @brief Enumeration for settings that can be changed in the system Input Method.
 *
 * Key types of map to change virtual keyboard settings in INPUT_METHOD_SETTINGS property.
 * @SINCE_1_3.20
 */
namespace Category
{

/**
* @brief Enumeration for Category type.
*
* @SINCE_1_3.20
*/
enum Type
{
  PANEL_LAYOUT,        ///< Set Keyboard layout @SINCE_1_3.20
  BUTTON_ACTION,       ///< Set Button Action @SINCE_1_3.20
  AUTO_CAPITALIZE,     ///< Set Auto capitalize of input @SINCE_1_3.20
  VARIATION            ///< Set variation @SINCE_1_3.20
};

} // namespace Category

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// The enumerations below is to set each type of Category. ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Sets Input panel (virtual keyboard) layout types.
 *
 * @SINCE_1_3.20
 */
namespace PanelLayout
{

/**
* @brief Enumeration for PanelLayout type.
*
* @SINCE_1_3.20
*/
enum Type
{
  NORMAL,          ///< Default layout @SINCE_1_3.20
  NUMBER,          ///< Number layout @SINCE_1_3.20
  EMAIL,           ///< Email layout @SINCE_1_3.20
  URL,             ///< URL layout @SINCE_1_3.20
  PHONENUMBER,     ///< Phone Number layout @SINCE_1_3.20
  IP,              ///< IP layout @SINCE_1_3.20
  MONTH,           ///< Month layout @SINCE_1_3.20
  NUMBER_ONLY,     ///< Number Only layout @SINCE_1_3.20
  HEX,             ///< Hexadecimal layout @SINCE_1_3.20
  TERMINAL,        ///< Command-line terminal layout including ESC, Alt, Ctrl key, so on (no auto-correct, no auto-capitalization) @SINCE_1_3.20
  PASSWORD,        ///< Like normal, but no auto-correct, no auto-capitalization etc @SINCE_1_3.20
  DATE_TIME,       ///< Date and time layout @SINCE_1_3.20
  EMOTICON         ///< Emoticon layout @SINCE_1_3.20
};

} // namespace PanelLayout

/**
 * @brief Enumeration for specifying what the Input Method "action" button functionality is set to.
 *
 * The 'Action' button is traditionally the [RETURN] or [DONE] button.
 * Not all these actions are supported by all systems.
 * Setting a custom label will still require one of these actions to be set.
 * @SINCE_1_3.20
 */
namespace ButtonAction
{

/**
* @brief Enumeration for ButtonAction type.
*
* @SINCE_1_3.20
*/
enum Type
{
  DEFAULT,       ///< Default action @SINCE_1_3.20
  DONE,          ///< Done @SINCE_1_3.20
  GO,            ///< Go action @SINCE_1_3.20
  JOIN,          ///< Join action @SINCE_1_3.20
  LOGIN,         ///< Login action @SINCE_1_3.20
  NEXT,          ///< Next action @SINCE_1_3.20
  PREVIOUS,      ///< Previous action @SINCE_1_3.20
  SEARCH,        ///< Search action @SINCE_1_3.20
  SEND,          ///< Send action @SINCE_1_3.20
  SIGNIN,        ///< Sign in action @SINCE_1_3.20
  UNSPECIFIED,   ///< Unspecified action @SINCE_1_3.20
  NONE           ///< Nothing to do @SINCE_1_3.20
};

} // namespace ButtonAction


/**
 * @brief Sets Autocapitalization Types.
 *
 * @SINCE_1_3.20
 */
namespace AutoCapital
{

/**
* @brief Enumeration for AutoCapital type.
*
* @SINCE_1_3.20
*/
enum Type
{
  NONE,         ///< No auto-capitalization when typing @SINCE_1_3.20
  WORD,         ///< Autocapitalize each word typed @SINCE_1_3.20
  SENTENCE,     ///< Autocapitalize the start of each sentence @SINCE_1_3.20
  ALL_CHARACTER ///< Autocapitalize all letters @SINCE_1_3.20
};

} // namespace AutoCapital


/////////////////////////// VARIATION based on PANEL_LAYOUT //////////////////////////////////

/**
 * @brief Available variation for Normal layout.
 *
 * @SINCE_1_3.20
 */
namespace NormalLayout
{

/**
* @brief Enumeration for NormalLayout Variation type.
*
* @SINCE_1_3.20
*/
enum Type
{
  NORMAL,                 ///< The plain normal layout @SINCE_1_3.20
  WITH_FILENAME,          ///< Filename layout. Symbols such as '/' should be disabled @SINCE_1_3.20
  WITH_PERSON_NAME        ///< The name of a person @SINCE_1_3.20
};

} // namespace NormalLayout

/**
 * @brief Available variation for Number only layout.
 *
 * @SINCE_1_3.20
 */
namespace NumberOnlyLayout
{

/**
* @brief Enumeration for NumberOnlyLayout Variation type.
*
* @SINCE_1_3.20
*/
enum Type
{
  NORMAL,                 ///< The plain normal number layout @SINCE_1_3.20
  WITH_SIGNED,            ///< The number layout to allow a positive or negative sign at the start @SINCE_1_3.20
  WITH_DECIMAL,           ///< The number layout to allow decimal point to provide fractional value @SINCE_1_3.20
  WITH_SIGNED_AND_DECIMAL ///< The number layout to allow decimal point and negative sign @SINCE_1_3.20
};

} // namespace NumberOnlyLayout

/**
 * @brief Available variation for Password layout.
 *
 * @SINCE_1_3.20
 */
namespace PasswordLayout
{

/**
* @brief Enumeration for PasswordLayout Variation type.
*
* @SINCE_1_3.20
*/
enum Type
{
  NORMAL,               ///< The normal password layout @SINCE_1_3.20
  WITH_NUMBER_ONLY      ///< The password layout to allow only number @SINCE_1_3.20
};

} // namespace PasswordLayout


/**
 * @DEPRECATED_1_3.20 Use ButtonAction instead.
 * @brief Enumeration for specifying what the Input Method "action" button functionality is set to.
 *
 * The 'Action' button is traditionally the [RETURN] or [DONE] button.
 *
 * Not all these actions are supported by all systems.
 *
 * Setting a custom label will still require one of these actions to be set.
 * @SINCE_1_0.0
 */
enum ActionButton
{
  ACTION_DEFAULT,       ///< Default action @SINCE_1_0.0
  ACTION_DONE,          ///< Done @SINCE_1_0.0
  ACTION_GO,            ///< Go action @SINCE_1_0.0
  ACTION_JOIN,          ///< Join action @SINCE_1_0.0
  ACTION_LOGIN,         ///< Login action @SINCE_1_0.0
  ACTION_NEXT,          ///< Next action @SINCE_1_0.0
  ACTION_PREVIOUS,      ///< Previous action @SINCE_1_0.0
  ACTION_SEARCH,        ///< Search action @SINCE_1_0.0
  ACTION_SEND,          ///< Send action @SINCE_1_0.0
  ACTION_SIGNIN,        ///< Sign in action @SINCE_1_0.0
  ACTION_UNSPECIFIED,   ///< Unspecified action @SINCE_1_0.0
  ACTION_NONE           ///< Nothing to do @SINCE_1_0.0
};

/**
 * @DEPRECATED_1_3.20 Use Category instead.
 * @brief Enumeration for settings that can be changed in the system Input Method.
 *
 * Not all these settings are supported by all systems.
 * @SINCE_1_0.0
 */
enum Settings
{
  ACTION_BUTTON,          ///< ActionButton. Apply the one of the ActionButton functions to the action button (return button). @SINCE_1_0.0
  AUTO_CAPITALISE,        ///< boolean.      Capitalize the first letter of each sentence automatically. @SINCE_1_0.0
  AUTO_COMPLETE,          ///< boolean.      Suggest words based on the current input. @SINCE_1_0.0
  AUTO_CORRECT            ///< boolean.      Automatically correct commonly misspelt words. @SINCE_1_0.0
};

} // namespace InputMethod

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_INPUT_MEHTOD_H__
