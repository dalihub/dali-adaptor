#ifndef DALI_STYLE_CHANGE_H
#define DALI_STYLE_CHANGE_H

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

// EXTERNAL INCLUDES
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief Enumeration for style change information.
 * @SINCE_1_0.0
 */
namespace StyleChange
{

/**
 * @brief Enumeration for StyleChange type.
 * @SINCE_1_0.0
 */
enum Type
{
  DEFAULT_FONT_CHANGE,      ///< Denotes that the default font has changed. @SINCE_1_0.0
  DEFAULT_FONT_SIZE_CHANGE, ///< Denotes that the default font size has changed. @SINCE_1_0.0
  THEME_CHANGE              ///< Denotes that the theme has changed. @SINCE_1_0.0
};

} // namespace StyleChange

/**
 * @}
 */
} // namespace Dali

#endif // DALI_STYLE_CHANGE_H
