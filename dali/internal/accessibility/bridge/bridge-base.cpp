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
#include <dali/internal/accessibility/bridge/bridge-base.h>

// EXTERNAL INCLUDES
#include <atomic>
#include <cstdlib>
#include <memory>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/timer.h>

using namespace Dali::Accessibility;

static Dali::Timer tickTimer;

BridgeBase::~BridgeBase()
{
}

BridgeBase::BridgeBase()
{
}

void BridgeBase::addFilteredEvent( FilteredEvents kind, Dali::Accessibility::Accessible* obj, float delay, std::function<void()> functor )
{
  if( delay < 0 )
  {
    delay = 0;
  }

  auto it = filteredEvents.insert({ { kind, obj }, { static_cast<unsigned int>(delay * 10), {} } });
  if (it.second)
  {
    functor();
  }
  else
  {
    it.first->second.second = std::move(functor);
  }

  if (!tickTimer)
  {
    tickTimer = Dali::Timer::New(100);
    tickTimer.TickSignal().Connect(this, &BridgeBase::tickFilteredEvents);
  }
}

bool BridgeBase::tickFilteredEvents()
{
  for(auto it = filteredEvents.begin(); it != filteredEvents.end(); )
  {
    if (it->second.first)
    {
      --it->second.first;
    }
    else
    {
      if (it->second.second)
      {
        it->second.second();
        it->second.second = {};
      }
      else
      {
        it = filteredEvents.erase(it);
        continue;
      }
    }
    ++it;
  }
  return !filteredEvents.empty();
}

BridgeBase::ForceUpResult BridgeBase::ForceUp()
{
  if( Bridge::ForceUp() == ForceUpResult::ALREADY_UP )
  {
    return ForceUpResult::ALREADY_UP;
  }
  auto proxy = DBus::DBusClient{dbusLocators::atspi::BUS, dbusLocators::atspi::OBJ_PATH, dbusLocators::atspi::BUS_INTERFACE, DBus::ConnectionType::SESSION};
  auto addr = proxy.method< std::string() >( dbusLocators::atspi::GET_ADDRESS ).call();

  if( !addr )
  {
    throw std::domain_error{std::string( "failed at call '" ) + dbusLocators::atspi::GET_ADDRESS + "': " + addr.getError().message};
  }

  con = DBusWrapper::Installed()->eldbus_address_connection_get_impl( std::get< 0 >( addr ) );
  data->busName = DBus::getConnectionName( con );
  dbusServer = { con };

  {
    DBus::DBusInterfaceDescription desc{"org.a11y.atspi.Cache"};
    AddFunctionToInterface( desc, "GetItems", &BridgeBase::GetItems );
    dbusServer.addInterface( "/org/a11y/atspi/cache", desc );
  }
  {
    DBus::DBusInterfaceDescription desc{"org.a11y.atspi.Application"};
    AddGetSetPropertyToInterface( desc, "Id", &BridgeBase::IdGet, &BridgeBase::IdSet );
    dbusServer.addInterface( AtspiPath, desc );
  }

  return ForceUpResult::JUST_STARTED;
}

void BridgeBase::ForceDown()
{
  Bridge::ForceDown();
  dbusServer = {};
  con = {};
}

const std::string& BridgeBase::GetBusName() const
{
  static std::string empty;
  return data ? data->busName : empty;
}

Accessible* BridgeBase::FindByPath( const std::string& name ) const
{
  try
  {
    return Find( name );
  }
  catch( std::domain_error& )
  {
    return nullptr;
  }
}

void BridgeBase::AddPopup( Accessible* obj )
{
  if( std::find( popups.begin(), popups.end(), obj ) != popups.end() )
  {
    return;
  }
  popups.push_back( obj );
  if (IsUp())
  {
    obj->Emit( WindowEvent::ACTIVATE, 0 );
  }
}

void BridgeBase::RemovePopup( Accessible* obj )
{
  auto it = std::find( popups.begin(), popups.end(), obj );
  if( it == popups.end() )
  {
    return;
  }
  popups.erase( it );
  if (IsUp())
  {
    obj->Emit( WindowEvent::DEACTIVATE, 0 );
    if( popups.empty() )
    {
      application.children.back()->Emit( WindowEvent::ACTIVATE, 0 );
    }
    else
    {
      popups.back()->Emit( WindowEvent::ACTIVATE, 0 );
    }
  }
}

void BridgeBase::AddTopLevelWindow( Accessible* root )
{
  application.children.push_back( root );
  SetIsOnRootLevel( root );
}

void BridgeBase::RemoveTopLevelWindow( Accessible* root )
{
  for(auto i = 0u; i < application.children.size(); ++i)
  {
    if( application.children[i] == root )
    {
      application.children.erase(application.children.begin() + i);
      break;
    }
  }
}

std::string BridgeBase::StripPrefix( const std::string& path )
{
  auto size = strlen( AtspiPath );
  return path.substr( size + 1 );
}

Accessible* BridgeBase::Find( const std::string& path ) const
{
  if( path == "root" )
  {
    return &application;
  }
  void* p;
  std::istringstream tmp{ path };
  if (! ( tmp >> p) )
  {
    throw std::domain_error{"invalid path '" + path + "'"};
  }
  auto it = data->knownObjects.find( static_cast<Accessible*>( p ) );
  if( it == data->knownObjects.end() )
  {
    throw std::domain_error{"unknown object '" + path + "'"};
  }
  return static_cast<Accessible*>( p );
}

Accessible* BridgeBase::Find( const Address& ptr ) const
{
  assert( ptr.GetBus() == data->busName );
  return Find( ptr.GetPath() );
}

Accessible* BridgeBase::FindSelf() const
{
  auto pth = DBus::DBusServer::getCurrentObjectPath();
  auto size = strlen( AtspiPath );
  if( pth.size() <= size )
  {
    throw std::domain_error{"invalid path '" + pth + "'"};
  }
  if( pth.substr( 0, size ) != AtspiPath )
  {
    throw std::domain_error{"invalid path '" + pth + "'"};
  }
  if( pth[size] != '/' )
  {
    throw std::domain_error{"invalid path '" + pth + "'"};
  }
  return Find( StripPrefix( pth ) );
}

void BridgeBase::IdSet( int id )
{
  this->id = id;
}

int BridgeBase::IdGet()
{
  return this->id;
}

auto BridgeBase::GetItems() -> DBus::ValueOrError< std::vector< CacheElementType > >
{
  auto root = &application;

  std::vector< CacheElementType > res;

  std::function< void(Accessible*) > proc =
    [&]( Accessible* item )
    {
      res.emplace_back( std::move( CreateCacheElement( root ) ) );
      for( auto i = 0u; i < item->GetChildCount(); ++i )
      {
        proc( item->GetChildAtIndex( i ) );
      }
    };

  return res;
}

auto BridgeBase::CreateCacheElement( Accessible* item ) -> CacheElementType
{
  if( !item )
  {
    return {};
  }

  auto root = &application;
  auto parent = item->GetParent();

  std::vector< Address > children;
  for( auto i = 0u; i < item->GetChildCount(); ++i )
  {
    children.emplace_back( item->GetChildAtIndex( i )->GetAddress() );
  }

  return std::make_tuple(
    item->GetAddress(),
    root->GetAddress(),
    parent ? parent->GetAddress() : Address{},
    children,
    item->GetInterfaces(),
    item->GetName(),
    item->GetRole(),
    item->GetDescription(),
    item->GetStates().GetRawData()
  );
}

