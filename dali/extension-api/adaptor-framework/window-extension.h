#ifndef DALI_WINDOW_EXTENSION_H
#define DALI_WINDOW_EXTENSION_H

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
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/object/any.h>

namespace Dali
{
class Window;

namespace Extension
{
/**
 * @brief Get the native window handle of the given Window.
 *
 * This returns an opaque handle to the underlying native window/surface object as a void* pointer,
 * wrapped in an Any container. This allows access to platform-specific window properties if needed.
 *
 * The returned value is always stored as a void* pointer, unlike GetNativeHandle() which may use
 * different representations.
 *
 * @param[in] window The window object
 * @return An Any object containing the native window handle as void*, or an empty Any if the handle
 *         is not available (e.g., if the window is not yet created or has been destroyed)
 *
 * @warning The returned void* is only valid as long as the Window object remains valid.
 *          Using the handle after the Window is destroyed leads to undefined behavior.
 */
DALI_ADAPTOR_API Any GetWindowNativeHandle(Window window);

} // namespace Extension
} // namespace Dali

#endif // DALI_WINDOW_EXTENSION_H
