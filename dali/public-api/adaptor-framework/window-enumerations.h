#ifndef DALI_WINDOW_ENUMERATIONS_H
#define DALI_WINDOW_ENUMERATIONS_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

namespace Dali
{
/**
 * @brief Enumeration for orientation of the window is the way in which a rectangular page is oriented for normal viewing.
 *
 * This Enumeration is used the available orientation APIs and the preferred orientation.
 *
 * @SINCE_2_0.0
 */
enum class WindowOrientation
{
  PORTRAIT                  = 0,   ///< Portrait orientation. The height of the display area is greater than the width. @SINCE_2_0.0
  LANDSCAPE                 = 90,  ///< Landscape orientation. A wide view area is needed. @SINCE_2_0.0
  PORTRAIT_INVERSE          = 180, ///< Portrait inverse orientation. @SINCE_2_0.0
  LANDSCAPE_INVERSE         = 270, ///< Landscape inverse orientation. @SINCE_2_0.0
  NO_ORIENTATION_PREFERENCE = -1   ///< No orientation. It is used to initialize or unset the preferred orientation. @SINCE_2_0.0
};

/**
 * @brief An enum of Window types.
 * Window type has effect of DALi window's behavior, window's stack and extra functions.
 * The default window type is NORMAL. If application does not set the specific window type, this type will be set.
 *
 * Ime window type is special type. It can only set by one Application::New function.
 * The fuction is "New(int* argc, char** argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize, WindowType type)".
 * Ime window type can not set by Window::SetType().
 *
 * @SINCE_2_0.0
 */
enum class WindowType
{
  NORMAL,       ///< A default window type. Indicates a normal, top-level window. Almost every window will be created with this type. @SINCE_2_0.0
  NOTIFICATION, ///< A notification window, like a warning about battery life or a new E-Mail received. @SINCE_2_0.0
  UTILITY,      ///< A persistent utility window, like a toolbox or palette. @SINCE_2_0.0
  DIALOG,       ///< Used for simple dialog window. @SINCE_2_0.0
  IME,          ///< Used for Ime keyboard window. It should be set in application New function. @SINCE_2_0.33
  DESKTOP       ///< Used for desktop windows.
};

/**
 * @brief An enum of notification level.
 * @SINCE_2_0.0
 */
enum class WindowNotificationLevel
{
  NONE   = -1, ///< No notification level. Default level. This value makes the notification window place in the layer of the normal window. @SINCE_2_0.0
  BASE   = 10, ///< Base notification level. @SINCE_2_0.0
  MEDIUM = 20, ///< Higher notification level than base. @SINCE_2_0.0
  HIGH   = 30, ///< Higher notification level than medium. @SINCE_2_0.0
  TOP    = 40  ///< The highest notification level. @SINCE_2_0.0
};

/**
 * @brief An enum of screen mode.
 * @SINCE_2_0.0
 */
enum class WindowScreenOffMode
{
  TIMEOUT, ///< The mode which turns the screen off after a timeout. @SINCE_2_0.0
  NEVER,   ///< The mode which keeps the screen turned on. @SINCE_2_0.0
};

/**
 * @brief Enumeration for transition effect's state.
 * @SINCE_2_0.0
 */
enum class WindowEffectState
{
  NONE = 0, ///< None state. @SINCE_2_0.0
  START,    ///< Transition effect is started. @SINCE_2_0.0
  END       ///< Transition effect is ended. @SINCE_2_0.0
};

/**
 * @brief Enumeration for transition effect's type.
 * @SINCE_2_0.0
 */
enum class WindowEffectType
{
  NONE = 0, ///< None type. @SINCE_2_0.0
  SHOW,     ///< Window show effect. @SINCE_2_0.0
  HIDE,     ///< Window hide effect. @SINCE_2_0.0
};

/**
 * @brief An enum of window operation result.
 */
enum class WindowOperationResult
{
  UNKNOWN_ERROR = 0, ///< Failed for unknown reason.
  SUCCEED,           ///< Succeed.
  PERMISSION_DENIED, ///< Permission denied.
  NOT_SUPPORTED,     ///< The operation is not supported.
  INVALID_OPERATION, ///< The operation is invalid. (e.g. Try to operate to the wrong window)
};

/**
 * @brief Enumeration of direction for window resized by display server.
 */
enum class WindowResizeDirection
{
  TOP_LEFT     = 1, ///< Start resizing window to the top-left edge.
  TOP          = 2, ///< Start resizing window to the top side.
  TOP_RIGHT    = 3, ///< Start resizing window to the top-right edge.
  LEFT         = 4, ///< Start resizing window to the left side.
  RIGHT        = 5, ///< Start resizing window to the right side.
  BOTTOM_LEFT  = 6, ///< Start resizing window to the bottom-left edge.
  BOTTOM       = 7, ///< Start resizing window to the bottom side.
  BOTTOM_RIGHT = 8  ///< Start resizing window to the bottom-right edge.
};

} // namespace Dali

#endif // DALI_WINDOW_ENUMERATIONS_H
