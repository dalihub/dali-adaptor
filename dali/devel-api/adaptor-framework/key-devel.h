#ifndef DALI_KEY_DEVEL_H
#define DALI_KEY_DEVEL_H

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
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/extension-api/adaptor-framework/key-lookup-entry.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
// Expose KeyLookupEntry as a convenience (defined in extension-api)
using KeyLookupEntry = Dali::Extension::KeyLookupEntry;

namespace DevelKey
{
/**
 * @brief Get the key code from a key name.
 * @param[in] keyName The key name
 * @return The key code. -1 if the daliKey does not exist in the supported key lookup table.
 */
DALI_ADAPTOR_API int GetDaliKeyCode(const char* keyName);

/**
 * @brief Makes the key code resolved by the window system take priority over the key look up table.
 *
 * Every backend resolves the key code of an incoming key event by consulting the key look up table
 * first and falling back to the code the platform reports, so that a named key such as XF86Back
 * arrives as DALI_KEY_BACK everywhere.
 *
 * The Tizen backends used to do the opposite: the code the window system resolved won, and the
 * look up table was only a fallback. On a device whose keymap does not agree with the table, that
 * produced a different key code than the other backends for the very same key.
 *
 * This exists solely so that a framework carrying applications written against that old behaviour
 * can keep it. It is not intended for applications, and it must be called before the adaptor
 * starts delivering key events.
 *
 * @param[in] preferSystemKeyCode True to restore the legacy Tizen order, false for the default
 *                                order shared by every backend.
 */
DALI_ADAPTOR_API void SetSystemKeyCodePriority(bool preferSystemKeyCode);

/**
 * @brief Retrieves whether the key code resolved by the window system takes priority.
 *
 * @return True if the legacy Tizen order is in effect, false otherwise.
 */
DALI_ADAPTOR_API bool IsSystemKeyCodePriority();

} // namespace DevelKey

} // namespace Dali

#endif // DALI_KEY_DEVEL_H
