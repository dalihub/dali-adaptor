#ifndef DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_EVENTS_H
#define DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_EVENTS_H
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

#include <cstdint>

namespace Dali
{
namespace Devel
{
namespace Accessibility
{

/**
 * @brief Enumeration describing type of object move relative to the screen. Only outgoing moves are signalled to AT-clients.
 */
enum class ScreenRelativeMoveType
{
  OUTSIDE               = 0, ///< Object moves outside of the screen
  OUTGOING_TOP_LEFT     = 1, ///< Object moves out through the top(or left) side of the screen. Maps to: ATSPI_MOVE_OUTED_TOP_LEFT
  OUTGOING_BOTTOM_RIGHT = 2, ///< Object moves out through the bottom(or right) side of the screen. Maps to: ATSPI_MOVE_OUTED_BOTTOM_RIGHT
  INSIDE                     ///< Object moves into the screen or moves inside the screen
};

/**
 * @brief Enumeration describing change of text object.
 */
enum class TextChangedState : uint32_t
{
  INSERTED,
  DELETED,
  MAX_COUNT
};

/**
 * @brief Enumeration describing change of object property.
 */
enum class ObjectPropertyChangeEvent
{
  NAME,
  DESCRIPTION,
  VALUE,
  ROLE,
  PARENT,
};

/**
 * @brief Enumeration describing change of window object.
 */
enum class WindowEvent
{
  PROPERTY_CHANGE,
  MINIMIZE,
  MAXIMIZE,
  RESTORE,
  CLOSE,
  CREATE,
  REPARENT,
  DESKTOP_CREATE,
  DESKTOP_DESTROY,
  DESTROY,
  ACTIVATE,
  DEACTIVATE,
  RAISE,
  LOWER,
  MOVE,
  RESIZE,
  SHADE,
  UU_SHADE,
  RESTYLE,
  POST_RENDER,
};

/**
 * @brief Restored window state.
 */
enum class WindowRestoreType
{
  RESTORE_FROM_ICONIFY, // The Window is Deiconified.
  RESTORE_FROM_MAXIMIZE // The Window is Unmaximized.
};

} // namespace Accessibility
} // namespace Devel
} // namespace Dali

#endif // DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_EVENTS_H
