#include "BridgeBase.hpp"
#include <atomic>
#include <cstdlib>

using namespace Dali::Accessibility;

BridgeBase::BridgeBase()
{
}

BridgeBase::ForceUpResult BridgeBase::ForceUp()
{
  if( Bridge::ForceUp() == ForceUpResult::alreadyUp )
    return ForceUpResult::alreadyUp;
  auto proxy = DBus::DBusClient{dbusLocators::atspi::BUS, dbusLocators::atspi::OBJ_PATH,
                                dbusLocators::atspi::BUS_INTERFACE, DBus::ConnectionType::SESSION};
  auto addr = proxy.method< std::string() >( dbusLocators::atspi::GET_ADDRESS ).call();

  if( !addr )
    throw AccessibleError{std::string( "failed at call '" ) + dbusLocators::atspi::GET_ADDRESS +
                          "': " + addr.getError().message};

  con = std::make_shared< DBus::EldbusConnection >( eldbus_address_connection_get( std::get< 0 >( addr ).c_str() ) );
  data->busName = DBus::getConnectionName( con );
  data->root = &application;
  dbusServer = {con};

  {
    DBus::DBusInterfaceDescription desc{"org.a11y.atspi.Cache"};
    AddFunctionToInterface( desc, "GetItems", &BridgeBase::GetItems );
    dbusServer.addInterface( "/org/a11y/atspi/cache", desc );
  }
  {
    DBus::DBusInterfaceDescription desc{"org.a11y.atspi.Application"};
    AddGetSetPropertyToInterface( desc, "Id", &BridgeBase::IdGet, &BridgeBase::IdSet );
    dbusServer.addInterface( ATSPI_PATH, desc );
  }
  return ForceUpResult::justStarted;
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
  catch( AccessibleError )
  {
    return nullptr;
  }
}

void BridgeBase::SetApplicationChild( Accessible* root )
{
  // for now you can set root only once.
  // to set it multiple times you'd have to remove old one first, which usually involves
  // a lot of messy events and cornercases (imagine old root becoming child of new-not-yet-announced one)
  application.children.push_back( root );
  SetIsOnRootLevel( root );
}

// Accessible *BridgeBase::getApplicationRoot() const
// {
//     return rootElement;
// }

std::string BridgeBase::StripPrefix( const std::string& path )
{
  auto size = strlen( ATSPI_PATH );
  return path.substr( size + 1 );
}

Accessible* BridgeBase::Find( const std::string& path ) const
{
  if( path == "root" )
    return &application;
  char* p;
  auto val = std::strtoll( path.c_str(), &p, 10 );
  if( p == path.c_str() )
    throw AccessibleError{"invalid path '" + path + "'"};
  auto it = data->objects.find( val );
  if( it == data->objects.end() )
    throw AccessibleError{"unknown object '" + path + "'"};
  return it->second;
}

Accessible* BridgeBase::Find( const Address& ptr ) const
{
  assert( ptr.GetBus() == data->busName );
  return Find( ptr.GetPath() );
}

Accessible* BridgeBase::FindSelf() const
{
  auto pth = DBus::DBusServer::getCurrentObjectPath();
  auto size = strlen( ATSPI_PATH );
  if( pth.size() <= size )
    throw AccessibleError{"invalid path '" + pth + "'"};
  if( pth.substr( 0, size ) != ATSPI_PATH )
    throw AccessibleError{"invalid path '" + pth + "'"};
  if( pth[size] != '/' )
    throw AccessibleError{"invalid path '" + pth + "'"};
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

  res.emplace_back( std::move( CreateCacheElement( root ) ) );
  for( auto const& it : data->objects )
    res.emplace_back( std::move( CreateCacheElement( it.second ) ) );

  return res;
}

auto BridgeBase::CreateCacheElement( Accessible* item ) -> CacheElementType
{
  if( !item )
    return {};

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
      parent->GetAddress(),
      children,
      item->GetInterfaces(),
      item->GetName(),
      item->GetRole(),
      item->GetDescription(),
      item->GetStates().GetRawData() );
}
