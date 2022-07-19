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

// CLASS HEADER
#include <dali/internal/accessibility/bridge/bridge-component.h>

#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"

using namespace Dali::Accessibility;

BridgeComponent::BridgeComponent()
{
}

void BridgeComponent::RegisterInterfaces()
{
  // The second arguments below are the names (or signatures) of DBus methods.
  // Screen Reader will call the methods with the exact names as specified in the AT-SPI Component interface:
  // https://gitlab.gnome.org/GNOME/at-spi2-core/-/blob/master/xml/Component.xml

  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::COMPONENT)};
  AddFunctionToInterface(desc, "Contains", &BridgeComponent::IsAccessibleContainingPoint);
  AddFunctionToInterface(desc, "GetAccessibleAtPoint", &BridgeComponent::GetAccessibleAtPoint);
  AddFunctionToInterface(desc, "GetExtents", &BridgeComponent::GetExtents);
  AddFunctionToInterface(desc, "GetPosition", &BridgeComponent::GetPosition);
  AddFunctionToInterface(desc, "GetSize", &BridgeComponent::GetSize);
  AddFunctionToInterface(desc, "GetLayer", &BridgeComponent::GetLayer);
  AddFunctionToInterface(desc, "GetAlpha", &BridgeComponent::GetAlpha);
  AddFunctionToInterface(desc, "GetMDIZOrder", &BridgeComponent::GetMdiZOrder);
  AddFunctionToInterface(desc, "GrabHighlight", &BridgeComponent::GrabHighlight);
  AddFunctionToInterface(desc, "GrabFocus", &BridgeComponent::GrabFocus);
  AddFunctionToInterface(desc, "ClearHighlight", &BridgeComponent::ClearHighlight);
  mDbusServer.addInterface("/", desc, true);
}

Component* BridgeComponent::FindSelf() const
{
  return FindCurrentObjectWithInterface<Dali::Accessibility::AtspiInterface::COMPONENT>();
}

DBus::ValueOrError<bool> BridgeComponent::IsAccessibleContainingPoint(int32_t x, int32_t y, uint32_t coordType)
{
  return FindSelf()->IsAccessibleContainingPoint({x, y}, static_cast<CoordinateType>(coordType));
}

DBus::ValueOrError<Accessible*> BridgeComponent::GetAccessibleAtPoint(int32_t x, int32_t y, uint32_t coordType)
{
  return FindSelf()->GetAccessibleAtPoint({x, y}, static_cast<CoordinateType>(coordType));
}

DBus::ValueOrError<std::tuple<int32_t, int32_t, int32_t, int32_t> > BridgeComponent::GetExtents(uint32_t coordType)
{
  auto rect = FindSelf()->GetExtents(static_cast<CoordinateType>(coordType));

  rect.x += mData->mExtentsOffset.first;
  rect.y += mData->mExtentsOffset.second;

  return std::tuple<int32_t, int32_t, int32_t, int32_t>{rect.x, rect.y, rect.width, rect.height};
}

DBus::ValueOrError<int32_t, int32_t> BridgeComponent::GetPosition(uint32_t coordType)
{
  auto rect = FindSelf()->GetExtents(static_cast<CoordinateType>(coordType));

  rect.x += mData->mExtentsOffset.first;
  rect.y += mData->mExtentsOffset.second;

  return {static_cast<int32_t>(rect.x), static_cast<int32_t>(rect.y)};
}

DBus::ValueOrError<int32_t, int32_t> BridgeComponent::GetSize(uint32_t coordType)
{
  auto rect = FindSelf()->GetExtents(static_cast<CoordinateType>(coordType));
  return {static_cast<int32_t>(rect.width), static_cast<int32_t>(rect.height)};
}

DBus::ValueOrError<ComponentLayer> BridgeComponent::GetLayer()
{
  return FindSelf()->GetLayer();
}

DBus::ValueOrError<double> BridgeComponent::GetAlpha()
{
  return FindSelf()->GetAlpha();
}

DBus::ValueOrError<bool> BridgeComponent::GrabFocus()
{
  return FindSelf()->GrabFocus();
}

DBus::ValueOrError<bool> BridgeComponent::GrabHighlight()
{
  return FindSelf()->GrabHighlight();
}

DBus::ValueOrError<bool> BridgeComponent::ClearHighlight()
{
  return FindSelf()->ClearHighlight();
}

DBus::ValueOrError<int16_t> BridgeComponent::GetMdiZOrder()
{
  return FindSelf()->GetMdiZOrder();
}
