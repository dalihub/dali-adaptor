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
#include <dali/internal/accessibility/bridge/bridge-action.h>

// EXTERNAL INCLUDES
#include <iostream>

using namespace Dali::Accessibility;

void BridgeAction::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{AtspiDbusInterfaceAction};

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
    throw std::domain_error{"object " + s->GetAddress().ToString() + " doesn't have Action interface"};
  return s2;
}

DBus::ValueOrError< std::string > BridgeAction::GetActionName( int32_t index )
{
  return FindSelf()->GetActionName( index );
}

DBus::ValueOrError< std::string > BridgeAction::GetLocalizedActionName( int32_t index )
{
  return FindSelf()->GetLocalizedActionName( index );
}

DBus::ValueOrError< std::string > BridgeAction::GetActionDescription( int32_t index )
{
  return FindSelf()->GetActionDescription( index );
}

DBus::ValueOrError< std::string > BridgeAction::GetActionKeyBinding( int32_t index )
{
  return FindSelf()->GetActionKeyBinding( index );
}

DBus::ValueOrError< int32_t > BridgeAction::GetActionCount()
{
  return FindSelf()->GetActionCount();
  ;
}

DBus::ValueOrError< bool > BridgeAction::DoAction( int32_t index )
{
  return FindSelf()->DoAction( index );
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
  throw std::domain_error{"object " + self->GetAddress().ToString() + " doesn't have action '" + name + "'"};
}
