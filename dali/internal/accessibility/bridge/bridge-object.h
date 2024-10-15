#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_OBJECT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_OBJECT_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <array>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>
#include <dali/public-api/math/rect.h>

/**
 * @brief The BridgeObject class is to correspond with Dali::Accessibility::Bridge.
 */
class BridgeObject : public virtual BridgeBase
{
protected:
  /**
   * @brief Constructor.
   */
  BridgeObject();

  /**
   * @brief Registers Bridge functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @copydoc Dali::Accessibility::Bridge::EmitActiveDescendantChanged()
   */
  void EmitActiveDescendantChanged(Dali::Accessibility::Accessible* obj, Dali::Accessibility::Accessible* child) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::EmitCursorMoved()
   */
  void EmitCursorMoved(Dali::Accessibility::Accessible* obj, unsigned int cursorPosition) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::EmitTextChanged()
   */
  void EmitTextChanged(Dali::Accessibility::Accessible* obj, Dali::Accessibility::TextChangedState state, unsigned int position, unsigned int length, const std::string& content) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::EmitStateChanged()
   */
  void EmitStateChanged(std::shared_ptr<Dali::Accessibility::Accessible> obj, Dali::Accessibility::State state, int newValue, int reserved) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::Emit()
   */
  void Emit(Dali::Accessibility::Accessible* obj, Dali::Accessibility::WindowEvent event, unsigned int detail = 0) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::Emit()
   */
  void Emit(std::shared_ptr<Dali::Accessibility::Accessible> obj, Dali::Accessibility::ObjectPropertyChangeEvent event) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::EmitBoundsChanged()
   */
  void EmitBoundsChanged(std::shared_ptr<Dali::Accessibility::Accessible> obj, Dali::Rect<> rect) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::EmitMovedOutOfScreen()
   */
  void EmitMovedOutOfScreen(Dali::Accessibility::Accessible* obj, Dali::Accessibility::ScreenRelativeMoveType type) override;

  /**
   * @copydoc Dali::Accessibility::Bridge::EmitSocketAvailable()
   */
  void EmitSocketAvailable(Dali::Accessibility::Accessible* obj) override;

protected:
  DBus::DBusInterfaceDescription::SignalId mStateChanged;
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_OBJECT_H
