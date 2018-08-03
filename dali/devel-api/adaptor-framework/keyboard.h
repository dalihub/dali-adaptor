#ifndef DALI_KEYBOARD_H
#define DALI_KEYBOARD_H

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
 * @brief Keyboard functions.
 */
namespace Keyboard
{

/**
 * @brief Sets keyboard repeat information.
 *
 * @param[in] rate The key repeat rate value in seconds
 * @param[in] delay The key repeat delay value in seconds
 * @return true if setting the keyboard repeat succeeds
 */
DALI_ADAPTOR_API bool SetRepeatInfo( float rate, float delay );


/**
 * @brief Gets keyboard repeat information.
 *
 * @param[in] rate The key repeat rate value in seconds
 * @param[in] delay The key repeat delay value in seconds
 * @return true if getting the keyboard repeat succeeds, false otherwise
 */
DALI_ADAPTOR_API bool GetRepeatInfo( float& rate, float& delay );

} // namespace Keyboard

/**
 * @}
 */
} // namespace Dali

#endif // DALI_KEYBOARD_H
