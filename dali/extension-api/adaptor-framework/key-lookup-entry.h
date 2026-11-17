#ifndef DALI_KEY_LOOKUP_ENTRY_H
#define DALI_KEY_LOOKUP_ENTRY_H

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
#include <dali/public-api/common/dali-namespace.h>

namespace DALI_NAMESPACE
{
namespace Extension
{
/**
 * @brief A single entry describing how a platform key name maps to a Dali key code.
 *
 * Leave the reserved members at zero. Aggregate initialization that lists only the
 * first three members does this.
 * @SINCE_3_0.0
 */
struct KeyLookupEntry
{
  const char* keyName;       ///< XF86 key name
  int         daliKeyCode;   ///< Dali key code
  bool        deviceButton;  ///< Whether the key is from a button on the device
  uint8_t     reserved1{0u}; ///< Reserved for future use. Must be zero.
  uint16_t    reserved2{0u}; ///< Reserved for future use. Must be zero.
};

// Tables of entries cross the library boundary as arrays, so the entry size is
// compiled into every table. The reserved members fill what would otherwise be
// padding; give them a meaning instead of adding members.
static_assert(sizeof(KeyLookupEntry) == sizeof(const char*) + sizeof(int) + 4u, "KeyLookupEntry size is part of the ABI");

} // namespace Extension
} //namespace DALI_NAMESPACE

#endif // DALI_KEY_LOOKUP_ENTRY_H
