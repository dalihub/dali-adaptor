#ifndef DALI_ADAPTOR_EXTENSION_H
#define DALI_ADAPTOR_EXTENSION_H

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
#include <dali/public-api/common/dali-vector.h>
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Extension
{
/**
 * @brief Get the render thread ID of the adaptor.
 *
 * @return The render thread ID. Returns a valid thread ID only if the render thread has started
 *         and the system supports thread ID retrieval. Otherwise, returns 0.
 */
DALI_ADAPTOR_API int32_t GetRenderThreadId();

/**
 * @brief Get the list of windows created by the adaptor.
 *
 * @return The list of windows. Returns an empty list if no window has been created yet.
 */
DALI_ADAPTOR_API Dali::Vector<Window> GetWindows();

} // namespace Extension
} // namespace Dali

#endif // DALI_ADAPTOR_EXTENSION_H
