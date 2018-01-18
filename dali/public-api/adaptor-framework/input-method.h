#ifndef __DALI_INPUT_MEHTOD_H__
#define __DALI_INPUT_MEHTOD_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

/**
 * @brief Enumeration for input method.
 * @SINCE_1_0.0
 */
namespace InputMethod
{
/**
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
