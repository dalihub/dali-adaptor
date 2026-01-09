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
#include <dali/internal/accessibility/bridge/bridge-value.h>

#include <dali/devel-api/atspi-interfaces/accessible.h>

using namespace Dali::Accessibility;

BridgeValue::BridgeValue()
{
}

void BridgeValue::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::VALUE)};
  AddGetSetPropertyToInterface(desc, "CurrentValue", &BridgeValue::GetCurrentValue, &BridgeValue::SetCurrentValue);
  AddGetPropertyToInterface(desc, "Text", &BridgeValue::GetCurrentValueText);
  AddGetPropertyToInterface(desc, "MaximumValue", &BridgeValue::GetMaximumValue);
  AddGetPropertyToInterface(desc, "MinimumIncrement", &BridgeValue::GetMinimumIncrement);
  AddGetPropertyToInterface(desc, "MinimumValue", &BridgeValue::GetMinimumValue);
  mDbusServer.addInterface("/", desc, true);
}

Value* BridgeValue::FindSelf() const
{
  return FindCurrentObjectWithInterface<Dali::Accessibility::AtspiInterface::VALUE>();
}

double BridgeValue::GetCurrentValue()
{
  return FindSelf()->GetCurrent();
}

void BridgeValue::SetCurrentValue(double newValue)
{
  FindSelf()->SetCurrent(newValue);
}

std::string BridgeValue::GetCurrentValueText()
{
  return FindSelf()->GetValueText();
}

double BridgeValue::GetMaximumValue()
{
  return FindSelf()->GetMaximum();
}

double BridgeValue::GetMinimumIncrement()
{
  return FindSelf()->GetMinimumIncrement();
}

double BridgeValue::GetMinimumValue()
{
  return FindSelf()->GetMinimum();
}
