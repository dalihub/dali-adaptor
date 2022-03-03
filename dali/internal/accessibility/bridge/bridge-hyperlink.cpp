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
#include <dali/internal/accessibility/bridge/bridge-hyperlink.h>

using namespace Dali::Accessibility;

void BridgeHyperlink::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::HYPERLINK)};
  AddGetPropertyToInterface(desc, "NAnchors", &BridgeHyperlink::GetAnchorCount);
  AddGetPropertyToInterface(desc, "StartIndex", &BridgeHyperlink::GetStartIndex);
  AddGetPropertyToInterface(desc, "EndIndex", &BridgeHyperlink::GetEndIndex);
  AddFunctionToInterface(desc, "GetObject", &BridgeHyperlink::GetAnchorAccessible);
  AddFunctionToInterface(desc, "GetURI", &BridgeHyperlink::GetAnchorUri);
  AddFunctionToInterface(desc, "IsValid", &BridgeHyperlink::IsValid);
  mDbusServer.addInterface("/", desc, true);
}

Hyperlink* BridgeHyperlink::FindSelf() const
{
  return FindCurrentObjectWithInterface<Dali::Accessibility::AtspiInterface::HYPERLINK>();
}

DBus::ValueOrError<int32_t> BridgeHyperlink::GetEndIndex()
{
  return FindSelf()->GetEndIndex();
}

DBus::ValueOrError<int32_t> BridgeHyperlink::GetStartIndex()
{
  return FindSelf()->GetStartIndex();
}

DBus::ValueOrError<int32_t> BridgeHyperlink::GetAnchorCount()
{
  return FindSelf()->GetAnchorCount();
}

DBus::ValueOrError<Dali::Accessibility::Accessible*> BridgeHyperlink::GetAnchorAccessible(int32_t anchorIndex)
{
  return FindSelf()->GetAnchorAccessible(anchorIndex);
}

DBus::ValueOrError<std::string> BridgeHyperlink::GetAnchorUri(int32_t anchorIndex)
{
  return FindSelf()->GetAnchorUri(anchorIndex);
}

DBus::ValueOrError<bool> BridgeHyperlink::IsValid()
{
  return FindSelf()->IsValid();
}
