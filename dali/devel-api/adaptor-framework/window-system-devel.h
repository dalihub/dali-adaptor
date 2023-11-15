#ifndef DALI_WINDOW_SYSTEM_DEVEL_H
#define DALI_WINDOW_SYSTEM_DEVEL_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace DevelWindowSystem
{
/**
 * @brief Get the screen size.
 *
 * This function gets the screen size.
 *
 * @param[out] width The width of the screen
 * @param[out] height The height of the screen
 */
DALI_ADAPTOR_API void GetScreenSize(int32_t& width, int32_t& height);

/**
 * @brief Sets the processes using geometry event propagation touch and hover events.
 *
 * @param[in] enabled True if the processes using geometry event propagation touch and hover events.
 */
DALI_ADAPTOR_API void SetGeometryHittestEnabled(bool enabled);

/**
 * @brief Queries whether the scene using geometry event propagation touch and hover events.
 *
 * @return True if the scene using geometry event propagation touch and hover events.
 */
DALI_ADAPTOR_API bool IsGeometryHittestEnabled();

} // namespace DevelWindowSystem

} // namespace Dali

#endif // DALI_WINDOW_SYSTEM_DEVEL_H
