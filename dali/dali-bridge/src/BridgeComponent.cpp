#include "BridgeComponent.hpp"
#include <iostream>

#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"

using namespace Dali::Accessibility;

BridgeComponent::BridgeComponent()
{
}

void BridgeComponent::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{ATSPI_DBUS_INTERFACE_COMPONENT};
  AddFunctionToInterface( desc, "Contains", &BridgeComponent::Contains );
  AddFunctionToInterface( desc, "GetAccessibleAtPoint", &BridgeComponent::GetAccessibleAtPoint );
  AddFunctionToInterface( desc, "GetExtents", &BridgeComponent::GetExtents );
  AddFunctionToInterface( desc, "GetPosition", &BridgeComponent::GetPosition );
  AddFunctionToInterface( desc, "GetSize", &BridgeComponent::GetSize );
  AddFunctionToInterface( desc, "GetLayer", &BridgeComponent::GetLayer );
  AddFunctionToInterface( desc, "GetAlpha", &BridgeComponent::GetAlpha );
  AddFunctionToInterface( desc, "GrabHighlight", &BridgeComponent::GrabHighlight );
  AddFunctionToInterface( desc, "ClearHighlight", &BridgeComponent::ClearHighlight );
  dbusServer.addInterface( "/", desc, true );
}

Component* BridgeComponent::FindSelf() const
{
  auto s = BridgeBase::FindSelf();
  assert( s );
  auto s2 = dynamic_cast< Component* >( s );
  if( !s2 )
    throw AccessibleError{"object " + s->GetAddress().ToString() + " doesn't have Component interface"};
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
  return std::tuple< int32_t, int32_t, int32_t, int32_t >{p.position.x, p.position.y, p.size.width, p.size.height};
}
DBus::ValueOrError< int32_t, int32_t > BridgeComponent::GetPosition( uint32_t coordType )
{
  auto p = FindSelf()->GetExtents( static_cast< CoordType >( coordType ) );
  return {p.position.x, p.position.y};
}
DBus::ValueOrError< int32_t, int32_t > BridgeComponent::GetSize( uint32_t coordType )
{
  auto p = FindSelf()->GetExtents( static_cast< CoordType >( coordType ) );
  return {p.size.width, p.size.height};
}
DBus::ValueOrError< ComponentLayer > BridgeComponent::GetLayer()
{
  return FindSelf()->GetLayer();
}
DBus::ValueOrError< double > BridgeComponent::GetAlpha()
{
  return FindSelf()->GetAlpha();
}
DBus::ValueOrError< bool > BridgeComponent::GrabHighlight()
{
  return FindSelf()->GrabHighlight();
}
DBus::ValueOrError< bool > BridgeComponent::ClearHighlight()
{
  return FindSelf()->ClearHighlight();
}
