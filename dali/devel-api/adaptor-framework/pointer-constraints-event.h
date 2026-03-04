#ifndef DALI_WINDOW_DEVEL_POINTER_CONSTRAINTS_EVENT_H
#define DALI_WINDOW_DEVEL_POINTER_CONSTRAINTS_EVENT_H

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
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace DevelWindow
{
/**
 * @brief PointerConstraintsEvent occurs when pointer is locked/unlocked.
 *
 * A signal is emitted when pointer is locked/unlocked.
 */
struct DALI_ADAPTOR_API PointerConstraintsEvent
{
  /**
   * @brief Constructor which creates a PointerConstraintsEvent instance
   * @param[in] x The x coordinate relative to window where event happened
   * @param[in] y The y coordinate relative to window where event happened
   * @param[in] locked The status whether pointer is locked/unlocked
   * @param[in] confined The status whether pointer is confined/unconfined
   */
  PointerConstraintsEvent(int32_t x, int32_t y, bool locked, bool confined)
  : x(x),
    y(y),
    locked(locked),
    confined(confined)
  {
  }

  int32_t x;
  int32_t y;
  bool    locked;
  bool    confined;
};

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_POINTER_CONSTRAINTS_EVENT_H
