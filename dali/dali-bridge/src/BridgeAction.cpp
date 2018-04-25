#include "BridgeAction.hpp"
#include <iostream>

using namespace Dali::Accessibility;

void BridgeAction::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{ATSPI_DBUS_INTERFACE_ACTION};

  AddGetPropertyToInterface( desc, "NActions", &BridgeAction::GetActionCount );

  AddFunctionToInterface( desc, "GetName", &BridgeAction::GetActionName );
  AddFunctionToInterface( desc, "GetLocalizedName", &BridgeAction::GetLocalizedActionName );
  AddFunctionToInterface( desc, "GetDescription", &BridgeAction::GetActionDescription );
  AddFunctionToInterface( desc, "GetKeyBinding", &BridgeAction::GetActionKeyBinding );
  AddFunctionToInterface( desc, "DoAction", &BridgeAction::DoAction );
  AddFunctionToInterface( desc, "DoActionName", &BridgeAction::DoActionName );
  dbusServer.addInterface( "/", desc, true );
}

Action* BridgeAction::FindSelf() const
{
  auto s = BridgeBase::FindSelf();
  assert( s );
  auto s2 = dynamic_cast< Action* >( s );
  if( !s2 )
    throw AccessibleError{"object " + s->GetAddress().ToString() + " doesn't have Action interface"};
  return s2;
}

DBus::ValueOrError< std::string > BridgeAction::GetActionName( int32_t index )
{
  auto self = FindSelf();
  return self->GetActionName( index );
}

DBus::ValueOrError< std::string > BridgeAction::GetLocalizedActionName( int32_t index )
{
  auto self = FindSelf();
  return self->GetLocalizedActionName( index );
}

DBus::ValueOrError< std::string > BridgeAction::GetActionDescription( int32_t index )
{
  auto self = FindSelf();
  return self->GetActionDescription( index );
}

DBus::ValueOrError< std::string > BridgeAction::GetActionKeyBinding( int32_t index )
{
  auto self = FindSelf();
  return self->GetActionKeyBinding( index );
}

DBus::ValueOrError< int32_t > BridgeAction::GetActionCount()
{
  auto self = FindSelf();
  return self->GetActionCount();
  ;
}

DBus::ValueOrError< bool > BridgeAction::DoAction( int32_t index )
{
  auto self = FindSelf();
  return self->DoAction( index );
}

DBus::ValueOrError< bool > BridgeAction::DoActionName( std::string name )
{
  auto self = FindSelf();
  auto cnt = self->GetActionCount();
  for( auto i = 0u; i < cnt; ++i )
  {
    if( self->GetActionName( i ) == name )
    {
      return self->DoAction( i );
    }
  }
  throw AccessibleError{"object " + self->GetAddress().ToString() + " doesn't have action '" + name + "'"};
}
