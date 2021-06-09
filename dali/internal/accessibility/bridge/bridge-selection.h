#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_SELECTION_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_SELECTION_H

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

// INTERNAL INCLUDES
#include <dali/internal/accessibility/bridge/bridge-base.h>

class BridgeSelection : public virtual BridgeBase
{
protected:
  BridgeSelection() = default;

  void RegisterInterfaces();

  Dali::Accessibility::Selection* FindSelf() const;

public:
  DBus::ValueOrError<int32_t> GetSelectedChildrenCount();
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetSelectedChild(int32_t selectedChildIndex);
  DBus::ValueOrError<bool> SelectChild(int32_t childIndex);
  DBus::ValueOrError<bool> DeselectSelectedChild(int32_t selectedChildIndex);
  DBus::ValueOrError<bool> IsChildSelected(int32_t childIndex);
  DBus::ValueOrError<bool> SelectAll();
  DBus::ValueOrError<bool> ClearSelection();
  DBus::ValueOrError<bool> DeselectChild(int32_t childIndex);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_SELECTION_H
