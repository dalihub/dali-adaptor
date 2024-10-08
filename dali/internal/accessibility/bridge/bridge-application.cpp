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
#include <string>

// INTERNAL INCLUDES
#include <dali/internal/accessibility/bridge/bridge-application.h>

using namespace Dali::Accessibility;

void BridgeApplication::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::APPLICATION)};
  AddGetPropertyToInterface(desc, "ToolkitName", &BridgeApplication::GetToolkitName);
  AddGetPropertyToInterface(desc, "Version", &BridgeApplication::GetVersion);
  mDbusServer.addInterface("/", desc, true);
}

Application* BridgeApplication::FindSelf() const
{
  return FindCurrentObjectWithInterface<Dali::Accessibility::AtspiInterface::APPLICATION>();
}

std::string BridgeApplication::GetToolkitName()
{
  return FindSelf()->GetToolkitName();
}

std::string BridgeApplication::GetVersion()
{
  return FindSelf()->GetVersion();
}
