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
#include <dali/internal/accessibility/bridge/bridge-socket.h>

#include <dali/devel-api/atspi-interfaces/accessible.h>

using namespace Dali::Accessibility;

void BridgeSocket::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::SOCKET)};

  AddFunctionToInterface(desc, "Embed", &BridgeSocket::Embed);
  AddFunctionToInterface(desc, "Unembed", &BridgeSocket::Unembed);
  AddFunctionToInterface(desc, "SetOffset", &BridgeSocket::SetOffset);

  mDbusServer.addInterface("/", desc, true);
}

Socket* BridgeSocket::FindSelf() const
{
  return FindCurrentObjectWithInterface<Dali::Accessibility::AtspiInterface::SOCKET>();
}

DBus::ValueOrError<Address> BridgeSocket::Embed(Address plug)
{
  return FindSelf()->Embed(plug);
}

DBus::ValueOrError<void> BridgeSocket::Unembed(Address plug)
{
  FindSelf()->Unembed(plug);
  return {};
}

DBus::ValueOrError<void> BridgeSocket::SetOffset(std::int32_t x, std::int32_t y)
{
  FindSelf()->SetOffset(x, y);
  return {};
}
