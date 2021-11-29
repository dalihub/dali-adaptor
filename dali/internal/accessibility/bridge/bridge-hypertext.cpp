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
#include <dali/internal/accessibility/bridge/bridge-hypertext.h>

using namespace Dali::Accessibility;

void BridgeHypertext::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{AtspiDbusInterfaceHypertext};
  AddFunctionToInterface(desc, "GetNLinks", &BridgeHypertext::GetLinkCount);
  AddFunctionToInterface(desc, "GetLink", &BridgeHypertext::GetLink);
  AddFunctionToInterface(desc, "GetLinkIndex", &BridgeHypertext::GetLinkIndex);
  mDbusServer.addInterface("/", desc, true);
}

Hypertext* BridgeHypertext::FindSelf() const
{
  auto self = BridgeBase::FindSelf();
  assert(self);
  auto hypertextInterface = dynamic_cast<Hypertext*>(self);
  if(!hypertextInterface)
  {
    throw std::domain_error{"object " + self->GetAddress().ToString() + " doesn't have Hypertext interface"};
  }
  return hypertextInterface;
}

DBus::ValueOrError<int32_t> BridgeHypertext::GetLinkCount()
{
  return FindSelf()->GetLinkCount();
}

DBus::ValueOrError<Dali::Accessibility::Accessible*> BridgeHypertext::GetLink(int32_t linkIndex)
{
  return FindSelf()->GetLink(linkIndex);
}

DBus::ValueOrError<int32_t> BridgeHypertext::GetLinkIndex(int32_t characterOffset)
{
  return FindSelf()->GetLinkIndex(characterOffset);
}