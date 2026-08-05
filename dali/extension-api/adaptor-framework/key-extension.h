#ifndef DALI_KEY_EXTENSION_H
#define DALI_KEY_EXTENSION_H

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
namespace Extension
{
/**
 * @brief Registers an extension key lookup table directly, without loading a key extension plugin (.so).
 *
 * This is an alternative to the plugin; only one source is used at a time.
 * The given table is deep-copied and owned internally, so the caller does not need to keep it alive.
 * If the key extension plugin has already been loaded, this call is ignored.
 *
 * @param[in] table The extension key lookup table
 * @param[in] count The number of entries in the table
 */
DALI_ADAPTOR_API void SetKeyExtensionLookupTable(const Dali::Extension::KeyLookupEntry* table, uint32_t count);

} // namespace Extension
} // namespace Dali

#endif // DALI_KEY_EXTENSION_H
