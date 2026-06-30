#ifndef DALI_WINDOW_DEFINITIONS_H
#define DALI_WINDOW_DEFINITIONS_H

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
#include <dali/public-api/common/extents.h>
#include <dali/public-api/common/type-traits.h>
#include <dali/public-api/common/unique-ptr.h>
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

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
 * The fuction is "New(int* argc, char** argv[], Dali::StringView stylesheet, Application::WindowOpacity windowOpacity, PositionSize positionSize, WindowType type)".
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
  DESKTOP       ///< Used for desktop windows. @SINCE_2_2.1
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
 * @SINCE_2_0.0
 */
enum class WindowOperationResult
{
  UNKNOWN_ERROR = 0, ///< Failed for unknown reason. @SINCE_2_0.0
  SUCCEED,           ///< Succeed. @SINCE_2_0.0
  PERMISSION_DENIED, ///< Permission denied. @SINCE_2_0.0
  NOT_SUPPORTED,     ///< The operation is not supported. @SINCE_2_0.0
  INVALID_OPERATION, ///< The operation is invalid. (e.g. Try to operate to the wrong window) @SINCE_2_0.0
};

/**
 * @brief Enumeration of direction for window resized by display server.
 * @SINCE_2_0.0
 */
enum class WindowResizeDirection
{
  TOP_LEFT     = 1, ///< Start resizing window to the top-left edge. @SINCE_2_0.0
  TOP          = 2, ///< Start resizing window to the top side. @SINCE_2_0.0
  TOP_RIGHT    = 3, ///< Start resizing window to the top-right edge. @SINCE_2_0.0
  LEFT         = 4, ///< Start resizing window to the left side. @SINCE_2_0.0
  RIGHT        = 5, ///< Start resizing window to the right side. @SINCE_2_0.0
  BOTTOM_LEFT  = 6, ///< Start resizing window to the bottom-left edge. @SINCE_2_0.0
  BOTTOM       = 7, ///< Start resizing window to the bottom side. @SINCE_2_0.0
  BOTTOM_RIGHT = 8  ///< Start resizing window to the bottom-right edge. @SINCE_2_0.0
};

/**
 * @brief Enumeration of window blur type.
 * @SINCE_2_5.28
 */
enum class WindowBlurType
{
  NONE = 0,   ///< Default value for blur disable @SINCE_2_5.28
  BACKGROUND, ///< The window's background @SINCE_2_5.28
  BEHIND,     ///< The window's behind except background @SINCE_2_5.28
};

/**
 * @brief Enumeration of window insets part type.
 * @SINCE_2_5.28
 */
enum class WindowInsetsPartType
{
  STATUS_BAR = 0, ///< Status bar @SINCE_2_5.28
  KEYBOARD,       ///< Keyboard @SINCE_2_5.28
  CLIPBOARD,      ///< Clipboard @SINCE_2_5.28
};

/**
 * @brief Enumeration of window insets part state.
 * @SINCE_2_5.28
 */
enum class WindowInsetsPartState
{
  INVISIBLE = 0, ///< Invisible @SINCE_2_5.28
  VISIBLE,       ///< Visible @SINCE_2_5.28
};

/**
 * @brief Enumeration of window insets part flag.
 * @SINCE_2_5.28
 */
enum class WindowInsetsPartFlags
{
  NONE       = 0,
  STATUS_BAR = 1 << 0, ///< Status bar @SINCE_2_5.28
  KEYBOARD   = 1 << 1, ///< Keyboard @SINCE_2_5.28
  CLIPBOARD  = 1 << 2, ///< Clipboard @SINCE_2_5.28
};

/**
 * @brief Bitwise OR operator for WindowInsetsPartFlags.
 * @SINCE_2_5.28
 * @param[in] a The first flag
 * @param[in] b The second flag
 * @return The bitwise OR of the two flags
 */
inline WindowInsetsPartFlags operator|(WindowInsetsPartFlags a, WindowInsetsPartFlags b)
{
  return static_cast<WindowInsetsPartFlags>(static_cast<GetUnderlyingType<WindowInsetsPartFlags>::type>(a) | static_cast<GetUnderlyingType<WindowInsetsPartFlags>::type>(b));
}

/**
 * @brief Bitwise AND operator for WindowInsetsPartFlags.
 * @SINCE_2_5.28
 * @param[in] a The first flag
 * @param[in] b The second flag
 * @return The bitwise AND of the two flags
 */
inline WindowInsetsPartFlags operator&(WindowInsetsPartFlags a, WindowInsetsPartFlags b)
{
  return static_cast<WindowInsetsPartFlags>(static_cast<GetUnderlyingType<WindowInsetsPartFlags>::type>(a) & static_cast<GetUnderlyingType<WindowInsetsPartFlags>::type>(b));
}

/**
 * @brief Check if a flag is set in the given value.
 * @SINCE_2_5.28
 * @param[in] value The flags value to check
 * @param[in] flag The flag to check for
 * @return True if the flag is set, false otherwise
 */
inline bool HasFlag(WindowInsetsPartFlags value, WindowInsetsPartFlags flag)
{
  return static_cast<GetUnderlyingType<WindowInsetsPartFlags>::type>(value & flag) != 0;
}

/**
 * @brief WindowInsetsInfo contains information about window insets.
 * @SINCE_2_5.28
 */
class DALI_ADAPTOR_API WindowInsetsInfo
{
public:
  /**
   * @brief Constructor.
   * @SINCE_2_5.28
   */
  WindowInsetsInfo();

  /**
   * @brief Constructor with insets information.
   * @SINCE_2_5.28
   * @param[in] partType The part type
   * @param[in] partState The part state
   * @param[in] extents The extents
   */
  WindowInsetsInfo(WindowInsetsPartType partType, WindowInsetsPartState partState, const Extents& extents);

  /**
   * @brief Destructor.
   * @SINCE_2_5.28
   */
  ~WindowInsetsInfo();

  /**
   * @brief Copy constructor.
   * @SINCE_2_5.28
   */
  WindowInsetsInfo(const WindowInsetsInfo& other);

  /**
   * @brief Copy assignment operator.
   * @SINCE_2_5.28
   */
  WindowInsetsInfo& operator=(const WindowInsetsInfo& other);

  /**
   * @brief Move constructor.
   * @SINCE_2_5.28
   */
  WindowInsetsInfo(WindowInsetsInfo&& other) noexcept;

  /**
   * @brief Move assignment operator.
   * @SINCE_2_5.28
   */
  WindowInsetsInfo& operator=(WindowInsetsInfo&& other) noexcept;

  /**
   * @brief Gets the part type of the window insets.
   * @SINCE_2_5.28
   * @return The part type
   */
  WindowInsetsPartType GetPartType() const;

  /**
   * @brief Gets the part state of the window insets.
   * @SINCE_2_5.28
   * @return The part state
   */
  WindowInsetsPartState GetPartState() const;

  /**
   * @brief Gets the extents of the window insets.
   * @SINCE_2_5.28
   * @return The extents
   */
  const Extents& GetExtents() const;

private:
  class Impl;
  Dali::UniquePtr<Impl> mImpl;
};

} // namespace Dali

#endif // DALI_WINDOW_DEFINITIONS_H
