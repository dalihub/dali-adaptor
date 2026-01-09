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

// CLASS HEADER
#include <dali/internal/accessibility/bridge/bridge-selection.h>

#include <dali/devel-api/atspi-interfaces/accessible.h>

using namespace Dali::Accessibility;

void BridgeSelection::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::SELECTION)};
  AddGetPropertyToInterface(desc, "NSelectedChildren", &BridgeSelection::GetSelectedChildrenCount);
  AddFunctionToInterface(desc, "GetSelectedChild", &BridgeSelection::GetSelectedChild);
  AddFunctionToInterface(desc, "SelectChild", &BridgeSelection::SelectChild);
  AddFunctionToInterface(desc, "DeselectSelectedChild", &BridgeSelection::DeselectSelectedChild);
  AddFunctionToInterface(desc, "IsChildSelected", &BridgeSelection::IsChildSelected);
  AddFunctionToInterface(desc, "SelectAll", &BridgeSelection::SelectAll);
  AddFunctionToInterface(desc, "ClearSelection", &BridgeSelection::ClearSelection);
  AddFunctionToInterface(desc, "DeselectChild", &BridgeSelection::DeselectChild);
  mDbusServer.addInterface("/", desc, true);
}

std::shared_ptr<Selection> BridgeSelection::FindSelf() const
{
  return FindCurrentObjectWithInterface<Dali::Accessibility::AtspiInterface::SELECTION>();
}

DBus::ValueOrError<int32_t> BridgeSelection::GetSelectedChildrenCount()
{
  return FindSelf()->GetSelectedChildrenCount();
}

DBus::ValueOrError<Dali::Accessibility::Accessible*> BridgeSelection::GetSelectedChild(int32_t selectedChildIndex)
{
  return FindSelf()->GetSelectedChild(selectedChildIndex);
}

DBus::ValueOrError<bool> BridgeSelection::SelectChild(int32_t childIndex)
{
  return FindSelf()->SelectChild(childIndex);
}

DBus::ValueOrError<bool> BridgeSelection::DeselectSelectedChild(int32_t selectedChildIndex)
{
  return FindSelf()->DeselectSelectedChild(selectedChildIndex);
}

DBus::ValueOrError<bool> BridgeSelection::IsChildSelected(int32_t childIndex)
{
  return FindSelf()->IsChildSelected(childIndex);
}

DBus::ValueOrError<bool> BridgeSelection::SelectAll()
{
  return FindSelf()->SelectAll();
}

DBus::ValueOrError<bool> BridgeSelection::ClearSelection()
{
  return FindSelf()->ClearSelection();
}

DBus::ValueOrError<bool> BridgeSelection::DeselectChild(int32_t childIndex)
{
  return FindSelf()->DeselectChild(childIndex);
}
