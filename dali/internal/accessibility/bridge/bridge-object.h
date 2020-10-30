#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_OBJECT_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_OBJECT_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

  void EmitActiveDescendantChanged( Dali::Accessibility::Accessible* obj, Dali::Accessibility::Accessible *child ) override;
  void EmitCaretMoved( Dali::Accessibility::Accessible* obj, unsigned int cursorPosition ) override;
  void EmitTextChanged( Dali::Accessibility::Accessible* obj, Dali::Accessibility::TextChangedState state, unsigned int position, unsigned int length, const std::string &content ) override;
  void EmitStateChanged( Dali::Accessibility::Accessible* obj, Dali::Accessibility::State state, int val1, int val2 ) override;
  void Emit( Dali::Accessibility::Accessible* obj, Dali::Accessibility::WindowEvent we, unsigned int detail1 ) override;
  void Emit( Dali::Accessibility::Accessible* obj, Dali::Accessibility::ObjectPropertyChangeEvent we ) override;
  void EmitBoundsChanged( Dali::Accessibility::Accessible* obj, Dali::Rect<> rect ) override;

public:
  int GetChildCount();
  DBus::ValueOrError< Dali::Accessibility::Accessible* > GetChildAtIndex( int index );
  Dali::Accessibility::Accessible* GetParent();
  DBus::ValueOrError< std::vector< Dali::Accessibility::Accessible* > > GetChildren();
  std::string GetName();
  std::string GetDescription();
  DBus::ValueOrError< uint32_t > GetRole();
  DBus::ValueOrError< std::string > GetRoleName();
  DBus::ValueOrError< std::string > GetLocalizedRoleName();
  DBus::ValueOrError< int32_t > GetIndexInParent();
  DBus::ValueOrError< std::array< uint32_t, 2 > > GetStates();
  DBus::ValueOrError< std::unordered_map< std::string, std::string > > GetAttributes();
  DBus::ValueOrError< std::vector< std::string > > GetInterfaces();
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_OBJECT_H
