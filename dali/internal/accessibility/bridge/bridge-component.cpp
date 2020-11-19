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

// CLASS HEADER
#include <dali/internal/accessibility/bridge/bridge-component.h>

// EXTERNAL INCLUDES
#include <iostream>

#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"

using namespace Dali::Accessibility;

BridgeComponent::BridgeComponent()
{
}

void BridgeComponent::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{AtspiDbusInterfaceComponent};
  AddFunctionToInterface( desc, "Contains", &BridgeComponent::Contains );
  AddFunctionToInterface( desc, "GetAccessibleAtPoint", &BridgeComponent::GetAccessibleAtPoint );
  AddFunctionToInterface( desc, "GetExtents", &BridgeComponent::GetExtents );
  AddFunctionToInterface( desc, "GetPosition", &BridgeComponent::GetPosition );
  AddFunctionToInterface( desc, "GetSize", &BridgeComponent::GetSize );
  AddFunctionToInterface( desc, "GetLayer", &BridgeComponent::GetLayer );
  AddFunctionToInterface( desc, "GetAlpha", &BridgeComponent::GetAlpha );
  AddFunctionToInterface( desc, "GetMDIZOrder", &BridgeComponent::GetMdiZOrder );
  AddFunctionToInterface( desc, "GrabHighlight", &BridgeComponent::GrabHighlight );
  AddFunctionToInterface( desc, "GrabFocus", &BridgeComponent::GrabFocus );
  AddFunctionToInterface( desc, "ClearHighlight", &BridgeComponent::ClearHighlight );
  dbusServer.addInterface( "/", desc, true );
}

Component* BridgeComponent::FindSelf() const
{
  auto s = BridgeBase::FindSelf();
  assert( s );
  auto s2 = dynamic_cast< Component* >( s );
  if( !s2 )
    throw std::domain_error{"object " + s->GetAddress().ToString() + " doesn't have Component interface"};
  return s2;
}

DBus::ValueOrError< bool > BridgeComponent::Contains( int32_t x, int32_t y, uint32_t coordType )
{
  return FindSelf()->Contains( {x, y}, static_cast< CoordType >( coordType ) );
}
DBus::ValueOrError< Accessible* > BridgeComponent::GetAccessibleAtPoint( int32_t x, int32_t y, uint32_t coordType )
{
  return FindSelf()->GetAccessibleAtPoint( {x, y}, static_cast< CoordType >( coordType ) );
}
DBus::ValueOrError< std::tuple< int32_t, int32_t, int32_t, int32_t > > BridgeComponent::GetExtents( uint32_t coordType )
{
  auto p = FindSelf()->GetExtents( static_cast< CoordType >( coordType ) );
  return std::tuple< int32_t, int32_t, int32_t, int32_t >{p.x, p.y, p.width, p.height};
}
DBus::ValueOrError< int32_t, int32_t > BridgeComponent::GetPosition( uint32_t coordType )
{
  auto p = FindSelf()->GetExtents( static_cast< CoordType >( coordType ) );
  return { static_cast<int32_t>(p.x), static_cast<int32_t>(p.y) };
}
DBus::ValueOrError< int32_t, int32_t > BridgeComponent::GetSize( uint32_t coordType )
{
  auto p = FindSelf()->GetExtents( static_cast< CoordType >( coordType ) );
  return { static_cast<int32_t>(p.width), static_cast<int32_t>(p.height) };
}
DBus::ValueOrError< ComponentLayer > BridgeComponent::GetLayer()
{
  return FindSelf()->GetLayer();
}
DBus::ValueOrError< double > BridgeComponent::GetAlpha()
{
  return FindSelf()->GetAlpha();
}
DBus::ValueOrError< bool > BridgeComponent::GrabFocus()
{
  return FindSelf()->GrabFocus();
}
DBus::ValueOrError< bool > BridgeComponent::GrabHighlight()
{
  return FindSelf()->GrabHighlight();
}
DBus::ValueOrError< bool > BridgeComponent::ClearHighlight()
{
  return FindSelf()->ClearHighlight();
}
DBus::ValueOrError< int16_t > BridgeComponent::GetMdiZOrder()
{
  return FindSelf()->GetMdiZOrder();
}
