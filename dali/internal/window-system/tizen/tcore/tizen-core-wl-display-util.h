#ifndef DALI_INTERNAL_WINDOWSYSTEM_TCOREWL_TIZEN_CORE_WL_DISPLAY_UTIL_H
#define DALI_INTERNAL_WINDOWSYSTEM_TCOREWL_TIZEN_CORE_WL_DISPLAY_UTIL_H

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

#include <tizen_core_wl.h>

namespace Dali::Internal::Adaptor
{

/**
 * @brief Creates a display handle and connects to the Wayland compositor.
 *
 * When another component already connected to the same display name,
 * tizen-core-wl reuses the cached connection and increments the reference count.
 *
 * @param[out] display The connected display handle (nullptr on failure).
 * @return true on success, false otherwise.
 */
inline bool TcoreWlAcquireDisplay(tizen_core_wl_display_h* display)
{
  if(!display)
  {
    return false;
  }

  *display = nullptr;
  if(tizen_core_wl_display_create(display) != TIZEN_CORE_WL_ERROR_NONE || !*display)
  {
    return false;
  }

  if(tizen_core_wl_display_connect(*display, nullptr) != TIZEN_CORE_WL_ERROR_NONE)
  {
    tizen_core_wl_display_destroy(*display);
    *display = nullptr;
    return false;
  }

  return true;
}

/**
 * @brief Disconnects and destroys a display handle acquired via TcoreWlAcquireDisplay().
 */
inline void TcoreWlReleaseDisplay(tizen_core_wl_display_h display)
{
  if(display)
  {
    tizen_core_wl_display_disconnect(display);
    tizen_core_wl_display_destroy(display);
  }
}

} // namespace Dali::Internal::Adaptor

#endif // DALI_INTERNAL_WINDOWSYSTEM_TCOREWL_TIZEN_CORE_WL_DISPLAY_UTIL_H
