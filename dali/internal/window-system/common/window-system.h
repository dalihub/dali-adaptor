#ifndef DALI_INTERNAL_WINDOW_SYSTEM_COMMON_WINDOW_SYSTEM_H
#define DALI_INTERNAL_WINDOW_SYSTEM_COMMON_WINDOW_SYSTEM_H

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

// EXTERNAL_HEADERS
#include <dali/public-api/object/any.h>
#include <dali/public-api/signals/dali-signal.h>
#include <cstdint>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/screen-information.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace WindowSystem
{
/**
 * @brief Initialize a window system
 */
void Initialize();

/**
 * @brief Shutdown a window system
 */
void Shutdown();

/**
 * @brief Set the DPI of the target screen.
 *
 * @note Multiple screens are not currently supported.
 * @param[in] horizontalDpi The horizontal resolution in DPI.
 * @param[in] verticalDpi The vertical resolution in DPI.
 */
void SetDpi(uint32_t dpiHorizontal, uint32_t dpiVertical);

/**
 * @brief Retrieves the DPI previously set to the target screen.
 *
 * @note Multiple screens are not currently supported.
 * @param[out] horizontalDpi The horizontal resolution in DPI.
 * @param[out] verticalDpi The vertical resolution in DPI.
 */
void GetDpi(uint32_t& dpiHorizontal, uint32_t& dpiVertical);

/**
 * @brief Get the screen size
 *
 * Needs exporting as it's called directly by the GlWindow library
 */
void GetScreenSize(int32_t& width, int32_t& height);

/**
 * Get the list of screen informations for this window system.
 * It is for multiple screen environment.
 *
 * @return The list of the screen information
 */
std::vector<Dali::ScreenInformation> GetAvailableScreens();

/**
 * @brief Update the screen size
 * @note The screen size may be updated while application is running. So update the stored size.
 */
void UpdateScreenSize();

/**
 * @copydoc Dali::Keyboard::SetRepeatInfo()
 */
bool SetKeyboardRepeatInfo(float rate, float delay);

/**
 * @copydoc Dali::Keyboard::GetRepeatInfo()
 */
bool GetKeyboardRepeatInfo(float& rate, float& delay);

/**
 * @copydoc Dali::Keyboard::SetHorizontalRepeatInfo()
 */
bool SetKeyboardHorizontalRepeatInfo(float rate, float delay);

/**
 * @copydoc Dali::Keyboard::GetHorizontalRepeatInfo()
 */
bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay);

/**
 * @copydoc Dali::Keyboard::SetVerticalRepeatInfo()
 */
bool SetKeyboardVerticalRepeatInfo(float rate, float delay);

/**
 * @copydoc Dali::Keyboard::GetVerticalRepeatInfo()
 */
bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay);

/**
 * @brief Sets whether the processes using geometry event propagation touch and hover events.
 *
 * @param[in] enabled True if the processes using geometry event propagation touch and hover events.
 */
void SetGeometryHittestEnabled(bool enabled);

/**
 * @brief Queries whether the scene using geometry event propagation touch and hover events.
 *
 * @return True if the scene using geometry event propagation touch and hover events.
 */
bool IsGeometryHittestEnabled();

/**
 * @brief Global keyboard repeat settings changed signal
 */
using KeyboardRepeatSettingsChangedSignalType = Dali::Signal<void()>;

/**
 * @brief Returns a signal that is emitted when the keyboard repeat settings are changed globally.
 *
 * @return The signal to connect to
 */
KeyboardRepeatSettingsChangedSignalType& KeyboardRepeatSettingsChangedSignal();

} // namespace WindowSystem

/**
 * Base class for window system. The minimal set of events and properties that should be received/set on the window
 */
class WindowSystemBase
{
public:
  /**
   * Get the screen size for this window system.
   *
   * @param[out] width The width of the screen
   * @param[out] height The height of the screen
   */
  virtual void GetScreenSize(int32_t& width, int32_t& height) = 0;

  /**
   * Get the list of screen informations for this window system.
   * It is for multiple screen environment.
   *
   * @return The list of the screen information
   * @note The default implementation returns an empty list (single-screen environments).
   *       Backends that support multiple screens (e.g. Tizen) override this.
   */
  virtual std::vector<Dali::ScreenInformation> GetAvailableScreens()
  {
    return {};
  }

  /**
   * @brief Update the stored screen size.
   * @note The screen size may change while the application is running.
   *       The default implementation does nothing.
   */
  virtual void UpdateScreenSize()
  {
  }

  /**
   * @copydoc Dali::Keyboard::SetRepeatInfo()
   * @note The default implementation does nothing and returns false. Backends that support
   *       keyboard repeat (e.g. Tizen) override this.
   */
  virtual bool SetKeyboardRepeatInfo(float rate, float delay)
  {
    return false;
  }

  /**
   * @copydoc Dali::Keyboard::GetRepeatInfo()
   */
  virtual bool GetKeyboardRepeatInfo(float& rate, float& delay)
  {
    return false;
  }

  /**
   * @copydoc Dali::Keyboard::SetHorizontalRepeatInfo()
   */
  virtual bool SetKeyboardHorizontalRepeatInfo(float rate, float delay)
  {
    return false;
  }

  /**
   * @copydoc Dali::Keyboard::GetHorizontalRepeatInfo()
   */
  virtual bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay)
  {
    return false;
  }

  /**
   * @copydoc Dali::Keyboard::SetVerticalRepeatInfo()
   */
  virtual bool SetKeyboardVerticalRepeatInfo(float rate, float delay)
  {
    return false;
  }

  /**
   * @copydoc Dali::Keyboard::GetVerticalRepeatInfo()
   */
  virtual bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay)
  {
    return false;
  }

  /**
   * @brief Sets whether geometry event propagation is used for touch and hover events.
   *
   * @param[in] enabled True if geometry event propagation should be used.
   * @note The default implementation propagates the setting to every scene holder.
   *       The behaviour is identical across all backends, so backends should not need to override this.
   */
  virtual void SetGeometryHittestEnabled(bool enabled);

  /**
   * @brief Queries whether geometry event propagation is enabled.
   *
   * @return True if geometry event propagation is enabled.
   */
  bool IsGeometryHittestEnabled() const
  {
    return mGeometryHittest;
  }

  /**
   * @brief Returns a signal emitted when keyboard repeat settings are changed globally.
   */
  WindowSystem::KeyboardRepeatSettingsChangedSignalType& KeyboardRepeatSettingsChangedSignal()
  {
    return mKeyboardRepeatSettingsChangedSignal;
  }

protected:
  WindowSystem::KeyboardRepeatSettingsChangedSignalType mKeyboardRepeatSettingsChangedSignal;
  bool                                                  mGeometryHittest{false}; ///< Whether geometry event propagation is enabled. Shared by all backends.
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOW_SYSTEM_COMMON_WINDOW_SYSTEM_H
