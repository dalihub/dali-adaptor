#ifndef DALI_INTERNAL_WINDOW_SYSTEM_TIZEN_KEYMAP_H
#define DALI_INTERNAL_WINDOW_SYSTEM_TIZEN_KEYMAP_H

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
#include <dali/public-api/common/dali-namespace.h>
#include <xkbcommon/xkbcommon.h>
#include <cstdint>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
namespace WindowSystem
{
/**
 * @brief Resolves a keysym name using the first symbol in layout zero, level zero.
 *
 * @param[in] keymap The system keymap
 * @param[in] keyName The case-sensitive XKB keysym name
 * @param[out] keyCode The first matching key code, unchanged on failure
 * @return True if a nonzero key code was found, false otherwise
 */
inline bool GetKeyCodeFromXkbKeymap(xkb_keymap* keymap, const char* keyName, uint32_t& keyCode)
{
  if(!keymap || !keyName || !*keyName)
  {
    return false;
  }

  const xkb_keysym_t keySym = xkb_keysym_from_name(keyName, XKB_KEYSYM_NO_FLAGS);
  if(keySym == XKB_KEY_NoSymbol)
  {
    return false;
  }

  struct KeyCodeMap
  {
    xkb_keysym_t  keySym;
    xkb_keycode_t keyCode;
    bool          found;
  };
  KeyCodeMap result{keySym, 0u, false};

  xkb_keymap_key_for_each(keymap, [](xkb_keymap* map, xkb_keycode_t key, void* data)
  {
    auto& keyCodeMap = *static_cast<KeyCodeMap*>(data);
    if(!keyCodeMap.found)
    {
      const xkb_keysym_t* symbols = nullptr;
      if(xkb_keymap_key_get_syms_by_level(map, key, 0, 0, &symbols) > 0 && symbols && symbols[0] == keyCodeMap.keySym)
      {
        keyCodeMap.keyCode = key;
        keyCodeMap.found   = true;
      }
    }
  }, &result);

  if(result.found && result.keyCode != 0u)
  {
    keyCode = result.keyCode;
    return true;
  }
  return false;
}

} // namespace WindowSystem
} // namespace Adaptor
} // namespace Internal
} // namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_WINDOW_SYSTEM_TIZEN_KEYMAP_H
