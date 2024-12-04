#ifndef DALI_GRAPHICS_BACKEND_H
#define DALI_GRAPHICS_BACKEND_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

namespace Graphics
{
enum class Backend
{
  GLES,   ///< The graphics backend uses GLES. @SINCE_2_3.53
  VULKAN, ///< The graphics backend uses VULKAN. @SINCE_2_3.53

  DEFAULT = GLES, ///< The default graphics backend. @SINCE_2_3.53
};

/**
 * @brief Returns the graphics backend currently in use.
 * @SINCE_2_3.53
 * @return The graphics backend currently in use.
 */
DALI_ADAPTOR_API Backend GetCurrentGraphicsBackend();

/**
 * @brief Sets the graphics backend.
 * @details Generally, calling this is not required and can be set using the environment variable (DALI_GRAPHICS_BACKEND) instead.
 *          If called before the Application class is created, then the environment variable is ignored.
 * @SINCE_2_3.53
 * @param[in] backend The graphics backend to use
 * @note This can only be called once and only before the graphics backend has been created (i.e. before the Application Class is started).
 *       If called again or after the graphics backend has started, then the call will not change anything.
 * @note Currently has no effect as the Graphics backend is chosen at compile time
 */
DALI_ADAPTOR_API void SetGraphicsBackend(Backend backend);

} // namespace Graphics

/**
 * @}
 */
} // namespace Dali

#endif // DALI_GRAPHICS_BACKEND_H
