#include "BridgeValue.hpp"
#include <iostream>

using namespace Dali::Accessibility;

BridgeValue::BridgeValue()
{
}

void BridgeValue::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{ATSPI_DBUS_INTERFACE_VALUE};
  AddGetSetPropertyToInterface( desc, "CurrentValue", &BridgeValue::GetCurrentValue, &BridgeValue::SetCurrentValue );
  AddGetPropertyToInterface( desc, "MaximumValue", &BridgeValue::GetMaximumValue );
  AddGetPropertyToInterface( desc, "MinimumIncrement", &BridgeValue::GetMinimumIncrement );
  AddGetPropertyToInterface( desc, "MinimumValue", &BridgeValue::GetMinimumValue );
  dbusServer.addInterface( "/", desc, true );
}

Value* BridgeValue::FindSelf() const
{
  auto s = BridgeBase::FindSelf();
  assert( s );
  auto s2 = dynamic_cast< Value* >( s );
  if( !s2 )
    throw AccessibleError{"object " + s->GetAddress().ToString() + " doesn't have Value interface"};
  return s2;
}
double BridgeValue::GetCurrentValue()
{
  return FindSelf()->GetCurrent();
}
void BridgeValue::SetCurrentValue( double new_value )
{
  FindSelf()->SetCurrent( new_value );
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
