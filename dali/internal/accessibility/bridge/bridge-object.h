#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_OBJECT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_OBJECT_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <unordered_map>
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/accessibility/bridge/bridge-base.h>
#include <dali/public-api/math/rect.h>

class BridgeObject : public virtual BridgeBase
{
protected:
  BridgeObject();

  void RegisterInterfaces();

  DBus::DBusInterfaceDescription::SignalId stateChanged;

  void EmitActiveDescendantChanged(Dali::Accessibility::Accessible* obj, Dali::Accessibility::Accessible* child) override;
  void EmitCursorMoved(Dali::Accessibility::Accessible* obj, unsigned int cursorPosition) override;
  void EmitTextChanged(Dali::Accessibility::Accessible* obj, Dali::Accessibility::TextChangedState state, unsigned int position, unsigned int length, const std::string& content) override;
  void EmitStateChanged(Dali::Accessibility::Accessible* obj, Dali::Accessibility::State state, int newValue, int reserved) override;
  void Emit(Dali::Accessibility::Accessible* obj, Dali::Accessibility::WindowEvent event, unsigned int detail) override;
  void Emit(Dali::Accessibility::Accessible* obj, Dali::Accessibility::ObjectPropertyChangeEvent we) override;
  void EmitBoundsChanged(Dali::Accessibility::Accessible* obj, Dali::Rect<> rect) override;
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_OBJECT_H
