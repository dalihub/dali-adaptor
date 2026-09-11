#ifndef DALI_WINDOW_SYSTEM_H
#define DALI_WINDOW_SYSTEM_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/math/int-pair.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace DALI_NAMESPACE
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief Window system functions.
 *
 * These functions apply to the window system as a whole rather than to a particular window.
 */
namespace WindowSystem
{
/**
 * @brief Retrieves the size of the main screen.
 *
 * The main screen is the screen a new window is shown on when no screen is specified.
 * This function can be called before any window is created.
 * The size is in pixels and does not change with window rotation.
 *
 * @SINCE_2_5.40
 * @return The size of the main screen in pixels, or a zero size if it cannot be retrieved
 */
DALI_ADAPTOR_API Int32Pair GetMainScreenSize();

} // namespace WindowSystem

/**
 * @}
 */
} // namespace DALI_NAMESPACE

#endif // DALI_WINDOW_SYSTEM_H
