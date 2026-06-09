#ifndef DALI_INPUT_METHOD_CONTEXT_H
#define DALI_INPUT_METHOD_CONTEXT_H

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
#include <dali/public-api/common/dali-string.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/input-method.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class InputMethodContext;
}
} //namespace Internal DALI_INTERNAL

/**
 * @brief Provides application-level access to the platform input method context.
 *
 * InputMethodContext allows applications to control input panel behavior, query
 * input panel state, configure input panel options, and receive application-level
 * input method notifications such as private commands.
 *
 * Applications normally obtain an InputMethodContext from a text input control
 * such as InputField or InputEditor instead of creating one directly.
 *
 * @SINCE_2_5.27
 */
class DALI_ADAPTOR_API InputMethodContext : public BaseHandle
{
public:
  /**
   * @brief Enumeration for state of the input panel.
   *
   * @SINCE_2_5.27
   */
  enum class State
  {
    SHOW,     ///< Input panel is shown @SINCE_2_5.27
    HIDE,     ///< Input panel is hidden @SINCE_2_5.27
    WILL_SHOW ///< Input panel is in the process of being shown @SINCE_2_5.27
  };

  /**
   * @brief Enumeration for the type of keyboard.
   *
   * @SINCE_2_5.27
   */
  enum class KeyboardType
  {
    SOFTWARE_KEYBOARD, ///< Software keyboard (virtual keyboard) is default @SINCE_2_5.27
    HARDWARE_KEYBOARD  ///< Hardware keyboard @SINCE_2_5.27
  };

  /**
   * @brief Enumeration for defining the input panel alignment.
   *
   * @SINCE_2_5.27
   */
  enum class InputPanelAlign
  {
    TOP_LEFT,      ///< The top-left corner @SINCE_2_5.27
    TOP_CENTER,    ///< The top-center position @SINCE_2_5.27
    TOP_RIGHT,     ///< The top-right corner @SINCE_2_5.27
    MIDDLE_LEFT,   ///< The middle-left position @SINCE_2_5.27
    MIDDLE_CENTER, ///< The middle-center position @SINCE_2_5.27
    MIDDLE_RIGHT,  ///< The middle-right position @SINCE_2_5.27
    BOTTOM_LEFT,   ///< The bottom-left corner @SINCE_2_5.27
    BOTTOM_CENTER, ///< The bottom-center position @SINCE_2_5.27
    BOTTOM_RIGHT   ///< The bottom-right corner @SINCE_2_5.27
  };

  using ActivatedSignalType              = Signal<void(InputMethodContext)>;                      ///< Keyboard activated signal
  using StatusChangedSignalType          = Signal<void(InputMethodContext, State)>;               ///< Input panel status signal
  using KeyboardTypeChangedSignalType    = Signal<void(InputMethodContext, KeyboardType)>;        ///< Keyboard type signal
  using KeyboardResizedSignalType        = Signal<void(InputMethodContext)>;                      ///< Keyboard resized signal
  using LanguageChangedSignalType        = Signal<void(InputMethodContext)>;                      ///< Language changed signal
  using PrivateCommandReceivedSignalType = Signal<void(InputMethodContext, const Dali::String&)>; ///< Private command received signal

public:
  /**
   * @brief Constructs an empty handle.
   *
   * @SINCE_2_5.27
   */
  InputMethodContext();

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived handle types must not contain data or virtual methods.
   *
   * @SINCE_2_5.27
   */
  ~InputMethodContext();

  /**
   * @brief Copy constructor.
   *
   * @param[in] inputMethodContext InputMethodContext to copy. The copied handle points at the same implementation.
   * @SINCE_2_5.27
   */
  InputMethodContext(const InputMethodContext& inputMethodContext);

  /**
   * @brief Assignment operator.
   *
   * @param[in] inputMethodContext The InputMethodContext to assign from.
   * @return A reference to this handle.
   * @SINCE_2_5.27
   */
  InputMethodContext& operator=(const InputMethodContext& inputMethodContext);

  /**
   * @brief Downcasts a handle to an InputMethodContext handle.
   *
   * If the handle points to an InputMethodContext, the downcast produces a valid handle.
   * Otherwise, the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object.
   * @return Handle to an InputMethodContext or an uninitialized handle.
   * @SINCE_2_5.27
   */
  static InputMethodContext DownCast(BaseHandle handle);

public:
  /**
   * @brief Gets whether the keyboard should be restored after focus is lost and regained.
   *
   * @return True if the keyboard will be restored.
   * @SINCE_2_5.27
   */
  bool IsRestoreAfterFocusLostEnabled() const;

  /**
   * @brief Sets whether the keyboard should be restored after focus is lost and regained.
   *
   * @param[in] enabled True if the keyboard should be restored.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetRestoreAfterFocusLostEnabled(bool enabled);

  /**
   * @brief Provides size and position of the current active input panel.
   *
   * The returned rectangle uses screen coordinates. The x and y values are the
   * top-left position of the input panel on the screen, and width and height
   * are the current size of the input panel.
   *
   * @return Rectangle containing the input panel x, y, width, and height.
   * @SINCE_2_5.27
   */
  Dali::BoundsInteger GetInputPanelArea();

  /**
   * @brief Sets input panel-specific user data to deliver to the input panel.
   *
   * The data is passed to the input panel as an application/input-panel
   * negotiated payload. Its format and meaning are defined by the input panel.
   *
   * @param[in] data The user data to be set to the input panel.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetInputPanelUserData(const Dali::String& data);

  /**
   * @brief Gets input-panel specific user data from the current active input panel.
   *
   * @return The user data from the input panel.
   * @SINCE_2_5.27
   */
  Dali::String GetInputPanelUserData() const;

  /**
   * @brief Gets the state of the current active input panel.
   *
   * @return The state of the input panel.
   * @SINCE_2_5.27
   */
  State GetInputPanelState() const;

  /**
   * @brief Sets whether the return key on the input panel is enabled.
   *
   * @param[in] enabled True if the return key is enabled, false otherwise.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetReturnKeyEnabled(bool enabled);

  /**
   * @brief Gets whether the return key on the input panel is enabled.
   *
   * @return True if the return key is enabled, false otherwise.
   * @SINCE_2_5.27
   */
  bool IsReturnKeyEnabled() const;

  /**
   * @brief Enables showing the input panel automatically when focused.
   *
   * @param[in] enabled If true, the input panel will be shown when focused.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetInputPanelAutoShowEnabled(bool enabled);

  /**
   * @brief Requests to show the input panel.
   *
   * The input panel state may change asynchronously. Use StatusChangedSignal()
   * or GetInputPanelState() to observe the actual state.
   *
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool ShowInputPanel();

  /**
   * @brief Requests to hide the input panel.
   *
   * The input panel state may change asynchronously. Use StatusChangedSignal()
   * or GetInputPanelState() to observe the actual state.
   *
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool HideInputPanel();

  /**
   * @brief Gets the keyboard type.
   *
   * @return The keyboard type.
   * @SINCE_2_5.27
   */
  KeyboardType GetKeyboardType() const;

  /**
   * @brief Sets the current language locale of the input panel.
   *
   * @param[in] locale The language locale to be set to the input panel.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetInputPanelLanguageLocale(const Dali::String& locale);

  /**
   * @brief Gets the current language locale of the input panel.
   *
   * @return The current language locale of the input panel.
   * @SINCE_2_5.27
   */
  Dali::String GetInputPanelLanguageLocale() const;

  /**
   * @brief Sets whether text prediction is enabled.
   *
   * @param[in] enabled Whether to enable text prediction.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetTextPredictionEnabled(bool enabled);

  /**
   * @brief Gets whether text prediction is enabled.
   *
   * @return True if text prediction is enabled.
   * @SINCE_2_5.27
   */
  bool IsTextPredictionEnabled() const;

  /**
   * @brief Sets whether the input method context should be shown in fullscreen mode.
   *
   * @param[in] enabled Whether to enable fullscreen mode.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetFullScreenModeEnabled(bool enabled);

  /**
   * @brief Gets whether the input method context should be shown in fullscreen mode.
   *
   * @return True if fullscreen mode is enabled.
   * @SINCE_2_5.27
   */
  bool IsFullScreenModeEnabled() const;

  /**
   * @brief Sets the x,y coordinates of the input panel.
   *
   * The coordinates are screen coordinates, where x and y specify the desired
   * top-left position of the input panel. This is the same coordinate space
   * used by GetInputPanelArea().
   *
   * The input panel may use this position when the current platform and input
   * panel mode support explicit positioning, such as floating input panel mode.
   *
   * @param[in] x The top-left x coordinate of the input panel.
   * @param[in] y The top-left y coordinate of the input panel.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetInputPanelPosition(uint32_t x, uint32_t y);

  /**
   * @brief Sets the alignment and x,y coordinates of the input panel.
   *
   * @param[in] x The x coordinate of the InputPanelAlign value.
   * @param[in] y The y coordinate of the InputPanelAlign value.
   * @param[in] align One of the InputPanelAlign values specifying the desired alignment.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetInputPanelPositionAlign(int32_t x, int32_t y, InputPanelAlign align);

  /**
   * @brief Sets the input panel layout.
   *
   * @param[in] layout The input panel layout.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetInputPanelLayout(Dali::InputMethod::PanelLayout layout);

  /**
   * @brief Gets the input panel layout.
   *
   * @return The input panel layout.
   * @SINCE_2_5.27
   */
  Dali::InputMethod::PanelLayout GetInputPanelLayout() const;

  /**
   * @brief Sets the input panel return key type.
   *
   * @param[in] type The input panel return key type.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetInputPanelReturnKeyType(Dali::InputMethod::ReturnKeyType type);

  /**
   * @brief Gets the input panel return key type.
   *
   * @return The input panel return key type.
   * @SINCE_2_5.27
   */
  Dali::InputMethod::ReturnKeyType GetInputPanelReturnKeyType() const;

  /**
   * @brief Sets the input panel auto-capitalization type.
   *
   * @param[in] type The input panel auto-capitalization type.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetInputPanelAutoCapitalType(Dali::InputMethod::AutoCapitalType type);

  /**
   * @brief Gets the input panel auto-capitalization type.
   *
   * @return The input panel auto-capitalization type.
   * @SINCE_2_5.27
   */
  Dali::InputMethod::AutoCapitalType GetInputPanelAutoCapitalType() const;

  /**
   * @brief Sets the input panel layout variation.
   *
   * The platform interprets the variation according to the selected input panel
   * layout.
   *
   * @param[in] variation The input panel layout variation.
   * @return True if the request was accepted by the platform, false otherwise.
   * @SINCE_2_5.27
   */
  bool SetInputPanelLayoutVariation(Dali::InputMethod::PanelLayoutVariation variation);

  /**
   * @brief Gets the input panel layout variation.
   *
   * @return The input panel layout variation.
   * @SINCE_2_5.27
   */
  Dali::InputMethod::PanelLayoutVariation GetInputPanelLayoutVariation() const;

public:
  // Signals

  /**
   * @brief Emitted when the virtual keyboard is connected or the hardware keyboard is activated.
   *
   * @return The InputMethodContext activated signal.
   * @SINCE_2_5.27
   */
  ActivatedSignalType& ActivatedSignal();

  /**
   * @brief Emitted when the input panel state changes.
   *
   * The signal provides the input method context and the new input panel state.
   *
   * @return The signal to connect to.
   * @SINCE_2_5.27
   */
  StatusChangedSignalType& StatusChangedSignal();

  /**
   * @brief Emitted when the input panel geometry changes.
   *
   * Use GetInputPanelArea() on the provided context to query the current
   * input panel geometry.
   *
   * @return The signal to connect to.
   * @SINCE_2_5.27
   */
  KeyboardResizedSignalType& ResizedSignal();

  /**
   * @brief Emitted when the input panel language changes.
   *
   * @return The signal to connect to.
   * @SINCE_2_5.27
   */
  LanguageChangedSignalType& LanguageChangedSignal();

  /**
   * @brief Emitted when the keyboard type changes.
   *
   * The signal provides the input method context and the new keyboard type.
   *
   * @return The signal to connect to.
   * @SINCE_2_5.27
   */
  KeyboardTypeChangedSignalType& KeyboardTypeChangedSignal();

  /**
   * @brief Emitted when a private command is received from the input panel.
   *
   * @return The signal to connect to.
   * @SINCE_2_5.27
   */
  PrivateCommandReceivedSignalType& PrivateCommandReceivedSignal();

public:
  /**
   * @brief This constructor is used by integration APIs and internal implementations.
   *
   * @param[in] inputMethodContext A pointer to the internal InputMethodContext.
   * @SINCE_2_5.27
   */
  explicit DALI_INTERNAL InputMethodContext(Internal::Adaptor::InputMethodContext* inputMethodContext);
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_INPUT_METHOD_CONTEXT_H
