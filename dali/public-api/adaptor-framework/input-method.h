#ifndef DALI_INPUT_METHOD_H
#define DALI_INPUT_METHOD_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
 *
 * @SINCE_1_0.0
 */
namespace InputMethod
{
/**
 * @brief Sets Input panel (virtual keyboard) layout types.
 *
 * @SINCE_2_5.27
 */
enum class PanelLayout
{
  NORMAL,      ///< Default layout @SINCE_2_5.27
  NUMBER,      ///< Number layout @SINCE_2_5.27
  EMAIL,       ///< Email layout @SINCE_2_5.27
  URL,         ///< URL layout @SINCE_2_5.27
  PHONENUMBER, ///< Phone Number layout @SINCE_2_5.27
  IP,          ///< IP layout @SINCE_2_5.27
  MONTH,       ///< Month layout @SINCE_2_5.27
  NUMBER_ONLY, ///< Number Only layout @SINCE_2_5.27
  PASSWORD,    ///< Like normal, but no auto-correct, no auto-capitalization etc @SINCE_2_5.27
  DATE_TIME,   ///< Date and time layout @SINCE_2_5.27
  EMOTICON,    ///< Emoticon layout @SINCE_2_5.27
  VOICE        ///< Voice layout, but if the IME does not support voice layout, then normal layout will be shown. @SINCE_2_5.27
};

/**
 * @brief Enumeration for specifying the input panel return key type.
 *
 * The return key is traditionally the [RETURN] or [DONE] key. Not all these
 * key types are supported by all systems.
 *
 * @SINCE_2_5.27
 */
enum class ReturnKeyType
{
  DEFAULT, ///< Default return key type @SINCE_2_5.27
  DONE,    ///< Done @SINCE_2_5.27
  GO,      ///< Go @SINCE_2_5.27
  JOIN,    ///< Join @SINCE_2_5.27
  LOGIN,   ///< Login @SINCE_2_5.27
  NEXT,    ///< Next @SINCE_2_5.27
  SEARCH,  ///< Search or magnifier icon @SINCE_2_5.27
  SEND,    ///< Send @SINCE_2_5.27
  SIGNIN   ///< Sign in @SINCE_2_5.27
};

/**
 * @brief Sets Autocapitalization Types.
 *
 * @SINCE_2_5.27
 */
enum class AutoCapitalType
{
  NONE,         ///< No auto-capitalization when typing @SINCE_2_5.27
  WORD,         ///< Autocapitalize each word typed @SINCE_2_5.27
  SENTENCE,     ///< Autocapitalize the start of each sentence @SINCE_2_5.27
  ALL_CHARACTER ///< Autocapitalize all letters @SINCE_2_5.27
};

/**
 * @brief Sets input panel layout variation types.
 *
 * The variation value is grouped by input panel layout.
 * Each layout defines its own variation range for extensibility.
 *
 * The actual platform input method variation value is layout-specific and may
 * differ from this enum value. The enum is converted to the platform-specific
 * variation value when applied to the input method context.
 *
 * @SINCE_2_5.27
 */
enum class PanelLayoutVariation
{
  NORMAL_NORMAL                       = 0,   ///< The plain normal variation for normal layout. @SINCE_2_5.27
  NORMAL_WITH_FILENAME                = 1,   ///< Normal layout variation for filename input. Symbols such as '/' should be disabled. @SINCE_2_5.27
  NORMAL_WITH_PERSON_NAME             = 2,   ///< Normal layout variation for person name input. @SINCE_2_5.27
  NUMBER_ONLY_NORMAL                  = 100, ///< The plain normal variation for number-only layout. @SINCE_2_5.27
  NUMBER_ONLY_WITH_SIGNED             = 101, ///< Number-only layout variation to allow a positive or negative sign at the start. @SINCE_2_5.27
  NUMBER_ONLY_WITH_DECIMAL            = 102, ///< Number-only layout variation to allow a decimal point for fractional values. @SINCE_2_5.27
  NUMBER_ONLY_WITH_SIGNED_AND_DECIMAL = 103, ///< Number-only layout variation to allow both sign and decimal point. @SINCE_2_5.27
  PASSWORD_NORMAL                     = 200, ///< The normal variation for password layout. @SINCE_2_5.27
  PASSWORD_WITH_NUMBER_ONLY           = 201  ///< Password layout variation to allow only numbers. @SINCE_2_5.27
};

} // namespace InputMethod

/**
 * @}
 */
} // namespace Dali

#endif // DALI_INPUT_METHOD_H
