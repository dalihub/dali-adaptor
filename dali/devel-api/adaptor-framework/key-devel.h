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

} // namespace DevelKey

} // namespace Dali

#endif // DALI_KEY_DEVEL_H
