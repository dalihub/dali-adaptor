#ifndef __DALI_INPUT_MEHTOD_DEVEL_H__
#define __DALI_INPUT_MEHTOD_DEVEL_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */
namespace InputMethod
{

/**
 * @brief SetType that can be changed in the system Input Method
 *
 * Key types of map to change virtual keyboard settings in INPUT_METHOD_SETTINGS property
 *
 */
namespace Category
{
  enum Type
  {
    PANEL_LAYOUT,        ///< Set Keyboard layout
    ACTION_BUTTON_TITLE,       ///< Set Action button title
    AUTO_CAPITALISE,     ///< Set Auto capitalise of input
    VARIATION,           ///< Set variation
  };
}

/**
 * @brief Autocapitalization Types
 *
 */
namespace AutoCapital
{
  enum Type
  {
     NONE,         ///< No auto-capitalization when typing
     WORD,         ///< Autocapitalize each word typed
     SENTENCE,     ///< Autocapitalize the start of each sentence
     ALLCHARACTER, ///< Autocapitalize all letters
  };
}

/**
 * @brief  Input panel (virtual keyboard) layout types.
 *
 */
namespace PanelLayout
{
  enum Type
  {
     NORMAL,          ///< Default layout
     NUMBER,          ///< Number layout
     EMAIL,           ///< Email layout
     URL,             ///< URL layout
     PHONENUMBER,     ///< Phone Number layout
     IP,              ///< IP layout
     MONTH,           ///< Month layout
     NUMBERONLY,      ///< Number Only layout
     HEX,             ///< Hexadecimal layout
     TERMINAL,        ///< Command-line terminal layout including ESC, Alt, Ctrl key, so on (no auto-correct, no auto-capitalization)
     PASSWORD,        ///< Like normal, but no auto-correct, no auto-capitalization etc
     DATETIME,        ///< Date and time layout
     EMOTICON,        ///< Emoticon layout
  };
}

/**
 * @brief Specifies what the Input Method "action" button functionality is set to.
 *
 * The 'Action' button is traditionally the [RETURN] or [DONE] button.
 *
 */
namespace ActionButtonTitle
{
  enum Type
  {
    DEFAULT,       ///< Default action
    DONE,          ///< Done
    GO,            ///< Go action
    JOIN,          ///< Join action
    LOGIN,         ///< Login action
    NEXT,          ///< Next action
    PREVIOUS,      ///< Previous action
    SEARCH,        ///< Search action
    SEND,          ///< Send action
    SIGNIN,        ///< Sign in action
    UNSPECIFIED,   ///< Unspecified action
    NONE           ///< Nothing to do
  };
}

/**
 * @brief Available variation for Normal layout
 *
 *
 */
namespace NormalLayout
{
  enum Type
  {
     NORMAL,                 ///< The plain normal layout
     WITH_FILENAME,          ///< Filename layout. Symbols such as '/' should be disabled
     WITH_PERSON_NAME        ///< The name of a person
  };
}

/**
 * @brief Available variation for Number only layout
 *
 *
 */
namespace NumberOnlyLayout
{
  enum Type
  {
     NORMAL,                 ///< The plain normal number layout
     WITH_SIGNED,            ///< The number layout to allow a positive or negative sign at the start
     WITH_DECIMAL,           ///< The number layout to allow decimal point to provide fractional value
     WITH_SIGNED_AND_DECIMAL ///< The number layout to allow decimal point and negative sign
  };
}

/**
 * @brief Available variation for Password layout
 *
 *
 */
namespace PasswordLayout
{
  enum Type
  {
     NORMAL,               ///< The normal password layout
     WITH_NUMBERONLY       ///< The password layout to allow only number
  };
}

} // namespace InputMethod

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_INPUT_MEHTOD_DEVEL_H__
